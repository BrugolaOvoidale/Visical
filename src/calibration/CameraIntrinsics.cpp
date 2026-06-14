#include "CameraIntrinsics.hpp"
#include <stdexcept>


CameraIntrinsics::CameraIntrinsics(
    const cv::Size& imageSize,
    const cv::Mat& camMatrix,
    DistortionModel distModel)
    : distModel_(distModel)
{
    validateCamMatrix(camMatrix);

    imageSize_ = imageSize;

    cameraMatrix_ = camMatrix.clone();

    mode_ = CameraIntrinsics::Mode::FREE;
}

CameraIntrinsics::CameraIntrinsics(
    int imageWidth,
    int imageHeight,
    const cv::Mat& camMatrix,
    DistortionModel distModel)
    : distModel_(distModel)
{
    validateCamMatrix(camMatrix);

    imageSize_ = { imageWidth, imageHeight };

    cameraMatrix_ = camMatrix.clone();

    mode_ = CameraIntrinsics::Mode::FREE;
}

CameraIntrinsics::CameraIntrinsics(
    Mode mode,
    const cv::Size& imageSize,
    const cv::Mat& camMatrix,
    DistortionModel distModel)
    : distModel_(distModel)
{
    validateCamMatrix(camMatrix);

    imageSize_ = imageSize;

    cameraMatrix_ = camMatrix.clone();

    mode_ = mode;
}

CameraIntrinsics::CameraIntrinsics(
    Mode mode,
    int imageWidth,
    int imageHeight,
    const cv::Mat& camMatrix,
    DistortionModel distModel)
    : distModel_(distModel)
{
    validateCamMatrix(camMatrix);

    imageSize_ = { imageWidth, imageHeight };

    cameraMatrix_ = camMatrix.clone();

    mode_ = mode;
}

///////////////////////////////////////////////////////////

CameraIntrinsics CameraIntrinsics::automatic(
    const cv::Size& imageSize,
    DistortionModel distModel)
{
    double max = std::max(imageSize.width, imageSize.height);

    double fx = max;
    double fy = max;
    double cx = imageSize.width / 2.0;
    double cy = imageSize.height / 2.0;

    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1
        );

    return CameraIntrinsics(Mode::AUTOMATIC, imageSize, cameraMatrix, distModel);
}

CameraIntrinsics CameraIntrinsics::automatic(
    int imageWidth,
    int imageHeight,
    DistortionModel distModel)
{
    double max = std::max(imageWidth, imageHeight);

    double fx = max;
    double fy = max;
    double cx = imageWidth / 2.0;
    double cy = imageHeight / 2.0;

    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1
        );

    return CameraIntrinsics(Mode::AUTOMATIC, imageWidth, imageHeight, cameraMatrix, distModel);
}

CameraIntrinsics CameraIntrinsics::fromHardware(
    const cv::Size& imageSize,
    double focalLength,
    const cv::Size2d& pixelPitch,
    DistortionModel distModel)
{
    double fx = focalLength / pixelPitch.width;
    double fy = focalLength / pixelPitch.height;
    double cx = imageSize.width / 2.0;
    double cy = imageSize.height / 2.0;

    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1
        );

    return CameraIntrinsics(Mode::FROM_HARDWARE, imageSize, cameraMatrix, distModel);
}

CameraIntrinsics CameraIntrinsics::fromHardware(
    int imageWidth,
    int imageHeight,
    double focalLength,
    double pixelPitchWidth,
    double pixelPitchHeight,
    DistortionModel distModel)
{
    double fx = focalLength / pixelPitchWidth;
    double fy = focalLength / pixelPitchHeight;
    double cx = imageWidth / 2.0;
    double cy = imageHeight / 2.0;

    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1
        );

    return CameraIntrinsics(Mode::FROM_HARDWARE, imageWidth, imageHeight, cameraMatrix, distModel);
}

CameraIntrinsics CameraIntrinsics::free(
    const cv::Size& imageSize,
    double fx,
    double fy,
    double cx,
    double cy,
    DistortionModel distModel)
{
    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1
        );

    return CameraIntrinsics(Mode::FREE, imageSize, cameraMatrix, distModel);
}

CameraIntrinsics CameraIntrinsics::free(
    int imageWidth,
    int imageHeight,
    double fx,
    double fy,
    double cx,
    double cy,
    DistortionModel distModel)
{
    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) <<
        fx, 0, cx,
        0, fy, cy,
        0, 0, 1
        );

    return CameraIntrinsics(Mode::FREE, imageWidth, imageHeight, cameraMatrix, distModel);
}

///////////////////////////////////////////////////////////

void CameraIntrinsics::validateCamMatrix(const cv::Mat& camMatrix)
{
    // 1. Check that Mat is not empty
    if (camMatrix.empty())
    {
        throw std::invalid_argument("Camera matrix Mat is empty");
    }

    // 2. Check dimensions, should be a 3x3
    if (camMatrix.rows != 3 || camMatrix.cols != 3)
    {
        throw std::invalid_argument("Camera matrix must be a 3x3 matrix");
    }

    // 3. Check data type, should be floating point
    if (camMatrix.type() != CV_32F && camMatrix.type() != CV_64F)
    {
        throw std::invalid_argument("Camera matrix must be CV_32F or CV_64F");
    }
}

const cv::Size& CameraIntrinsics::imageSize() const
{
    return imageSize_;
}

const cv::Mat& CameraIntrinsics::cameraMatrix() const
{
    return cameraMatrix_;
}

const DistortionModel& CameraIntrinsics::distortionModel() const
{
    return distModel_;
}

CameraIntrinsics::Mode CameraIntrinsics::mode() const
{
    return mode_;
}
