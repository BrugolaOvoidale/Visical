#pragma once
#include <opencv2/videoio.hpp>
#ifdef _WIN32
#include "windows/DirectShow.hpp"
#elif __linux__
#include "linux/V4L2.hpp"
#elif __APPLE__
#include "apple/AVFoundation.hpp"
#endif
#include "../../ICamera.hpp"


/**
 * @brief Concrete implementation of ICamera using the OpenCV VideoCapture module.
 * 
 * This class uses different OpenCV backends, based on the OS:
 * 
 * - Windows: Uses DirectShow (CAP_DSHOW).
 * 
 * - Linux: Uses Video4Linux2 (CAP_V4L2).
 * 
 * - Apple: Uses AVFoundation (CAP_AVFOUNDATION).
 */
class OpenCVCamera : public ICamera
{
public:
    //-----------------------------------------------------------------------------
    // Defs
    //-----------------------------------------------------------------------------

#ifdef _WIN32
    using OSInterface = DirectShow;
    static constexpr cv::VideoCaptureAPIs API{ cv::VideoCaptureAPIs::CAP_DSHOW };
#elif __linux__
    using OSInterface = V4L2;
    static constexpr cv::VideoCaptureAPIs API{ cv::VideoCaptureAPIs::CAP_V4L2 };
#elif __APPLE__
    using OSInterface = AVFoundation;
    static constexpr cv::VideoCaptureAPIs API{ cv::VideoCaptureAPIs::CAP_AVFOUNDATION };
#endif


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~OpenCVCamera() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    static std::shared_ptr<OpenCVCamera> create(
        const std::string& serialNmb,
        const std::string& interfaceId
    );

private:
    OpenCVCamera() = default;

    OpenCVCamera(
        const std::string& serialNmb,
        const std::string& interfaceId
    );

    TaskResult checkCamera() override;

    TaskResult checkConnectionHealth() override;

    TaskResult connectCamera() override;

    TaskResult loadCameraParams(std::vector<std::shared_ptr<Parameter>>& features) override;

    TaskResult disconnectCamera() override;

    TaskResultP<AcquisitionMode> acquisitionMode() const override;

    TaskResult setAcquisitionMode(AcquisitionMode acqMode) override;

    TaskResult startFrameCapture() override;

    TaskResult stopFrameCapture() override;

    TaskResultP<std::shared_ptr<const CvImage>> acquireFrame(int timeout) override;

    TaskResult setStringParameter(
        const std::string& key,
        const std::string& value
    ) override;

    TaskResult setIntParameter(
        const std::string& key,
        int value
    ) override;

    TaskResult setDoubleParameter(
        const std::string& key,
        double value
    ) override;

    TaskResult setBoolParameter(
        const std::string& key,
        bool value
    ) override;

    TaskResult executeCommand(
        const std::string& key
    ) override;

    /**
     * @brief Internal helper to enumerate all OpenCV VideoCapture properties.
     * @param[out] features Vector to be populated with discovered parameters.
     * @param[out] logs Vector to store diagnostic information during discovery.
     */
    void listAllFeatures(
        std::vector<std::shared_ptr<Parameter>>& features,
        std::vector<Log>& logs
    );

private:
    cv::VideoCapture cap_;

	int deviceIndex_{ -1 };

    bool acquiring_{ false };
};
