#include "OpenCVCamera.hpp"
#include <cv/CvImage.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>


///////////////////////////////////////////////////////////////////////////////

static const std::unordered_map<cv::VideoCaptureProperties, std::string> ALLOWED_PROPERTIES {
    {cv::CAP_PROP_FRAME_WIDTH, "Width"},
    {cv::CAP_PROP_FRAME_HEIGHT, "Height"},
    {cv::CAP_PROP_FPS, "FPS"},
    {cv::CAP_PROP_EXPOSURE, "Exposure"},
    {cv::CAP_PROP_GAIN, "Gain"},
};

static const std::unordered_map<std::string, cv::VideoCaptureProperties> ALLOWED_PROPERTIES_STR{
    {"Width", cv::CAP_PROP_FRAME_WIDTH},
    {"Height", cv::CAP_PROP_FRAME_HEIGHT},
    {"FPS", cv::CAP_PROP_FPS},
    {"Exposure", cv::CAP_PROP_EXPOSURE},
    {"Gain", cv::CAP_PROP_GAIN},
};

///////////////////////////////////////////////////////////////////////////////


OpenCVCamera::OpenCVCamera(
    const std::string& serialNmb,
    const std::string& interfaceId)
    : ICamera(serialNmb, interfaceId, CameraInterface::OPENCV)
{
}

///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<OpenCVCamera> OpenCVCamera::create(
    const std::string& serialNmb,
    const std::string& interfaceId)
{
    struct Enabler : OpenCVCamera
    {
        Enabler(const std::string& serialNmb,
            const std::string& interfaceId)
            : OpenCVCamera(serialNmb, interfaceId)
        {
        }
    };

    return std::make_shared<Enabler>(serialNmb, interfaceId);
}

///////////////////////////////////////////////////////////////////////////////

TaskResult OpenCVCamera::checkCamera()
{
    bool success = false;
    std::vector<Log> logs;

    std::vector<std::string> deviceIds;

    deviceIds = OSInterface::listDeviceIds();
	const size_t numDevices = deviceIds.size();
    for (size_t i = 0 ; i < numDevices ; ++i)
    {
        if (deviceIds[i] == serialNumber())
        {
            deviceIndex_ = i;

            success = true;
            break;
        }
    }

    if (!success) {
        logs.push_back({ "Could not find device with Id: " + serialNumber(), Log::Level::LVL_ERROR });
    }

    return { success, logs };
}

TaskResult OpenCVCamera::checkConnectionHealth()
{
    if (!cap_.isOpened())
    {
        return false;
    }

    // Try to grab a frame
    cv::Mat frame;
    if (!cap_.read(frame) || frame.empty())
    {
        return { false, { "Failed to read frame from camera", Log::Level::LVL_ERROR } };
    }

    // Camera is healthy
    return true;
}

TaskResult OpenCVCamera::connectCamera()
{
    // Open camera
    if (!cap_.open(deviceIndex_, API))
    {
        return { false, { "Failed open camera", Log::Level::LVL_ERROR } };
    }

    return true;
}

TaskResult OpenCVCamera::loadCameraParams(std::vector<std::shared_ptr<Parameter>>& features)
{
    std::vector<Log> logs;

    features.clear();
    listAllFeatures(features, logs);

    bool success = !features.empty();
    return { success, logs };
}

TaskResult OpenCVCamera::disconnectCamera()
{
    if (!cap_.isOpened())
    {
        cap_.release();
    }

    return true;
}

TaskResultP<AcquisitionMode> OpenCVCamera::acquisitionMode() const
{
    if (!cap_.isOpened())
    {
        return {
            std::nullopt,
            { "VideoCapture is not opened", Log::Level::LVL_ERROR }
        };
    }

    // Try reading CAP_PROP_MODE
    double modeValue = cap_.get(cv::CAP_PROP_MODE);

    switch (static_cast<int>(modeValue))
    {
        // TODO:
        case 0:
            return AcquisitionMode::CONTINUOUS;

        case 1:
            return AcquisitionMode::SINGLE_FRAME;

        case 2:
            return AcquisitionMode::MULTI_FRAME;

        default:
            return {
                std::nullopt,
                { "Unknown acquisition mode", Log::Level::LVL_ERROR }
            };
    }
}

TaskResult OpenCVCamera::setAcquisitionMode(AcquisitionMode acqMode)
{
    if (!cap_.isOpened())
    {
        return {
            false,
            { "VideoCapture is not opened", Log::Level::LVL_ERROR }
        };
    }

    // OpenCV does not reliably support CAP_PROP_MODE, but we can attempt it
    // Most backends ignore this; it's mainly for reference
    double backendMode = 0;
    switch (acqMode)
    {
        case AcquisitionMode::SINGLE_FRAME:   backendMode = 1; break;

        case AcquisitionMode::MULTI_FRAME:    backendMode = 2; break;

        case AcquisitionMode::CONTINUOUS:     backendMode = 0; break;

        default:
            return {
                false,
                { "Unknown acquisition mode", Log::Level::LVL_ERROR }
            };
    }

    Log aLog;

    bool hasLog = false;
    // Attempt to set CAP_PROP_MODE (best effort, may be ignored)
    if (!cap_.set(cv::CAP_PROP_MODE, backendMode))
    {
        // log a warning but not an error

        aLog = { "CAP_PROP_MODE not supported by this backend", Log::Level::LVL_WARNING };

        hasLog = true;
    }

    return hasLog ? TaskResult{ true, std::move(aLog)} : true;
}

TaskResult OpenCVCamera::startFrameCapture()
{
    // Optional log diagnostics
    int width = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));
    double fps = cap_.get(cv::CAP_PROP_FPS);

    std::string diagMsg = "Stream started: width=" + std::to_string(width)
        + ", height=" + std::to_string(height)
        + ", fps=" + std::to_string(fps);

    acquiring_ = true;

    return { true, { std::move(diagMsg), Log::Level::LVL_DEBUG } };
}

TaskResult OpenCVCamera::stopFrameCapture()
{
    if (!acquiring_)
    {
        return {
            true,
            { "Camera is not acquiring", Log::Level::LVL_ERROR }
        };
    }

    acquiring_ = false;

    return true;
}

TaskResultP<std::shared_ptr<const CvImage>> OpenCVCamera::acquireFrame(int timeout)
{
    if (!cap_.isOpened())
    {
        return {
            std::nullopt,
            { "Camera handle is not opened", Log::Level::LVL_ERROR }
        };
    }

    // Return if acquisition is not running
    if (!acquiring_)
    {
        return {
            std::nullopt,
            { "Acquisition is not started", Log::Level::LVL_ERROR }
        };
    }

    cv::Mat mat;
    bool gotFrame = false;
    auto startTime = std::chrono::steady_clock::now();
    bool useTimeout = (timeout != -1);

    while (!gotFrame)
    {
        gotFrame = cap_.read(mat);

        if (gotFrame)
            break;

        // Check timeout
        if (useTimeout)
        {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();
            if (elapsed >= timeout)
            {
                return { std::nullopt, { "Timeout while waiting for frame", Log::Level::LVL_ERROR } };
            }
        }

        // Small sleep to avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (mat.empty())
    {
        return { std::nullopt, { "Captured empty frame", Log::Level::LVL_ERROR } };
    }

    return std::make_shared<const CvImage>(std::move(mat));
}

TaskResult OpenCVCamera::setStringParameter(
    const std::string& key,
    const std::string& value)
{
    return {
        false,
        { "String parameter not supported by OpenCV", Log::Level::LVL_ERROR }
    };
}

TaskResult OpenCVCamera::setIntParameter(
    const std::string& key,
    int value)
{
    if (!cap_.set(ALLOWED_PROPERTIES_STR.at(key), static_cast<double>(value)))
    {
        return {
            false,
            {"Failed to set int parameter: " + key, Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult OpenCVCamera::setDoubleParameter(
    const std::string& key,
    double value)
{
    if (!cap_.set(ALLOWED_PROPERTIES_STR.at(key), value))
    {
        return {
            false,
            {"Failed to set double parameter: " + key, Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult OpenCVCamera::setBoolParameter(
    const std::string& key,
    bool value)
{
    if (!cap_.set(ALLOWED_PROPERTIES_STR.at(key), static_cast<double>(value)))
    {
        return {
            false,
            {"Failed to set int parameter: " + key, Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult OpenCVCamera::executeCommand(const std::string& key)
{
    return {
        false,
        { "Command execution not supported by OpenCV", Log::Level::LVL_ERROR }
    };

}

///////////////////////////////////////////////////////////////////////////////

void OpenCVCamera::listAllFeatures(
    std::vector<std::shared_ptr<Parameter>>& features,
    std::vector<Log>& logs)
{
    try
    {
        // Width
        int width = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_WIDTH));

        features.push_back(
            ParameterInt::create(
                ALLOWED_PROPERTIES.at(cv::CAP_PROP_FRAME_WIDTH),
                "Capture",
                AccessMode::READWRITE,
                VisibilityLevel::BASIC,
                width,
                0,
                std::nullopt,
                1,
                "Frame Width",
                std::nullopt,
                std::nullopt)
        );

        // Height
        int height = static_cast<int>(cap_.get(cv::CAP_PROP_FRAME_HEIGHT));

        features.push_back(
            ParameterInt::create(
                ALLOWED_PROPERTIES.at(cv::CAP_PROP_FRAME_HEIGHT),
                "Capture",
                AccessMode::READWRITE,
                VisibilityLevel::BASIC,
                height,
                0,
                std::nullopt,
                1,
                "Frame Height",
                std::nullopt,
                std::nullopt)
        );

        // FPS
        double fps = cap_.get(cv::CAP_PROP_FPS);
        if (fps > 0)
        {
            features.push_back(
                ParameterDouble::create(
                    ALLOWED_PROPERTIES.at(cv::CAP_PROP_FPS),
                    "Capture",
                    AccessMode::READWRITE,
                    VisibilityLevel::BASIC,
                    fps,
                    0.0,
                    std::nullopt,
                    1.0,
                    "Frames per Second",
                    std::nullopt,
                    std::nullopt)
            );
        }

        // Exposure
        double exposure = cap_.get(cv::CAP_PROP_EXPOSURE);
        if (exposure >= 0)
        {
            features.push_back(
                ParameterDouble::create(
                    ALLOWED_PROPERTIES.at(cv::CAP_PROP_EXPOSURE),
                    "Capture",
                    AccessMode::READWRITE,
                    VisibilityLevel::BASIC,
                    exposure,
                    0.0,
                    std::nullopt,
                    0.1,
                    "Exposure",
                    std::nullopt,
                    std::nullopt)
            );
        }

        // Gain
        double gain = cap_.get(cv::CAP_PROP_GAIN);
        if (gain >= 0)
        {
            features.push_back(
                ParameterDouble::create(
                    ALLOWED_PROPERTIES.at(cv::CAP_PROP_GAIN),
                    "Capture",
                    AccessMode::READWRITE,
                    VisibilityLevel::BASIC,
                    exposure,
                    0.0,
                    std::nullopt,
                    0.1,
                    "Gain",
                    std::nullopt,
                    std::nullopt)
            );
        }
    }
    catch (const std::exception& e)
    {
        logs.push_back(
            { e.what(), Log::Level::LVL_ERROR }
        );
    }
};