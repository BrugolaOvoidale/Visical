#include "CvRegion.hpp"
#include "CvContour.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <stdexcept>


static void pasteMask(
    cv::Mat& canvas,
    const cv::Mat& src,
    const cv::Rect& srcBounds,
    const cv::Rect& canvasBounds)
{
    cv::Rect intersect =
        srcBounds & canvasBounds;

    if (intersect.empty())
        return;

    cv::Rect srcROI(
        intersect.x - srcBounds.x,
        intersect.y - srcBounds.y,
        intersect.width,
        intersect.height);

    cv::Rect dstROI(
        intersect.x - canvasBounds.x,
        intersect.y - canvasBounds.y,
        intersect.width,
        intersect.height);

    src(srcROI).copyTo(canvas(dstROI));
}

/////////////////////////////////////////////////////////////////////////

CvRegion::CvRegion(const CvRegion& other)
    : mask_(other.mask_),
    boundingBox_(other.boundingBox_),
    cachedLabels_(other.cachedLabels_),
    cachedNLabels_(other.cachedNLabels_),
    cacheLabelsValid_(other.cacheLabelsValid_),
    cachedPoints_(other.cachedPoints_),
    cachePtsValid_(other.cachePtsValid_)
{
}

CvRegion CvRegion::clone() const
{
    CvRegion r;

	r.mask_ = mask_.clone();
    r.boundingBox_ = boundingBox_;
    r.cachedLabels_ = cachedLabels_.clone();
    r.cachedNLabels_ = cachedNLabels_;
    r.cacheLabelsValid_ = cacheLabelsValid_;
    r.cachedPoints_ = cachedPoints_;
	r.cachePtsValid_ = cachePtsValid_;

    return r;
}

CvRegion::CvRegion(
    const cv::Mat& m,
    const cv::Rect& bbox)
{
    if (m.type() != CV_8UC1)
        throw std::invalid_argument("CvRegion mask must be CV_8UC1");

    // Enforce binary values (0 or 255 only)
    double minVal = 0.0, maxVal = 0.0;
    cv::minMaxLoc(mask_, &minVal, &maxVal);

    if (minVal < 0.0 || maxVal > 255.0 || (minVal != 0.0 && minVal != 255.0) ||
        (maxVal != 0.0 && maxVal != 255.0))
    {
        throw std::invalid_argument("mask must contain only 0 and 255 values");
    }

    mask_ = m;
    boundingBox_ = bbox;
}

/////////////////////////////////////////////////////////////////////////

CvRegion CvRegion::fromPolygon(const std::vector<cv::Point>& polygon)
{
    if (polygon.empty())
        return {};

    // 1. Compute bounding box in global coordinates
    cv::Rect bbox = cv::boundingRect(polygon);

    // 2. Create compact local mask
    cv::Mat mask = cv::Mat::zeros(bbox.size(), CV_8UC1);

    // 3. Shift polygon into local coordinates
    std::vector<cv::Point> localPoly;
    localPoly.reserve(polygon.size());

    for (const auto& p : polygon)
    {
        localPoly.emplace_back(p.x - bbox.x, p.y - bbox.y);
    }

    // 4. Fill polygon
    std::vector<std::vector<cv::Point>> polys = { localPoly };
    cv::fillPoly(mask, polys, cv::Scalar(255));

    // 5. Return region with bounds
    return CvRegion(mask, bbox);
}

CvRegion CvRegion::fromPoints(const std::vector<cv::Point2f>& points)
{
    if (points.empty())
        return {};

    // 1. Compute bounding box in global coordinates
    cv::Rect bbox = cv::boundingRect(points);

    // 2. Create compact local mask
    cv::Mat mask = cv::Mat::zeros(bbox.size(), CV_8UC1);

    // 3. Rasterize contours points into local coordinates
    for (const auto& p : points)
    {
        int row = std::min(cvFloor(p.y) - bbox.y, mask.rows - 1);
        int col = std::min(cvFloor(p.x) - bbox.x, mask.cols - 1);
        mask.at<uchar>(row, col) = 255;
    }

    return CvRegion(mask, bbox);
}

CvRegion CvRegion::fromPoints(const std::vector<std::vector<cv::Point2f>>& points)
{
    if (points.empty())
        return {};

    std::vector<cv::Point2f> flattenPts;

    for (const auto& innerVec : points)
    {
        flattenPts.insert(flattenPts.end(), innerVec.begin(), innerVec.end());
    }

    return fromPoints(flattenPts);
}

CvRegion CvRegion::fromRectangle(
    int row1, int col1,
    int row2, int col2)
{
    // 1. Normalize coordinates (important if inverted input is possible)
    int rMin = std::min(row1, row2);
    int rMax = std::max(row1, row2);
    int cMin = std::min(col1, col2);
    int cMax = std::max(col1, col2);

    // 2. Define bounding box (global coordinates)
    cv::Rect bbox(
        cMin,
        rMin,
        cMax - cMin + 1,
        rMax - rMin + 1
    );

    // 3. Create compact mask
    cv::Mat mask = cv::Mat::ones(bbox.size(), CV_8UC1) * 255;

    return CvRegion(mask, bbox);
}

CvRegion CvRegion::fromContours(
    const std::vector<CvContour>& contours,
    bool filled)
{
    if (contours.empty())
        return {};

    // 1. Compute global bounding box
    cv::Rect bbox = contours[0].boundingBox();
    for (size_t i = 1; i < contours.size(); ++i)
    {
        bbox |=  contours[i].boundingBox();
    }

    // 2. Create compact mask
    cv::Mat mask = cv::Mat::zeros(bbox.size(), CV_8UC1);

    // 3. Convert contours to local coordinates
    std::vector<std::vector<cv::Point>> localContours;
    localContours.reserve(contours.size());

    for (const auto& contour : contours)
    {
        std::vector<cv::Point> pts;
        pts.reserve(contour.points().size());

        for (const auto& p : contour.points())
        {
            pts.emplace_back(
                cvRound(p.x - bbox.x),
                cvRound(p.y - bbox.y));
        }

        localContours.push_back(std::move(pts));
    }

    // 4. Rasterize
    if (filled)
    {
        cv::fillPoly(mask, localContours, cv::Scalar(255));
    }
    else
    {
        cv::polylines(mask, localContours, true, cv::Scalar(255), 1);
    }

    // 5. Return compact region with bounds
    return CvRegion(mask, bbox);
}

CvRegion CvRegion::fromContours(
    const std::vector<std::vector<CvContour>>& contours,
    bool filled)
{
	std::vector<CvContour> flatContours;
	for (const auto& vec : contours)
	{
		for (const auto& c : vec)
		{
			flatContours.push_back(c);
		}
	}

	return fromContours(flatContours, filled);
}

/////////////////////////////////////////////////////////////////////////

CvRegion CvRegion::operator&(const CvRegion& other) const
{
    cv::Rect bbox = boundingBox_ | other.boundingBox_;

    cv::Mat a = cv::Mat::zeros(bbox.size(), CV_8UC1);
    cv::Mat b = cv::Mat::zeros(bbox.size(), CV_8UC1);

    pasteMask(a, mask_, boundingBox_, bbox);
    pasteMask(b, other.mask_, other.boundingBox_, bbox);

    cv::Mat result;
    cv::bitwise_and(a, b, result);

    return CvRegion(result, bbox);
}

CvRegion CvRegion::operator|(const CvRegion& other) const
{
    cv::Rect bbox = boundingBox_ | other.boundingBox_;

    cv::Mat a = cv::Mat::zeros(bbox.size(), CV_8UC1);
    cv::Mat b = cv::Mat::zeros(bbox.size(), CV_8UC1);

    pasteMask(a, mask_, boundingBox_, bbox);
    pasteMask(b, other.mask_, other.boundingBox_, bbox);

    cv::Mat result;
    cv::bitwise_or(a, b, result);

    return CvRegion(result, bbox);
}

CvRegion CvRegion::operator-(const CvRegion& other) const
{
    cv::Rect bbox = boundingBox_ | other.boundingBox_;

    cv::Mat a = cv::Mat::zeros(bbox.size(), CV_8UC1);
    cv::Mat b = cv::Mat::zeros(bbox.size(), CV_8UC1);

    pasteMask(a, mask_, boundingBox_, bbox);
    pasteMask(b, other.mask_, other.boundingBox_, bbox);

    cv::Mat result;
    cv::bitwise_and(a, ~b, result);

    return CvRegion(result, bbox);
}

/////////////////////////////////////////////////////////////////////////

cv::Size CvRegion::size() const
{
    return cv::Size(mask_.cols, mask_.rows);
}

const cv::Rect& CvRegion::boundingBox() const
{
    return boundingBox_;
}

double CvRegion::areaCenter(double* row, double* col) const
{
    const double area = static_cast<double>(cv::countNonZero(mask_));

    if (row || col)
    {
        cv::Moments m = cv::moments(mask_, true);

        if (m.m00 > 0)
        {
            double localRow = m.m01 / m.m00;
            double localCol = m.m10 / m.m00;

            // convert to global coordinates
            if (row) *row = localRow + boundingBox_.y;
            if (col) *col = localCol + boundingBox_.x;
        }
        else
        {
            if (row) *row = 0.0;
            if (col) *col = 0.0;
        }
    }

    return area;
}

int CvRegion::countConnectedComponents() const
{
    computeConnectedComponents();
    return cachedNLabels_ - 1; // exclude background
}

std::vector<CvRegion> CvRegion::connectedComponents() const
{
    computeConnectedComponents();

    std::vector<CvRegion> components;
    components.reserve(cachedNLabels_ > 1 ? cachedNLabels_ - 1 : 0);

    for (int label = 1; label < cachedNLabels_; ++label)
    {
        int x = cachedStats_.at<int>(label, cv::CC_STAT_LEFT);
        int y = cachedStats_.at<int>(label, cv::CC_STAT_TOP);
        int w = cachedStats_.at<int>(label, cv::CC_STAT_WIDTH);
        int h = cachedStats_.at<int>(label, cv::CC_STAT_HEIGHT);

        cv::Rect bbox(x, y, w, h);

        cv::Mat localMask = cv::Mat::zeros(bbox.size(), CV_8UC1);

        // single pass over bbox only
        for (int row = 0; row < h; ++row)
        {
            const int* labelRow = cachedLabels_.ptr<int>(y + row);

            uchar* outRow = localMask.ptr<uchar>(row);

            for (int col = 0; col < w; ++col)
            {
                if (labelRow[x + col] == label)
                    outRow[col] = 255;
            }
        }

        components.emplace_back(localMask, bbox);
    }

    return components;
}

double CvRegion::diameterRegionMean() const
{
    std::vector<CvRegion> components = connectedComponents();

    double sumDiam = 0.0;
    int count = 0;

    for (const auto& comp : components)
    {
        std::vector<cv::Point> pts = comp.points();

        cv::Point2f center;
        float radius = 0.0f;

        cv::minEnclosingCircle(pts, center, radius);

        sumDiam += 2.0 * radius;
        ++count;
    }

    return (count > 0) ? (sumDiam / count) : 0.0;
}

CvRegion CvRegion::dilationCircle(double radius) const
{
    int r = static_cast<int>(std::ceil(radius));
    if (mask_.empty())
        return {};

    // 1. Create structuring element
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_ELLIPSE,
        cv::Size(2 * r + 1, 2 * r + 1));

    // 2. Pad mask by r on all sides BEFORE dilating,
    //    otherwise border pixels get clipped during dilation
    cv::Mat paddedMask;
    cv::copyMakeBorder(
        mask_, paddedMask,
        r, r, r, r,
        cv::BORDER_CONSTANT, cv::Scalar(0));

    // 3. Dilate the padded mask, now free to grow into the border
    cv::Mat dilatedMask;
    cv::dilate(paddedMask, dilatedMask, kernel);

    // 4. Expand bounding box to match the padded/dilated region
    cv::Rect expandedBounds = boundingBox_;
    expandedBounds.x -= r;
    expandedBounds.y -= r;
    expandedBounds.width += 2 * r;
    expandedBounds.height += 2 * r;

    return CvRegion(dilatedMask, expandedBounds);
}

CvRegion CvRegion::dilationRectangle(
    double width,
    double height) const
{
    int rw = static_cast<int>(std::ceil(width / 2.0));
    int rh = static_cast<int>(std::ceil(height / 2.0));
    if (mask_.empty())
        return {};

    // 1. Create structuring element
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT,
        cv::Size(2 * rw + 1, 2 * rh + 1));

    // 2. Pad mask asymmetrically to match the rectangular kernel extents
    cv::Mat paddedMask;
    cv::copyMakeBorder(
        mask_, paddedMask,
        rh, rh, rw, rw,
        cv::BORDER_CONSTANT, cv::Scalar(0)
    );

    // 3. Dilate the padded mask, now free to grow into the border
    cv::Mat dilatedMask;
    cv::dilate(paddedMask, dilatedMask, kernel);

    // 4. Expand bounding box to match the padded/dilated region
    cv::Rect expandedBounds = boundingBox_;
    expandedBounds.x -= rw;
    expandedBounds.y -= rh;
    expandedBounds.width += 2 * rw;
    expandedBounds.height += 2 * rh;

    return CvRegion(dilatedMask, expandedBounds);
}

CvRegion CvRegion::erosionRectangle(
    double width,
    double height) const
{
    int rw = static_cast<int>(std::ceil(width / 2.0));
    int rh = static_cast<int>(std::ceil(height / 2.0));

    // Safety check: if the mask is empty, or smaller than the erosion kernel, 
    // it will completely erode away into nothing.
    if (mask_.empty() || mask_.cols <= 2 * rw || mask_.rows <= 2 * rh)
        return {};

    // 1. Create structuring element
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT,
        cv::Size(2 * rw + 1, 2 * rh + 1));

    // 2. Erode the mask
    // We force cv::BORDER_CONSTANT with a value of 0. 
    // This treats the outside world as background, allowing erosion to eat into the mask edges.
    cv::Mat erodedMask;
    cv::erode(
        mask_, erodedMask, kernel,
        cv::Point(-1, -1), 1,
        cv::BORDER_CONSTANT, cv::Scalar(0)
    );

    // 3. Crop the mask to match the newly contracted region size
    // We trim 'rw' from the left/right and 'rh' from the top/bottom
    cv::Mat croppedMask = erodedMask(
        cv::Rect(rw, rh, mask_.cols - 2 * rw, mask_.rows - 2 * rh)
    ).clone();

    // 4. Contract the bounding box to match the cropped region
    cv::Rect contractedBounds = boundingBox_;
    contractedBounds.x += rw;
    contractedBounds.y += rh;
    contractedBounds.width -= 2 * rw;
    contractedBounds.height -= 2 * rh;

    return CvRegion(croppedMask, contractedBounds);
}

std::vector<CvContour> CvRegion::toContours(ContourMode mode) const
{
    if (mask_.empty())
        return {};

    cv::Mat binary;
    cv::threshold(mask_, binary, 0, 255, cv::THRESH_BINARY);

    int retrievalMode = (mode == ContourMode::BORDER_HOLES)
        ? cv::RETR_CCOMP
        : cv::RETR_EXTERNAL;

    int chainApprox = cv::CHAIN_APPROX_NONE;

    std::vector<std::vector<cv::Point>> rawContours;
    std::vector<cv::Vec4i> hierarchy;

    // NOTE: no dilation needed in a correct region model
    cv::findContours(binary, rawContours, hierarchy, retrievalMode, chainApprox);

    std::vector<CvContour> contours;
    contours.reserve(rawContours.size());

    for (auto& c : rawContours)
    {
        std::vector<cv::Point2f> contour;
        contour.reserve(c.size());

        for (const auto& p : c)
        {
            // shift from local -> global coordinates
            contour.emplace_back(
                static_cast<float>(p.x + boundingBox_.x),
                static_cast<float>(p.y + boundingBox_.y)
            );
        }

        contours.emplace_back(std::move(contour));
    }

    return contours;
}

const std::vector<cv::Point>& CvRegion::points() const
{
    computeRegionPoints();

    return cachedPoints_;
}

bool CvRegion::write(const std::string& path) const
{
    return cv::imwrite(path, mask_);
}

const cv::Mat& CvRegion::mask() const
{
    return mask_;
}

cv::Mat CvRegion::toMask(cv::Size imageSize) const
{
    cv::Mat full = cv::Mat::zeros(imageSize, CV_8UC1);
    mask_.copyTo(full(boundingBox_));
    return full;
}

/////////////////////////////////////////////////////////////////////////

void CvRegion::invalidateCache()
{
    cacheLabelsValid_ = false;
    cachePtsValid_ = false;
}

void CvRegion::computeConnectedComponents() const
{
    if (cacheLabelsValid_)
        return;

    if (mask_.empty())
    {
        cachedLabels_.release();
        cachedStats_.release();
        cachedNLabels_ = 0;
        cacheLabelsValid_ = true;
        return;
    }

    cachedNLabels_ =
        cv::connectedComponentsWithStats(
            mask_,
            cachedLabels_,
            cachedStats_,
            cachedCentroids_,
            8,
            CV_32S);

    cacheLabelsValid_ = true;
}

void CvRegion::computeRegionPoints() const
{
    if (cachePtsValid_)
        return;

    cachedPoints_.clear();

    if (mask_.empty())
    {
        cachePtsValid_ = true;
        return;
    }

    for (int y = 0; y < mask_.rows; ++y)
    {
        const uchar* row = mask_.ptr<uchar>(y);

        for (int x = 0; x < mask_.cols; ++x)
        {
            if (row[x])
            {
                cachedPoints_.emplace_back(
                    x + boundingBox_.x,
                    y + boundingBox_.y);
            }
        }
    }

    cachePtsValid_ = true;
}
