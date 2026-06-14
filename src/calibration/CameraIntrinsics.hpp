#pragma once
#include "DistortionModel.hpp"


/**
 * @brief Manages the internal parameters of a pinhole camera model.
 *
 * This class handles the camera matrix and associated distortion coefficients.
 * It supports different initialization strategies (modes) to provide starting
 * guesses for the calibration process.
 */
class CameraIntrinsics
{
public:
    //-----------------------------------------------------------------------------
    // Enums
    //-----------------------------------------------------------------------------

    /**
     * @brief Specifies how the intrinsics were generated.
     */
    enum class Mode {
        AUTOMATIC,          ///< Estimated based on image dimensions.
        FROM_HARDWARE,      ///< Guess based on focal length and sensor physical specs.
        FREE                ///< Manually defined parameters.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Assigns and validates an existing cv::Mat.
     * @param imageSize Dimensions of the calibration image.
     * @param camMatrix A 3x3 matrix.
     * @param distModel Associated distortion model.
     */
    CameraIntrinsics(
        const cv::Size& imageSize,
        const cv::Mat& camMatrix,
        DistortionModel distModel
    );
    CameraIntrinsics(
        int imageWidth,
        int imageHeight,
        const cv::Mat& camMatrix,
        DistortionModel distModel
    );

    ~CameraIntrinsics() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------
    
    /**
     * @brief Factory for AUTOMATIC mode.
     * 
     * Assumes no hardware knowledge. Generates a safe starting point where:
     * f_x = f_y = max(width, height) and 
     * c_x, c_y are at the image center.
     */
    static CameraIntrinsics automatic(
        const cv::Size& imageSize,
        DistortionModel distModel
    );
    static CameraIntrinsics automatic(
        int imageWidth,
        int imageHeight,
        DistortionModel distModel
    );

    /**
     * @brief Factory for FROM_HARDWARE mode.
     * 
     * Uses physical sensor specs to place the optimization closer to reality.
     * f_x = focalLength / pixelPitchWidth
     * f_y = focalLength / pixelPitchHeight
     * 
     * @param focalLength Focal length in mm.
     * @param pixelPitch Width and Height of a single sensor cell in mm.
     */
    static CameraIntrinsics fromHardware(
        const cv::Size& imageSize,
        double focalLength,
        const cv::Size2d& pixelPitch,
        DistortionModel distModel
    );
    static CameraIntrinsics fromHardware(
        int imageWidth,
        int imageHeight,
        double focalLength,
        double sensorCellWidth,
        double sensorCellHeight,
        DistortionModel distModel
    );

    /**
     * @brief Factory for FREE mode.
     * 
     * Allows manual definition of all intrinsic parameters (f_x, f_y, c_x, c_y).
     */
    static CameraIntrinsics free(
        const cv::Size& imageSize,
        double fx,
        double fy,
        double cx,
        double cy,
        DistortionModel distModel
    );
    static CameraIntrinsics free(
        int imageWidth,
        int imageHeight,
        double fx,
        double fy,
        double cx,
        double cy,
        DistortionModel distModel
    );


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Checks if a camera matrix is mathematically valid (3x3, numeric).
     * @throw std::invalid_argument if the matrix is malformed.
     */
    static void validateCamMatrix(const cv::Mat& camMatrix);

    /** @brief Returns the image dimensions in pixels. */
    const cv::Size& imageSize() const;

    /** @brief Returns the 3x3 intrinsic camera matrix. */
    const cv::Mat& cameraMatrix() const;

    /** @brief Returns the distortion model (coefficients and type). */
    const DistortionModel& distortionModel() const;

    /** @brief Returns the mode used to create these intrinsics. */
    Mode mode() const;

private:
    // Internal constructor used by factories
    CameraIntrinsics(
        Mode mode,
        const cv::Size& imageSize,
        const cv::Mat& camMatrix,
        DistortionModel distModel
    );

    // Internal constructor used by factories
    CameraIntrinsics(
        Mode mode,
        int imageWidth,
        int imageHeight,
        const cv::Mat& camMatrix,
        DistortionModel distModel
    );

private:
    Mode mode_;

    // Image dimensions in pixels
    cv::Size imageSize_;

    // Intrinsics matrix
    cv::Mat cameraMatrix_;

	// Distortion model (coefficients and type)
    DistortionModel distModel_;
};

/** @brief String representation mapping for CameraIntrinsics::Mode. */
template <>
struct EnumTraits<CameraIntrinsics::Mode> {
    static constexpr std::array<std::pair<CameraIntrinsics::Mode, const char*>, 3> values{ {
    {CameraIntrinsics::Mode::AUTOMATIC, "automatic"},
    {CameraIntrinsics::Mode::FROM_HARDWARE, "from_hardware"},
    {CameraIntrinsics::Mode::FREE, "free"}
    } };
};