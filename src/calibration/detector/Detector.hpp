#pragma once
#include <opencv2/core/mat.hpp>
#include <memory>
#include <cv/CvContour.hpp>
#include "../CameraIntrinsics.hpp"


// Forward declarations
class PatternParams;
class ChessboardParams;
class CircleboardParams;
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
        ChessboardParams detParams
    );

    /**
     * @brief Constructor for circleboard detection.
     * @param camIntrinsics Object containing the parameters of the camera.
     * @param detParams Object containing the circleboard detection parameters.
     */
    Detector(
        CameraIntrinsics camIntrinsics,
        CircleboardParams detParams
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
    void setDetectionParameters(ChessboardParams detParams);

    /**
     * @brief Configures the detector for Circle-grid patterns.
     * @param detParams Parameters specifically for symmetric/asymmetric circle patterns.
     */
    void setDetectionParameters(CircleboardParams detParams);

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

    // Low-level OpenCV chessboard detection wrapper.
    FindBoardResult findChessboard(const cv::Mat& input) const;

    // Low-level OpenCV circle grid detection wrapper.
    FindBoardResult findCircleboard(const cv::Mat& input) const;

    // Generate 3D object points for the calibration pattern using actual physical dimensions
    std::vector<cv::Point3f> generateObjectPoints() const;

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
    std::shared_ptr<PatternParams> detectionParams_;
};
