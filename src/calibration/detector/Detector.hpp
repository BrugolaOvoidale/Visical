#pragma once
#include <opencv2/core/mat.hpp>
#include <memory>
#include <cv/CvContour.hpp>
#include "../CameraIntrinsics.hpp"


// Forward declarations
class PatternParameters;
class ChessboardParameters;
class CircleboardParameters;
class CharucoParameters;
class AprilTagParameters;
class CvImage;
class Board;


/**
 * @brief High-level engine for detecting calibration patterns in images.
 * 
 * This class handles the detection of calibration boards.
 * It uses camera intrinsics to project between 2D image space and 3D world space,
 * ultimately producing a Board object containing the localized features.
 */
class Detector
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructor for chessboard detection.
     * @param camIntrinsics Object containing the parameters of the camera.
     * @param detParams Object containing the chessboard detection parameters.
     */
    Detector(
        CameraIntrinsics camIntrinsics,
        ChessboardParameters detParams
    );

    /**
     * @brief Constructor for circleboard detection.
     * @param camIntrinsics Object containing the parameters of the camera.
     * @param detParams Object containing the circleboard detection parameters.
     */
    Detector(
        CameraIntrinsics camIntrinsics,
        CircleboardParameters detParams
    );

    /**
     * @brief Constructor for ChArUco detection.
     * @param camIntrinsics Object containing the parameters of the camera.
     * @param detParams Object containing the ChArUco detection parameters.
     */
    Detector(
        CameraIntrinsics camIntrinsics,
        CharucoParameters detParams
    );

    /**
     * @brief Constructor for AprilTag detection.
     * @param camIntrinsics Object containing the parameters of the camera.
     * @param detParams Object containing the AprilTag detection parameters.
     */
    Detector(
        CameraIntrinsics camIntrinsics,
        AprilTagParameters detParams
    );
    
    ~Detector() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Configures the camera intrinsic parameters.
     * @param camIntrinsics Object containing the parameters of the camera.
     */
    void setCameraIntrinsics(CameraIntrinsics camIntrinsics);

    /**
     * @brief Configures the detector for Chessboard patterns.
     * @param detParams Parameters specifically for square-grid patterns.
     */
    void setDetectionParameters(ChessboardParameters detParams);

    /**
     * @brief Configures the detector for Circle-grid patterns.
     * @param detParams Parameters specifically for symmetric/asymmetric circle patterns.
     */
    void setDetectionParameters(CircleboardParameters detParams);

    /**
     * @brief Configures the detector for ChArUco patterns.
     * @param detParams Parameters specifically for ChArUco patterns.
     */
    void setDetectionParameters(CharucoParameters detParams);

    /**
     * @brief Configures the detector for AprilTag patterns.
     * @param detParams Parameters specifically for AprilTag patterns.
     */
    void setDetectionParameters(AprilTagParameters detParams);

    /**
     * @brief Processes an image to find and localize the calibration board.
     * @param image The input image.
     * @return A shared pointer of a Board object. Check Board::ErrorCode() to verify if detection status.
     * @throws If invalid parameters combinations or OpenCV throws
     */
    std::shared_ptr<Board> findBoard(const CvImage& image);

private:
    // Internal structure to hold raw detection results.
    struct FindBoardResult {
        // True if the pattern was successfully localized.
        bool found = false;

        // Detected 2D coordinates in image pixels.
		std::vector<cv::Point2f> corners;
    };

    // Internal structure to hold ArUco-based detection results.
    struct FindArucoResult : public FindBoardResult {
        std::vector<int> arucoIds;

        std::vector<std::vector<cv::Point2f>> markerCorners;

        std::vector<int> markerIds;
    };

    // Entirely separate pipeline from 'findBoard' for common boards.
    std::shared_ptr<Board> findCommonTypesBoard(const CvImage& iconicImage);

    // Entirely separate pipeline from 'findBoard' for ArUco-based boards.
    std::shared_ptr<Board> findArucoBasedBoard(const CvImage& iconicImage);

    // Low-level OpenCV chessboard detection wrapper.
    FindBoardResult findChessboard(const cv::Mat& input) const;

    // Low-level OpenCV circle grid detection wrapper.
    FindBoardResult findCircleboard(const cv::Mat& input) const;

    // Low-level OpenCV ChArUco detection wrapper.
    FindArucoResult findCharucoBoard(const cv::Mat& input) const;

    // Low-level OpenCV AprilTag detection wrapper.
    FindArucoResult findAprilTagBoard(const cv::Mat& input) const;

    std::vector<cv::Point3f> generateAprilTagObjectPoints(
        const std::vector<int>& tagIds,
        const std::shared_ptr<AprilTagParameters>& p
    ) const;

    // Builds one CvContour per detected ArUco marker, directly
    // from the already-detected 2D marker corners.
    std::vector<CvContour> generateQuadMarksContour(const std::vector<std::vector<cv::Point2f>>& markerCorners) const;

    // Generates 3D coordinates for chessboard corners
    std::vector<cv::Point3f> generateChessboardObjectPoints() const;

    // Generates 3D coordinates for circle centers
    std::vector<cv::Point3f> generateCirclesObjectPoints() const;

    // Calculates the safety margin around the board in world units (mm)
    float computeBoardMarginWorld() const;

    // Creates a contour representing the outer boundary of the board.
    CvContour generateBoardContour(
        const std::vector<cv::Point2f>& imagePoints,
        const std::vector<cv::Point3f>& worldPoints
    ) const;

    // Generates individual contours for each square on a chessboard.
    std::vector<CvContour> generateChessboardMarksContour(const std::vector<cv::Point2f>& corners) const;

    // Generate individual mark contours from detected corners
    std::vector<CvContour> generateCirclesMarksContour(
        const std::vector<cv::Point2f>& centers,
        const std::vector<cv::Point3f>& objectPoints,
        const cv::Mat& rvec,
        const cv::Mat& tvec
    ) const;

    // Generate the X, Y, Z 2D axes from the board center
    std::vector<cv::Point2f> generateAxes2D(
        const std::vector<cv::Point3f>& objectPoints,
        const cv::Mat& rvec,
        const cv::Mat& tvec
    );

private:
    // Camera parameters
    CameraIntrinsics camIntrinsics_;

    // Polymorphic storage for pattern settings.
    std::shared_ptr<PatternParameters> detectionParams_;
};
