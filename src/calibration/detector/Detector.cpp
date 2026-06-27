#include "Detector.hpp"
#include <opencv2/calib3d.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <cv/CvImage.hpp>
#include "ChessboardParameters.hpp"
#include "CircleboardParameters.hpp"
#include "CharucoParameters.hpp"
#include "AprilTagParameters.hpp"
#include "../CameraIntrinsics.hpp"
#include "../Board.hpp"


Detector::Detector(
    CameraIntrinsics camIntrinsics,
    ChessboardParameters detParams)
	: camIntrinsics_(std::move(camIntrinsics)),
    detectionParams_(std::make_shared<ChessboardParameters>(std::move(detParams)))
{
}

Detector::Detector(
    CameraIntrinsics camIntrinsics,
    CircleboardParameters detParams)
	: camIntrinsics_(std::move(camIntrinsics)),
    detectionParams_(std::make_shared<CircleboardParameters>(std::move(detParams)))
{
}

Detector::Detector(
    CameraIntrinsics camIntrinsics,
    CharucoParameters detParams)
	: camIntrinsics_(std::move(camIntrinsics)),
    detectionParams_(std::make_shared<CharucoParameters>(std::move(detParams)))
{
}

Detector::Detector(
    CameraIntrinsics camIntrinsics,
    AprilTagParameters detParams)
	: camIntrinsics_(std::move(camIntrinsics)),
    detectionParams_(std::make_shared<AprilTagParameters>(std::move(detParams)))
{
}

//////////////////////////////////////////////////////

void Detector::setCameraIntrinsics(CameraIntrinsics camIntrinsics)
{
    camIntrinsics_ = std::move(camIntrinsics);
}

void Detector::setDetectionParameters(ChessboardParameters detParams)
{
    detectionParams_ = std::make_shared<ChessboardParameters>(std::move(detParams));
}

void Detector::setDetectionParameters(CircleboardParameters detParams)
{
    detectionParams_ = std::make_shared<CircleboardParameters>(std::move(detParams));
}

void Detector::setDetectionParameters(CharucoParameters detParams)
{
    detectionParams_ = std::make_shared<CharucoParameters>(std::move(detParams));
}

void Detector::setDetectionParameters(AprilTagParameters detParams)
{
    detectionParams_ = std::make_shared<AprilTagParameters>(std::move(detParams));
}

std::shared_ptr<Board> Detector::findBoard(const CvImage& iconicImage)
{
    const BoardPattern patternType = detectionParams_->patternType();

    if (patternType == BoardPattern::CHESSBOARD || patternType == BoardPattern::SYMMETRIC_CIRCLES || patternType == BoardPattern::ASYMMETRIC_CIRCLES)
        return findCommonTypesBoard(iconicImage);

    if (patternType == BoardPattern::CHARUCO || patternType == BoardPattern::APRIL_TAG)
        return findArucoBasedBoard(iconicImage);

    throw std::runtime_error("Board pattern not supported");
}

//////////////////////////////////////////////////////////////

std::shared_ptr<Board> Detector::findCommonTypesBoard(const CvImage& iconicImage)
{
    cv::Mat mat = iconicImage.toGray().mat();

    const BoardPattern patternType = detectionParams_->patternType();
    Detector::FindBoardResult findResult;

    // Detect calibration pattern
    switch (patternType)
    {
        case BoardPattern::CHESSBOARD:
            findResult = findChessboard(mat);

            break;

        case BoardPattern::SYMMETRIC_CIRCLES:
        case BoardPattern::ASYMMETRIC_CIRCLES:
            findResult = findCircleboard(mat);

            break;

        default:
            break;
    }

    if (!findResult.found)
    {
        return std::make_shared<Board>(
            Board::notDetected(
                iconicImage,
                patternType,
                detectionParams_->getGeometry().patternSize()
            )
        );
    }


    std::vector<cv::Point3f> objectPoints;

    switch (detectionParams_->patternType())
    {
        case BoardPattern::CHESSBOARD:
            objectPoints = generateChessboardObjectPoints();

        case BoardPattern::SYMMETRIC_CIRCLES:
        case BoardPattern::ASYMMETRIC_CIRCLES:
            objectPoints = generateCirclesObjectPoints();

        default:
            break;
    }

    CvContour boardContour = generateBoardContour(findResult.corners, objectPoints);

    // Solve PnP to get pose (rotation and translation vectors)
    cv::Mat rvec, tvec;
    bool pnpSuccess = false;
    try
    {
        pnpSuccess = cv::solvePnP(
            objectPoints,
            findResult.corners,
            camIntrinsics_.cameraMatrix(),
            camIntrinsics_.distortionModel().coeffs(),
            rvec,
            tvec,
            false,
            cv::SOLVEPNP_ITERATIVE
        );
    }
    catch (const cv::Exception& ex)
    {
        const std::string& err = ex.what();
        pnpSuccess = false;
    }

    if (!pnpSuccess)
    {
        return std::make_shared<Board>(
            Board::poseNotSolved(
                iconicImage,
                patternType,
                detectionParams_->getGeometry().patternSize(),
                std::move(boardContour),
                std::move(findResult.corners),
                std::move(objectPoints)
            )
        );
    }

    std::vector<CvContour> marksContours;
    switch (patternType)
    {
        case BoardPattern::CHESSBOARD:
            marksContours = generateChessboardMarksContour(findResult.corners);
            break;

        case BoardPattern::SYMMETRIC_CIRCLES:
        case BoardPattern::ASYMMETRIC_CIRCLES:
            marksContours = generateCirclesMarksContour(findResult.corners, objectPoints, rvec, tvec);
            break;

        default:
            break;
    }

    std::vector<cv::Point2f> axes2D = generateAxes2D(objectPoints, rvec, tvec);

    return std::make_shared<Board>(
        Board(
            iconicImage,
            detectionParams_->patternType(),
            detectionParams_->getGeometry().patternSize(),
            std::move(boardContour),
            std::move(marksContours),
            std::move(findResult.corners),
            std::move(objectPoints),
            std::move(rvec),
            std::move(tvec),
            std::move(axes2D)
        )
    );
}

std::shared_ptr<Board> Detector::findArucoBasedBoard(const CvImage& iconicImage)
{
    cv::Mat mat = iconicImage.toGray().mat();
    const BoardPattern patternType = detectionParams_->patternType();

    cv::Size patternSize;
    cv::aruco::Board* arucoBoard{ nullptr };

    FindArucoResult findResult;

    switch (patternType)
    {
        case BoardPattern::CHARUCO:
        {
            findResult = findCharucoBoard(mat);

            auto p = std::static_pointer_cast<CharucoParameters>(detectionParams_);

            patternSize = p->geometry.patternSize();
            arucoBoard = &p->geometry.charuco;

            break;
        }

        case BoardPattern::APRIL_TAG:
        {
            findResult = findAprilTagBoard(mat);

            auto p = std::static_pointer_cast<AprilTagParameters>(detectionParams_);

            patternSize = p->geometry.patternSize();
            arucoBoard = &p->geometry.aprilTag;

            break;
        }

        default:
            break;
    }

    if (!findResult.found)
    {
        return std::make_shared<Board>(Board::notDetected(
            iconicImage,
            patternType,
            patternSize
        ));
    }

    std::vector<cv::Point3f> objectPoints;
    std::vector<cv::Point2f> imgPoints;

    switch (patternType)
    {
    case BoardPattern::CHARUCO:
        arucoBoard->matchImagePoints(
            findResult.corners,
            findResult.arucoIds,
            objectPoints,
            imgPoints
        );

        break;

    case BoardPattern::APRIL_TAG:
        arucoBoard->matchImagePoints(
            findResult.markerCorners,
            findResult.markerIds,
            objectPoints,
            imgPoints
        );

        break;

    default:
        break;
    }
   
    if (imgPoints.size() < 4)
    {
        return std::make_shared<Board>(Board::notDetected(
            iconicImage,
            patternType,
            patternSize
        ));
    }

    CvContour boardContour = generateBoardContour(imgPoints, objectPoints);

    // Solve PnP to get pose (rotation and translation vectors)
    cv::Mat rvec, tvec;

    bool pnpSuccess = cv::solvePnP(
            objectPoints,
            imgPoints,
            camIntrinsics_.cameraMatrix(),
            camIntrinsics_.distortionModel().coeffs(),
            rvec,
            tvec,
            false,
            cv::SOLVEPNP_ITERATIVE
        );

    if (!pnpSuccess)
    {
        return std::make_shared<Board>(Board::poseNotSolved(
            iconicImage,
            patternType,
            patternSize,
            std::move(boardContour),
            std::move(imgPoints),
            std::move(objectPoints)
        ));
    }

    // Marks contours: project the four corners of each detected ArUco marker square.
    std::vector<CvContour> marksContours = generateQuadMarksContour(findResult.markerCorners);


    std::vector<cv::Point2f> axes2D = generateAxes2D(objectPoints, rvec, tvec);

    return std::make_shared<Board>(Board(
        iconicImage,
        patternType,
        patternSize,
        std::move(boardContour),
        std::move(marksContours),
        std::move(imgPoints),
        std::move(objectPoints),
        std::move(rvec),
        std::move(tvec),
        std::move(axes2D)
    ));
}

Detector::FindBoardResult Detector::findChessboard(const cv::Mat& input) const
{
    auto p = std::static_pointer_cast<ChessboardParameters>(detectionParams_);

    int flags = 0;

    if (p->detection.adaptiveThreshold) flags |= cv::CALIB_CB_ADAPTIVE_THRESH;
    if (p->detection.normalizeImage)    flags |= cv::CALIB_CB_NORMALIZE_IMAGE;
    if (p->detection.filterQuads)       flags |= cv::CALIB_CB_FILTER_QUADS;
    if (p->detection.subpixelAccuracy)  flags |= cv::CALIB_CB_ACCURACY;
    if (p->detection.allowLargerBoards) flags |= cv::CALIB_CB_LARGER;

    switch (p->detection.searchAccuracy)
    {
    case ChessboardParameters::Detection::SearchAccuracy::FAST:
            flags |= cv::CALIB_CB_FAST_CHECK;
            break;
        case ChessboardParameters::Detection::SearchAccuracy::EXHAUSTIVE:
            flags |= cv::CALIB_CB_EXHAUSTIVE;
            break;
        case ChessboardParameters::Detection::SearchAccuracy::BALANCED:
            break;
    }

    Detector::FindBoardResult result;

    result.found = cv::findChessboardCorners(
        input,
        p->getGeometry().patternSize(),
        result.corners,
        flags
    );

    // Refine corner positions with sub-pixel accuarcy
    if (result.found)
    {
        int termCriteria = 0;

        switch (p->refine.type)
        {
        case ChessboardParameters::Refine::TermCriteriaType::COUNT:
            termCriteria = cv::TermCriteria::Type::COUNT;

            break;

        case ChessboardParameters::Refine::TermCriteriaType::EPSILON:
            termCriteria = cv::TermCriteria::Type::EPS;

            break;

        case ChessboardParameters::Refine::TermCriteriaType::BOTH:
            termCriteria = cv::TermCriteria::Type::COUNT + cv::TermCriteria::Type::EPS;

            break;
        }

        cv::TermCriteria criteria(
            termCriteria,
            p->refine.maxCount,
            p->refine.epsilon
        );
        cv::cornerSubPix(
            input,
            result.corners,
            p->refine.winSize,
            p->refine.zeroZone,
            criteria
        );
    }

	return result;
}

Detector::FindBoardResult Detector::findCircleboard(const cv::Mat& input) const
{
    auto p = std::static_pointer_cast<CircleboardParameters>(detectionParams_);

    int flags = 0;

    if (detectionParams_->patternType() == BoardPattern::SYMMETRIC_CIRCLES)    flags |= cv::CALIB_CB_SYMMETRIC_GRID;
    else                                                    flags |= cv::CALIB_CB_ASYMMETRIC_GRID;
    if (p->detection.useClustering)                         flags |= cv::CALIB_CB_CLUSTERING;

    Detector::FindBoardResult result;

    result.found = cv::findCirclesGrid(
        input,
        p->getGeometry().patternSize(),
        result.corners,
        flags
    );

	return result;
}

Detector::FindArucoResult Detector::findCharucoBoard(const cv::Mat& input) const
{
    auto p = std::static_pointer_cast<CharucoParameters>(detectionParams_);

    Detector::FindArucoResult result;

    cv::aruco::CharucoDetector charucoDetector(
        p->geometry.charuco,
        p->detection.charuco,
        p->detection.arucoDetector,
        p->refine.arucoRefine
    );

    charucoDetector.detectBoard(
        input,
        result.corners,
        result.arucoIds,
        result.markerCorners,
        result.markerIds
    );

    result.found = !result.corners.empty();

    return result;
}

Detector::FindArucoResult Detector::findAprilTagBoard(const cv::Mat& input) const
{
    auto p = std::static_pointer_cast<AprilTagParameters>(detectionParams_);

    Detector::FindArucoResult result;

    cv::aruco::ArucoDetector aprilTagDetector(
        cv::aruco::getPredefinedDictionary(p->geometry.dictionary()),
        p->detection.arucoDetector,
        p->refine.arucoRefine
    );

    std::vector<std::vector<cv::Point2f>> rejectedCorners;

    aprilTagDetector.detectMarkers(
        input,
        result.markerCorners,
        result.markerIds,
        rejectedCorners
    );

    result.found = !result.markerCorners.empty();

    if (result.found)
    {
        aprilTagDetector.refineDetectedMarkers(
            input,
            p->geometry.aprilTag,
            result.markerCorners,
            result.markerIds,
            rejectedCorners
        );
    }

    for (const auto& quad : result.markerCorners)
    {
        for (const auto& pt : quad)
        {
            result.corners.push_back(pt);
        }
    }

    return result;
}

std::vector<cv::Point3f> Detector::generateChessboardObjectPoints() const
{
    if (detectionParams_->patternType() != BoardPattern::CHESSBOARD)
    {
        throw std::runtime_error("Detection parameters type mismatch: expected CHESSBOARD");
    }

    auto p = std::static_pointer_cast<ChessboardParameters>(detectionParams_);

    std::vector<cv::Point3f> objectPoints;

    // Use actual physical dimensions from board description
    // squareSize should be in the same units (typically mm)
    const float spacing = p->geometry.squareSize;
	const cv::Size& patternSize = p->getGeometry().patternSize();

    for (int i = 0; i < patternSize.height; ++i)
    {
        for (int j = 0; j < patternSize.width; ++j)
        {
            objectPoints.emplace_back(j * spacing, i * spacing, 0.0f);
        }
    }

    return objectPoints;
}

std::vector<cv::Point3f> Detector::generateCirclesObjectPoints() const
{
    const BoardPattern& boardPattern = detectionParams_->patternType();

    if (boardPattern != BoardPattern::SYMMETRIC_CIRCLES && boardPattern != BoardPattern::ASYMMETRIC_CIRCLES)
    {
        throw std::runtime_error("Detection parameters type mismatch: expected SYMMETRIC_CIRCLES or ASYMMETRIC_CIRCLES");
    }

    auto p = std::static_pointer_cast<CircleboardParameters>(detectionParams_);

    std::vector<cv::Point3f> objectPoints;

    // Use actual physical dimensions from board description
    // centerDistance should be in the same units (typically mm)
    const float spacing = p->geometry.centerDistance;
	const cv::Size& patternSize = p->getGeometry().patternSize();

    if (boardPattern == BoardPattern::SYMMETRIC_CIRCLES)
    {
        for (int i = 0; i < patternSize.height; ++i)
        {
            for (int j = 0; j < patternSize.width; ++j)
            {
                objectPoints.emplace_back(j * spacing, i * spacing, 0.0f);
            }
        }
    }
    else // ASYMMETRIC_CIRCLES
    {
        for (int i = 0; i < patternSize.height; ++i)
        {
            for (int j = 0; j < patternSize.width; ++j)
            {
                float x = (2 * j + (i % 2)) * spacing;
                float y = i * spacing;
                objectPoints.emplace_back(x, y, 0.0f);
            }
        }
    }

    return objectPoints;
}

std::vector<cv::Point3f> Detector::generateAprilTagObjectPoints(
    const std::vector<int>& tagIds,
    const std::shared_ptr<AprilTagParameters>& p) const
{
    const std::vector<std::vector<cv::Point3f>>& allObjPoints = p->geometry.aprilTag.getObjPoints();
    // allObjPoints[i] holds the 4 corners of the i-th marker in board space,
    // indexed by marker index.

    std::vector<cv::Point3f> objectPoints;
    objectPoints.reserve(tagIds.size() * 4);
    for (int id : tagIds)
    {
        if (id >= 0 && id < static_cast<int>(allObjPoints.size()))
        {
            for (const auto& pt : allObjPoints[id])
                objectPoints.push_back(pt);
        }
    }

    return objectPoints;
}

float Detector::computeBoardMarginWorld() const
{
    switch (detectionParams_->patternType())
    {
    case BoardPattern::CHESSBOARD:
        return std::static_pointer_cast<ChessboardParameters>(detectionParams_)->geometry.squareSize * 1.5f;

    case BoardPattern::SYMMETRIC_CIRCLES:
    case BoardPattern::ASYMMETRIC_CIRCLES:
    {
        auto p = std::static_pointer_cast<CircleboardParameters>(detectionParams_);

        const float halfCenterDistance = p->geometry.centerDistance * 0.5f;
        const float radius = p->geometry.markDiameter * 0.5f;

        return halfCenterDistance + radius * 0.5f;
    }

    case BoardPattern::CHARUCO:
        return std::static_pointer_cast<CharucoParameters>(detectionParams_)->geometry.charuco.getSquareLength() * 1.5f;

    case BoardPattern::APRIL_TAG:
        return std::static_pointer_cast<AprilTagParameters>(detectionParams_)->geometry.aprilTag.getMarkerLength() * 1.5f;

    default:
        throw std::runtime_error("Unsupported pattern");
    }
}

CvContour Detector::generateBoardContour(
    const std::vector<cv::Point2f>& imagePoints,
    const std::vector<cv::Point3f>& worldPoints) const
{
    if (imagePoints.size() < 4)
        throw std::runtime_error("Not enough points for homography");

    // 1. World -> 2D (planar board)
    std::vector<cv::Point2f> world2d(worldPoints.size());
    for (size_t i = 0; i < worldPoints.size(); ++i)
    {
        world2d[i] = { worldPoints[i].x, worldPoints[i].y };
    }

    // 2. Homography world -> image
    cv::Mat H = cv::findHomography(world2d, imagePoints, 0, 3.0);   // 3 px reproj threshold

    if (H.empty())
        throw std::runtime_error("Homography estimation failed");

    // 3. Pattern bounding box in world coordinates
    float minX = std::numeric_limits<float>::max(),
        maxX = std::numeric_limits<float>::lowest(),
        minY = minX, maxY = maxX;

    for (const auto& wp : worldPoints)
    {
        minX = std::min(minX, wp.x);
        maxX = std::max(maxX, wp.x);
        minY = std::min(minY, wp.y);
        maxY = std::max(maxY, wp.y);
    }

    // 4. Add the "half feature-size"
    const float marginWorld = computeBoardMarginWorld();

    minX -= marginWorld;  maxX += marginWorld;
    minY -= marginWorld;  maxY += marginWorld;

    std::vector<cv::Point2f> boardWorldCorners = {
        {minX, minY}, {maxX, minY},
        {maxX, maxY}, {minX, maxY}
    };

    // 5. Project to image
    std::vector<cv::Point2f> boardImageCorners(4);
    cv::perspectiveTransform(boardWorldCorners, boardImageCorners, H);

    return CvContour(std::move(boardImageCorners));
}

std::vector<CvContour> Detector::generateChessboardMarksContour(const std::vector<cv::Point2f>& corners) const
{
    if (corners.size() < 4)
    {
        throw std::runtime_error("Wrong number of corners");
    }

	std::vector<CvContour> marksContours;
    const cv::Size& patternSize = detectionParams_->getGeometry().patternSize();

    // Generate contours for each square/cell in the pattern
    for (int i = 0; i < patternSize.height - 1; ++i)
    {
        for (int j = 0; j < patternSize.width - 1; ++j)
        {
            int idx0 = i * patternSize.width + j;
            int idx1 = idx0 + 1;
            int idx2 = idx0 + patternSize.width + 1;
            int idx3 = idx0 + patternSize.width;

            std::vector<cv::Point2f> square = {
                corners[idx0],
                corners[idx1],
                corners[idx2],
                corners[idx3]
            };

            marksContours.push_back(CvContour(square));
        }
    }

	return marksContours;
}

std::vector<CvContour> Detector::generateCirclesMarksContour(
    const std::vector<cv::Point2f>& centers,
    const std::vector<cv::Point3f>& objectPoints,
    const cv::Mat& rvec,
    const cv::Mat& tvec) const
{
    if (centers.size() != objectPoints.size())
        throw std::runtime_error("Mismatch between detected centers and object points");

    auto p = std::static_pointer_cast<CircleboardParameters>(detectionParams_);

    std::vector<CvContour> marksContours;

    const float realRadius = p->geometry.markDiameter * 0.5f;  // Real-world radius (e.g., in mm)
    const int segments = 16;  // Number of points for polygon approximation
    static constexpr float CV_PIf = static_cast<float>(CV_PI);

    cv::Mat cameraMatrix = camIntrinsics_.cameraMatrix();
    cv::Mat distCoeffs = camIntrinsics_.distortionModel().coeffs();

    for (size_t i = 0; i < objectPoints.size(); ++i)
    {
        const cv::Point3f& center3D = objectPoints[i];

        // Project just the 3D center to find the perspective-induced offset
        std::vector<cv::Point3f> centerVec = { center3D };
        std::vector<cv::Point2f> projectedCenter;
        cv::projectPoints(centerVec, rvec, tvec, cameraMatrix, distCoeffs, projectedCenter);

        // Offset between projected 3D center and actual detected centroid
        cv::Point2f offset = centers[i] - projectedCenter[0];

        // Generate and project circumference points as before
        std::vector<cv::Point3f> circlePoints3D(segments);
        for (int k = 0; k < segments; ++k)
        {
            float theta = 2.0f * CV_PIf * k / segments;
            circlePoints3D[k] = cv::Point3f(
                center3D.x + realRadius * std::cos(theta),
                center3D.y + realRadius * std::sin(theta),
                center3D.z
            );
        }

        std::vector<cv::Point2f> projectedPoints;
        cv::projectPoints(circlePoints3D, rvec, tvec, cameraMatrix, distCoeffs, projectedPoints);

        // Apply the offset to re-center the contour on the detected centroid
        for (auto& pt : projectedPoints)
            pt += offset;

        marksContours.push_back(CvContour(projectedPoints));
    }

    return marksContours;
}

std::vector<CvContour> Detector::generateQuadMarksContour(const std::vector<std::vector<cv::Point2f>>& markerCorners) const
{
    std::vector<CvContour> contours;
    contours.reserve(markerCorners.size());

    for (const auto& quad : markerCorners)
        contours.emplace_back(quad);   // quad is already the 4 image-space corners

    return contours;
}

std::vector<cv::Point2f> Detector::generateAxes2D(
    const std::vector<cv::Point3f>& objectPoints,
    const cv::Mat& rvec,
    const cv::Mat& tvec)
{
    if (objectPoints.empty())
        throw std::runtime_error("objectPoints is empty");

    cv::Size patternSize = detectionParams_->getGeometry().patternSize();

    float gridStep = cv::norm(objectPoints[1] - objectPoints[0]);

    // Board span in grid units
    int shorterSide = std::min(patternSize.width, patternSize.height);

    // Axes reach half the shorter board dimension
    float multiplier = static_cast<float>(shorterSide) / 2.f;
    multiplier = multiplier < 1.f ? std::ceil(multiplier) : std::floor(multiplier);

    float axesLength = gridStep * multiplier;

    // 3D centroid of the pattern in world space
    cv::Point3f centroid3D(0.f, 0.f, 0.f);
    for (const auto& p : objectPoints) centroid3D += p;

    centroid3D *= 1.f / static_cast<float>(objectPoints.size());

    std::vector<cv::Point3f> axesPoints3D = {
        centroid3D,
        centroid3D + cv::Point3f{ axesLength, 0.f, 0.f },
        centroid3D + cv::Point3f{ 0.f, axesLength, 0.f },
        centroid3D + cv::Point3f{ 0.f, 0.f, axesLength },
    };

    std::vector<cv::Point2f> axes2D;
    cv::projectPoints(
        axesPoints3D,
        rvec, tvec,
        camIntrinsics_.cameraMatrix(),
        camIntrinsics_.distortionModel().coeffs(),
        axes2D
    );

    return axes2D;
}
