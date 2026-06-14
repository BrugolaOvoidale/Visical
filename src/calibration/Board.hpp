#pragma once
#include <cv/CvImage.hpp>
#include <cv/CvContour.hpp>
#include "BoardPattern.hpp"


/**
 * @brief Represents the result of a calibration board detection process.
 *
 * This class stores the geometric and topological information of a detected board,
 * including its 3D pose relative to the camera. It uses a state-based design
 * where the object can represent a successful detection, a partial detection,
 * or a total failure via ErrorCode.
 */
class Board
{
public:
    //-----------------------------------------------------------------------------
    // Enums and support classes
    //-----------------------------------------------------------------------------

    /**
     * @brief Result status of the board detection algorithm.
     */
    enum class ErrorCode {
        NONE,               ///< Detection and pose estimation successful.
        NOT_DETECTED,       ///< No board found in the image.
        POSE_NOT_SOLVED     ///< Board features found, but PnP solver failed.
    };

    /**
     * @brief Wrapper for the extrinsic parameters (Rotation and Translation).
     */
    struct Pose
    {
        Pose() = default;

        /**
         * @brief Construct pose from OpenCV vectors.
         * @param rvec 3x1 Rotation vector (Rodrigues).
         * @param tvec 3x1 Translation vector.
         */
        Pose(const cv::Mat& r, const cv::Mat& t) : rvec_(r), tvec_(t) {}


        /** @brief Returns the Rotation matrix. */
        const cv::Mat& rvec() const { return rvec_; }

        /** @brief Returns the Translation matrix. */
        const cv::Mat& tvec() const { return tvec_; }

    private:
        // Rotation vector (3x1)
        const cv::Mat rvec_;

        // Translation vector (3x1)
        const cv::Mat tvec_;
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Full constructor for a successfully detected and solved board. Sets ErrorCode to NONE.
     * @param image The source image.
     * @param patternType The type of board.
     * @param patternSize Number of inner corners (cols, rows).
     * @param boardContour The boundary of the entire board.
     * @param marksContours Individual contours for each mark/corner.
     * @param imagePoints Detected 2D coordinates in pixels.
     * @param objectPoints Corresponding 3D coordinates in the board's local space.
     * @param rvec Rotation vector.
     * @param tvec Translation vector.
     * @param axes2D Projected 2D points representing the XYZ coordinate axes.
     */
    Board(
        const CvImage& image,
		BoardPattern patternType,
        cv::Size patternSize,
        CvContour boardContour,
        std::vector<CvContour> marksContours,
        std::vector<cv::Point2f> imagePoints,
        std::vector<cv::Point3f> objectPoints,
        const cv::Mat& rvec,
        const cv::Mat& tvec,
        std::vector<cv::Point2f> axes2D
    );

    ~Board() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory for when no board features are not found.
     * @param image The source image.
     * @param patternType The type of board.
     * @param patternSize Number of inner corners (cols, rows).
     * @return A Board object with ErrorCode::NOT_DETECTED.
     */
    static Board notDetected(
        const CvImage& image,
        BoardPattern patternType,
        cv::Size patternSize
    );

    /**
     * @brief Factory for when no board features are found, but pose estimation failed.
     * @param image The source image.
     * @param patternType The type of board.
     * @param patternSize Number of inner corners (cols, rows).
     * @param boardContour The boundary of the entire board.
     * @param marksContours Individual contours for each mark/corner.
     * @param imagePoints Detected 2D coordinates in pixels.
     * @param objectPoints Corresponding 3D coordinates in the board's local space.
     * @return Board with ErrorCode::POSE_NOT_SOLVED.
     */
    static Board poseNotSolved(
        const CvImage& image,
        BoardPattern patternType,
        cv::Size patternSize,
        CvContour boardContour,
        std::vector<cv::Point2f> imagePoints,
        std::vector<cv::Point3f> objectPoints
    );


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    const CvImage& image() const;

    BoardPattern patternType() const;

    cv::Size patternSize() const;

    const CvContour& boardContour() const;

    const std::vector<CvContour>& marksContours() const;

    /** @brief Gets the 2D pixel coordinates of the detected features. */
    const std::vector<cv::Point2f>& imagePoints() const;

    /** @brief Gets the 3D world coordinates of the board features. */
    const std::vector<cv::Point3f>& objectPoints() const;

    /** @brief Gets the calculated 3D pose (Extrinsics). */
    const Pose& pose() const;

    /** @brief Gets the projected 2D image points of the board's coordinate axes. */
    const std::vector<cv::Point2f>& axes() const;

    /** @brief Returns the error status of the board detection. */
    ErrorCode errorCode() const;

    /** @brief Helper to check if the board was detected. */
    bool isDetected() const;

    /** @brief Helper to check if the board pose was solved. */
    bool hasPose() const;

private:
	// Private default constructor to enforce use of parameterized constructors and factories.
    Board() = default;

private:
    CvImage image_;

	BoardPattern patternType_;

	cv::Size patternSize_;

    CvContour boardContour_;

    std::vector<CvContour> marksContours_;

    std::vector<cv::Point2f> imagePoints_;      // 2D observed points

    std::vector<cv::Point3f> objectPoints_;     // 3D world points

	Pose pose_;                                 // Rotation and translation vectors

    std::vector<cv::Point2f> axes2D_;

    ErrorCode errorCode_;
};
