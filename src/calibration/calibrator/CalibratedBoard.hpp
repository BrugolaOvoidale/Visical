#pragma once
#include <memory>
#include <opencv2/core/mat.hpp>


// Forward declarations
class Board;


/**
 * @brief Represents a physical board that has been localized in 3D space and
 * then calibrated.
 * 
 * This class stores the results of a camera-to-target calibration, including
 * the calibrated rotation and translation vectors and the associated
 * projection error metrics.
 */
class CalibratedBoard
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a calibrated board with specific pose and error data.
     * @param board A shared pointer of the detected board.
     * @param rvec Rotation vector (Rodrigues) from board to camera coords.
     * @param tvec Translation vector from board to camera coords.
     * @param RMSerror Root Mean Square error of the projection in pixels.
     * @param maxError Maximum observed projection error in pixels.
     * @throws std::invalid_argument if board is not detected.
     */
    CalibratedBoard(
        const std::shared_ptr<Board>& board,
        const cv::Mat& rvec,
        const cv::Mat& tvec,
        double RMSerror,
        double maxError
    );

    ~CalibratedBoard() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------
    
    /** @brief Returns the underlying detected board. */
    const std::shared_ptr<Board>& board() const;

    /** @brief Returns the Root Mean Square (RMS) reprojection error. */
    double RMSerror() const;

    /** @brief Returns the maximum reprojection error found across all points. */
    double maxError() const;

    /** @brief Returns the rotation vector (3x1 CV_64F). */
    const cv::Mat& rvec() const;

    /** @brief Returns the translation vector (3x1 CV_64F). */
    const cv::Mat& tvec() const;

private:
    // The physical board geometry.
    std::shared_ptr<Board> board_;

    // Rotation vector.
    cv::Mat rvec_;

    // Translation vector.
    cv::Mat tvec_;

    // Average reprojection accuracy.
    double RMSerror_;

    // Worst-case reprojection accuracy.
    double maxError_;
};
