#ifdef WITH_GENICAM
#include "AravisGenICamCamera.hpp"
#include <cv/CvImage.hpp>
#include <parameter/numeric/ParameterNumeric.hpp>
#include <parameter/enum/ParameterEnum.hpp>
#include <parameter/bool/ParameterBool.hpp>
#include <parameter/string/ParameterString.hpp>
#include <parameter/command/ParameterCommand.hpp>


AravisGenICamCamera::AravisGenICamCamera(
    const std::string& serialNmb,
    const std::string& interfaceId)
    : ICamera(serialNmb, interfaceId, CameraInterface::ARAVIS_GENICAM)
{
}

///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<AravisGenICamCamera> AravisGenICamCamera::create(
    const std::string& serialNmb,
    const std::string& interfaceId)
{
    struct Enabler : AravisGenICamCamera
    {
        Enabler(const std::string& serialNmb,
            const std::string& interfaceId)
            : AravisGenICamCamera(serialNmb, interfaceId)
        {
        }
    };

    return std::make_shared<Enabler>(serialNmb, interfaceId);
}

///////////////////////////////////////////////////////////////////////////////

TaskResult AravisGenICamCamera::checkCamera()
{
    bool success = false;
	std::vector<Log> logs;

    arv_update_device_list();
    int num_devices = arv_get_n_devices();

    for (int i = 0; i < num_devices; i++)
    {
        const char* serial = arv_get_device_serial_nbr(i);
        if (serial && serialNumber() == serial)
        {
            success = true;
            break;
        }
    }

    if (!success)
    {
        logs.push_back(
            { "Could not find device with Id: " + serialNumber(), Log::Level::LVL_ERROR }
        );
    }

    return { success, logs };
}

TaskResult AravisGenICamCamera::checkConnectionHealth()
{
    if (!arv_cameraHandle_)
    {
        return false;
    }

    GError* error = nullptr;
    const char* srlNmb = arv_camera_get_device_serial_number(arv_cameraHandle_, &error);
    if (!srlNmb || error)
    {
        std::string errorMsg = (error ? error->message : "unknown error");
        if (error) g_error_free(error);

        return { 
            false,
            { "Error checking connection health -> " + errorMsg, Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult AravisGenICamCamera::connectCamera()
{
    std::vector<Log> exitLogs;

    if (arv_cameraHandle_)
    {
        // Defensive: release old camera
        g_object_unref(arv_cameraHandle_);
        arv_cameraHandle_ = nullptr;
    }

    // Open camera by Id
    GError* error = nullptr;
    arv_cameraHandle_ = arv_camera_new(interfaceId().c_str(), &error);
    if (!arv_cameraHandle_ || error)
    {
        if (arv_cameraHandle_)
        {
            g_object_unref(arv_cameraHandle_);
            arv_cameraHandle_ = nullptr;
        }

		std::string errorMsg = (error ? error->message : "unknown error");
        if (error) g_error_free(error);

        return {
            false,
            { "Error opening camera -> " +  errorMsg, Log::Level::LVL_ERROR }
        };
    }

    // Get the device object
    arv_deviceHandle_ = arv_camera_get_device(arv_cameraHandle_);
    if (!arv_deviceHandle_)
    {
        g_object_unref(arv_cameraHandle_);

        return {
            false,
            { "Failed to get device", Log::Level::LVL_ERROR }
        };
    }

    // Configure GigE-specific settings before creating stream
    if (ARV_IS_GV_DEVICE(arv_deviceHandle_))
    {
        //arv_gv_device_auto_packet_size(ARV_GV_DEVICE(arv_deviceHandle_), &error);
        //if (error)
        //{
        //    std::string errorMsg = (error ? error->message : "unknown error");

        //    exitLogs.push_back(
        //        UtilityFunctions::LogFormat(Log::Level::LVL_WARNING, "AravisGenICamCamera::connectCamera", std::string("Error while auto-negotiating packet size for ") + serialNumber() + " -> " + errorMsg)
        //    );

        //    // Not fatal, just log it
        //    g_error_free(error);
        //    error = nullptr;
        //}

        //// Force a packet size that fits inside standard 1500-byte Ethernet frames:
        ////   1500 (MTU) - 20 (IP header) - 8 (UDP header) - 8 (GVSP header) = ~1464
        //GError* error = nullptr;
        //arv_device_set_integer_feature_value(
        //    arv_deviceHandle_, "GevSCPSPacketSize", 1464, &error);

        //if (error)
        //{
        //    exitLogs.push_back(UtilityFunctions::LogFormat(
        //        Log::Level::LVL_WARNING, "AravisGenICamCamera::connectCamera",
        //        std::string("Could not set GevSCPSPacketSize: ") + error->message));
        //    g_error_free(error);
        //    error = nullptr;
        //}
    }

    // Create stream
    arv_streamHandle_ = arv_camera_create_stream(arv_cameraHandle_, nullptr, nullptr, &error);
    if (!arv_streamHandle_ || error)
    {
        if (arv_streamHandle_)
        {
            g_object_unref(arv_streamHandle_);
            arv_streamHandle_ = nullptr;
        }

        g_object_unref(arv_cameraHandle_);
		arv_cameraHandle_ = nullptr;

        std::string errorMsg = (error ? error->message : "unknown error");
        if (error) g_error_free(error);

        return {
            false,
            { std::string("Error opening stream ") + serialNumber() + " -> " + errorMsg, Log::Level::LVL_ERROR }
        };
    }

    // Configure stream parameters
    if (ARV_IS_GV_STREAM(arv_streamHandle_))
    {
        g_object_set(arv_streamHandle_,
            "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
            "socket-buffer-size", 0,                                // auto
            "packet-resend", ARV_GV_STREAM_PACKET_RESEND_ALWAYS,
            "packet-timeout", 100000,  // 100ms in microseconds
            "frame-retention", 200000, // 200ms
            //"packet-timeout", 500000,                               // 500ms in microseconds
            //"frame-retention", 1000000,                             // 1s
            nullptr);
    }

    // Pre-allocate buffers
    guint payload = arv_camera_get_payload(arv_cameraHandle_, &error);
    // Case 1: Error occurred during get_payload
    if (error)
    {
        if (arv_streamHandle_)
        {
            g_object_unref(arv_streamHandle_);
            arv_streamHandle_ = nullptr;
        }

        if (arv_cameraHandle_)
        {
            g_object_unref(arv_cameraHandle_);
            arv_cameraHandle_ = nullptr;
        }

        std::string errorMsg = error->message;
        g_error_free(error);

        return {
            false,
            { std::string("Failed to get payload size: ") + errorMsg, Log::Level::LVL_ERROR }
        };
    }

    // Case 2: Invalid payload size (<= 0) but no error
    if (payload <= 0)
    {
        if (arv_streamHandle_)
        {
            g_object_unref(arv_streamHandle_);
            arv_streamHandle_ = nullptr;
        }

        if (arv_cameraHandle_)
        {
            g_object_unref(arv_cameraHandle_);
            arv_cameraHandle_ = nullptr;
        }

        return {
            false,
            { std::string("Invalid payload size (") + std::to_string(payload) +
              "). Check if camera is configured correctly",
              Log::Level::LVL_ERROR }
        };
    }

    for (size_t i = 0; i < MAX_BUFFER_SIZE; ++i)
    {
        ArvBuffer* buffer = arv_buffer_new(payload, nullptr);
        arv_stream_push_buffer(arv_streamHandle_, buffer);
    }

    return { true, exitLogs };
}

TaskResult AravisGenICamCamera::loadCameraParams(std::vector<std::shared_ptr<Parameter>>& features)
{
    // Get the GenICam interface
    ArvGc* genicam = arv_device_get_genicam(arv_deviceHandle_);
    if (!genicam)
    {
        return {
            false,
            { "Error while getting genicam device", Log::Level::LVL_ERROR }
        };
    }

    // Get the root node of the GenICam tree
    ArvGcNode* root = arv_gc_get_node(genicam, "Root");
    if (!root)
    {
        return {
            false,
            { "Error while getting 'Root'", Log::Level::LVL_ERROR }
        };
    }


    std::vector<std::shared_ptr<Parameter>> tmp_features;
    std::vector<Log> logs;
        
    listAllFeatures(tmp_features, genicam, "Root", "Root", logs);

    features = std::move(tmp_features);

    bool success = !features.empty();

    return { success, logs };
}

TaskResult AravisGenICamCamera::disconnectCamera()
{
    if (arv_streamHandle_)
    {
        g_object_unref(arv_streamHandle_);
        arv_streamHandle_ = nullptr;
    }

    if (arv_deviceHandle_)
    {
        g_object_unref(arv_deviceHandle_);
        arv_deviceHandle_ = nullptr;
        arv_cameraHandle_ = nullptr;
    }

    if (arv_cameraHandle_)
    {
        g_object_unref(arv_cameraHandle_);
        arv_cameraHandle_ = nullptr;
    }

    return true;
}

TaskResultP<AcquisitionMode> AravisGenICamCamera::acquisitionMode() const
{
    if (!arv_cameraHandle_)
    {
        return {
            std::nullopt,
            { "Camera handle is nullptr", Log::Level::LVL_ERROR }
        };
    }

    
    GError* error = nullptr;
    ArvAcquisitionMode acqMode = arv_camera_get_acquisition_mode(arv_cameraHandle_, &error);

    if (error)
    {
        std::string errorMsg = (error ? error->message : "unknown error");
        g_error_free(error);

        return {
            std::nullopt,
            { "Error getting acquisition mode -> " + errorMsg, Log::Level::LVL_ERROR }
        };
    }

    switch (acqMode)
    {
    case ARV_ACQUISITION_MODE_SINGLE_FRAME:
        return AcquisitionMode::SINGLE_FRAME;

    case ARV_ACQUISITION_MODE_MULTI_FRAME:
        return AcquisitionMode::MULTI_FRAME;

    case ARV_ACQUISITION_MODE_CONTINUOUS:
        return AcquisitionMode::CONTINUOUS;

    default:
        return {
            std::nullopt,
            { "Unknown acquisition mode", Log::Level::LVL_ERROR }
        };
    }
}

TaskResult AravisGenICamCamera::setAcquisitionMode(AcquisitionMode acqMode)
{
    if (!arv_cameraHandle_)
    {
        return {
            false,
            { "Camera handle is nullptr", Log::Level::LVL_ERROR }
        };
    }

    GError* error = nullptr;

    switch (acqMode)
    {
        case AcquisitionMode::SINGLE_FRAME:
            arv_camera_set_acquisition_mode(arv_cameraHandle_, ARV_ACQUISITION_MODE_SINGLE_FRAME, &error);
            if (error)
            {
                std::string errorMsg = (error ? error->message : "unknown error");
                g_error_free(error);

                return {
                    false,
                    { "Error setting acquisition mode to 'ARV_ACQUISITION_MODE_SINGLE_FRAME' -> " + errorMsg, Log::Level::LVL_ERROR }
                };

            }

            break;

        case AcquisitionMode::MULTI_FRAME:
            arv_camera_set_acquisition_mode(arv_cameraHandle_, ARV_ACQUISITION_MODE_MULTI_FRAME, &error);
            if (error)
            {
                std::string errorMsg = (error ? error->message : "unknown error");
                g_error_free(error);

                return {
                    false,
                    { "Error setting acquisition mode to 'ARV_ACQUISITION_MODE_MULTI_FRAME' -> " + errorMsg, Log::Level::LVL_ERROR }
                };

            }

            break;

        case AcquisitionMode::CONTINUOUS:
            arv_camera_set_acquisition_mode(arv_cameraHandle_, ARV_ACQUISITION_MODE_CONTINUOUS, &error);
            if (error)
            {
                std::string errorMsg = (error ? error->message : "unknown error");
                g_error_free(error);

                return {
                    false,
                    { "Error setting acquisition mode to 'ARV_ACQUISITION_MODE_CONTINUOUS' -> " + errorMsg, Log::Level::LVL_ERROR }
                };

            }

            break;

        default:
            return {
                false,
                { "Unknown acquisition mode", Log::Level::LVL_ERROR }
            };
    }

    return true;
}

TaskResult AravisGenICamCamera::startFrameCapture()
{
    GError* error = nullptr;
    arv_camera_start_acquisition(arv_cameraHandle_, &error);
    if (error)
    {
        std::string errorMsg = (error ? error->message : "unknown error");
        if (error) g_error_free(error);

        return {
            false,
            { "Error starting camera acquisition -> " + errorMsg, Log::Level::LVL_ERROR }
        };
    }

    std::vector<Log> exitLogs;

    // === DIAGNOSTICS START ===
    gint n_input_buffers, n_output_buffers;
    arv_stream_get_n_buffers(arv_streamHandle_, &n_input_buffers, &n_output_buffers);

    guint64 n_completed_buffers, n_failures, n_underruns;
    arv_stream_get_statistics(arv_streamHandle_, &n_completed_buffers, &n_failures, &n_underruns);

    std::string diagMsg = "Stream stats after start: "
        "input=" + std::to_string(n_input_buffers) +
        ", output=" + std::to_string(n_output_buffers) +
        ", completed=" + std::to_string(n_completed_buffers) +
        ", failures=" + std::to_string(n_failures) +
        ", underruns=" + std::to_string(n_underruns);

    exitLogs.push_back(
        { diagMsg, Log::Level::LVL_DEBUG }
    );

    const char* triggerMode = arv_camera_get_string(arv_cameraHandle_, "TriggerMode", nullptr);  // or use device_get_string_feature_value
    const char* triggerSelector = arv_camera_get_string(arv_cameraHandle_, "TriggerSelector", nullptr);

    std::string trigMsg = "Trigger state: Selector=" + std::string(triggerSelector ? triggerSelector : "N/A") +
        ", Mode=" + std::string(triggerMode ? triggerMode : "N/A");

    exitLogs.push_back(
        { trigMsg, Log::Level::LVL_DEBUG }
    );

    guint64 scpsPacketSize = arv_device_get_integer_feature_value(arv_deviceHandle_, "GevSCPSPacketSize", nullptr);


    exitLogs.push_back(
        { "GevSCPSPacketSize = " + std::to_string(scpsPacketSize), Log::Level::LVL_DEBUG }
    );
    // === DIAGNOSTICS END ===

    acquiring_ = true;

    return { true, std::move(exitLogs)};
}

TaskResult AravisGenICamCamera::stopFrameCapture()
{
    GError* error = nullptr;
    arv_camera_stop_acquisition(arv_cameraHandle_, &error);
    if (error)
    {
        std::string errorMsg = (error ? error->message : "unknown error");
        if (error) g_error_free(error);

        return {
            false,
            { "Error stopping frame capture -> " + errorMsg, Log::Level::LVL_ERROR }
        };
    }

    acquiring_ = false;

    TaskResult flushRes = flushBuffers();

    return { true, flushRes.takeLogs() };
}

TaskResult AravisGenICamCamera::flushBuffers()
{
    // Flush all pending buffers from the stream
    ArvBuffer* buffer;
    while ((buffer = arv_stream_try_pop_buffer(arv_streamHandle_)) != nullptr)
    {
        // Recycle all pending buffers (complete or incomplete)
        arv_stream_push_buffer(arv_streamHandle_, buffer);
    }

    // Optional: Log how many buffers were flushed
    gint n_input_buffers, n_output_buffers;
    arv_stream_get_n_buffers(arv_streamHandle_, nullptr, &n_output_buffers);
    if (n_output_buffers > 0)
    {
        // This shouldn't happen after flushing, but log it if it does
        return {
            true,
            { "Warning: " + std::to_string(n_output_buffers) + " buffers still in output queue after flush", Log::Level::LVL_WARNING }
        };
    }

    return true;
}

TaskResultP<std::shared_ptr<const CvImage>> AravisGenICamCamera::acquireFrame(int timeout)
{
    if (!arv_cameraHandle_)
    {
        return {
            std::nullopt,
            { "Camera handle is nullptr", Log::Level::LVL_ERROR }
        };
    }

    // Return if acquisition is not running
	if (!acquiring_)
    {
        return {
            std::nullopt,
            { "Stream is not started", Log::Level::LVL_ERROR }
        };
    }

    // Pop buffer (timeout ...)

    // Setup timing for true timeout logic
    bool useTimeout = (timeout != -1);

    ArvBuffer* buffer = nullptr;
    ArvBufferStatus bufferStatus = ARV_BUFFER_STATUS_UNKNOWN;

    auto startTime = std::chrono::steady_clock::now();
    // Retry loop until we get a successful buffer or timeout
    while (true)
    {
        // Calculate remaining timeout
        guint64 remainingTimeout = 0;
        if (useTimeout)
        {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();

            if (elapsed >= timeout)
            {
                return {
                    std::nullopt,
                    { "Timeout (" + std::to_string(timeout) + ") while waiting for successful frame", Log::Level::LVL_ERROR}
                };
            }

            remainingTimeout = (timeout - elapsed) * 1000; // Convert to microseconds
        }

        // Pop buffer with remaining timeout
        if (useTimeout)
        {
            buffer = arv_stream_timeout_pop_buffer(arv_streamHandle_, remainingTimeout);
        }
        else
        {
            buffer = arv_stream_pop_buffer(arv_streamHandle_);
        }

        if (!buffer)
        {
            return {
                std::nullopt,
                { "Timeout (" + std::to_string(timeout) + ") while waiting for successful frame", Log::Level::LVL_ERROR}
            };
        }

        // Check buffer status
        bufferStatus = arv_buffer_get_status(buffer);

        //std::string str_bufferStatus;
        //if (bufferStatus == ARV_BUFFER_STATUS_UNKNOWN) str_bufferStatus = "ARV_BUFFER_STATUS_UNKNOWN";
        //else if (bufferStatus == ARV_BUFFER_STATUS_SUCCESS) str_bufferStatus = "ARV_BUFFER_STATUS_SUCCESS";
        //else if (bufferStatus == ARV_BUFFER_STATUS_CLEARED) str_bufferStatus = "ARV_BUFFER_STATUS_CLEARED";
        //else if (bufferStatus == ARV_BUFFER_STATUS_TIMEOUT) str_bufferStatus = "ARV_BUFFER_STATUS_TIMEOUT";
        //else if (bufferStatus == ARV_BUFFER_STATUS_MISSING_PACKETS) str_bufferStatus = "ARV_BUFFER_STATUS_MISSING_PACKETS";
        //else if (bufferStatus == ARV_BUFFER_STATUS_WRONG_PACKET_ID) str_bufferStatus = "ARV_BUFFER_STATUS_WRONG_PACKET_ID";
        //else if (bufferStatus == ARV_BUFFER_STATUS_SIZE_MISMATCH) str_bufferStatus = "ARV_BUFFER_STATUS_SIZE_MISMATCH";
        //else if (bufferStatus == ARV_BUFFER_STATUS_FILLING) str_bufferStatus = "ARV_BUFFER_STATUS_FILLING";
        //else if (bufferStatus == ARV_BUFFER_STATUS_ABORTED) str_bufferStatus = "ARV_BUFFER_STATUS_ABORTED";
        //else if (bufferStatus == ARV_BUFFER_STATUS_PAYLOAD_NOT_SUPPORTED) str_bufferStatus = "ARV_BUFFER_STATUS_PAYLOAD_NOT_SUPPORTED";
        //else str_bufferStatus = "UNKNOWN_BUFFER_STATUS";

        //result.logs.emplace_back(
        //    UtilityFunctions::LogFormat(Log::Level::LVL_DEBUG,
        //        "AravisGenICamCamera::acquireFrame",
        //        "Buffer status: " + str_bufferStatus)
        //);

        // If successful, break out of retry loop
        if (bufferStatus == ARV_BUFFER_STATUS_SUCCESS)
        {
            break;
        }

        // Buffer was not successful - recycle and retry
        arv_stream_push_buffer(arv_streamHandle_, buffer);
        buffer = nullptr;

        // For infinite timeout (-1), continue retrying indefinitely
        // For finite timeout, the while loop will check elapsed time on next iteration
    }

    // At this point, we have a successful buffer
    // Extract image data
    GError* error = nullptr;
    std::optional<std::shared_ptr<const CvImage>> resultImage;
	std::vector<Log> exitLogs;

    size_t size;
    void* data = (void*)arv_buffer_get_data(buffer, &size);

    if (data && size > 0)
    {
        // Get image info
        uint32_t width = arv_buffer_get_image_width(buffer);
        uint32_t height = arv_buffer_get_image_height(buffer);
        ArvPixelFormat pixelFormat = arv_camera_get_pixel_format(arv_cameraHandle_, &error);
        if (error)
        {
			std::string errorMsg = (error ? error->message : "unknown error");

            if (error) g_error_free(error);

            // Recycle buffer
            arv_stream_push_buffer(arv_streamHandle_, buffer); 
                
            return {
                std::nullopt,
                { "Error getting frame pixel format -> " + errorMsg, Log::Level::LVL_ERROR}
            };
        }

        // Convert to cv::Mat
		bool conversionSuccess = true;
        cv::Mat mat;
        switch (pixelFormat)
        {
            case ARV_PIXEL_FORMAT_MONO_8:
                mat = cv::Mat(height, width, CV_8UC1, data).clone();

                break;

            case ARV_PIXEL_FORMAT_MONO_16:
                mat = cv::Mat(height, width, CV_16UC1, data).clone();

                break;

            case ARV_PIXEL_FORMAT_RGB_8_PACKED:
                mat = cv::Mat(height, width, CV_8UC3, data).clone();
                cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);

                break;

            case ARV_PIXEL_FORMAT_BAYER_RG_8:
                mat = cv::Mat(height, width, CV_8UC1, data).clone();
                cv::cvtColor(mat, mat, cv::COLOR_BayerRG2BGR);

                break;

            case ARV_PIXEL_FORMAT_BAYER_RG_16:
                mat = cv::Mat(height, width, CV_16UC1, data).clone();
                cv::cvtColor(mat, mat, cv::COLOR_BayerRG2BGR);

                break;

            default:
                exitLogs.push_back(
                    { "Unsupported Aravis pixel format", Log::Level::LVL_ERROR }
                );

				conversionSuccess = false;
        }

        if (conversionSuccess)
        {
            resultImage = std::make_shared<const CvImage>(std::move(mat));
		}
    }
    else
    {
        exitLogs.push_back(
            { "No data in buffer", Log::Level::LVL_ERROR }
        );
    }

    // Recycle buffer
    arv_stream_push_buffer(arv_streamHandle_, buffer);

    return { std::move(resultImage), std::move(exitLogs)};
}

TaskResult AravisGenICamCamera::setStringParameter(
    const std::string& key,
    const std::string& value
)
{
    GError* error = nullptr;

    try
    {
        // Set string feature
        arv_camera_set_string(arv_cameraHandle_, key.c_str(), value.c_str(), &error);
        if (error)
        {
            std::string errorMsg = (error ? error->message : "unknown error");
            if (error) g_error_free(error);

            return {
                false,
                { "Failed to set string parameter -> " + errorMsg, Log::Level::LVL_ERROR }
            };
        }
    }
    catch (const std::exception& e)
    {
        return {
            false,
            { e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult AravisGenICamCamera::setIntParameter(
    const std::string& key,
    int value
)
{
    GError* error = nullptr;

    try
    {
        // Set int feature
        arv_camera_set_integer(arv_cameraHandle_, key.c_str(), value, &error);
        if (error)
        {
            std::string errorMsg = (error ? error->message : "unknown error");
            if (error) g_error_free(error);

            return {
                false,
                { "Failed to set int parameter -> " + errorMsg, Log::Level::LVL_ERROR }
            };
        }
    }
    catch (const std::exception& e)
    {
        return {
            false,
            { e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult AravisGenICamCamera::setDoubleParameter(
    const std::string& key,
    double value
)
{
    GError* error = nullptr;

    try
    {
        // Set float feature
        arv_camera_set_float(arv_cameraHandle_, key.c_str(), value, &error);
        if (error)
        {
            std::string errorMsg = (error ? error->message : "unknown error");
            if (error) g_error_free(error);

            return {
                false,
                { "Failed to set float parameter -> " + errorMsg, Log::Level::LVL_ERROR }
            };
        }
    }
    catch (const std::exception& e)
    {
        return {
            false,
            { e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult AravisGenICamCamera::setBoolParameter(
    const std::string& key,
    bool value
)
{
    GError* error = nullptr;

    try
    {
        // Set bool feature
        arv_camera_set_boolean(arv_cameraHandle_, key.c_str(), value, &error);
        if (error)
        {
            std::string errorMsg = (error ? error->message : "unknown error");
            if (error) g_error_free(error);

            return {
                false,
                { "Failed to set boolean parameter -> " + errorMsg, Log::Level::LVL_ERROR }
            };
        }
    }
    catch (const std::exception& e)
    {
        return {
            false,
            { e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

TaskResult AravisGenICamCamera::executeCommand(
    const std::string& key
)
{
    GError* error = nullptr;

    try
    {
        // Execute command
        arv_camera_execute_command(arv_cameraHandle_, key.c_str(), &error);
        if (error)
        {
            std::string errorMsg = (error ? error->message : "unknown error");
            if (error) g_error_free(error);

            return {
                false,
                { "Failed to execute command -> " + errorMsg, Log::Level::LVL_ERROR }
            };
        }
    }
    catch (const std::exception& e)
    {
        return {
            false,
            { e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

DataType AravisGenICamCamera::getFeatureType(ArvGcNode* node)
{
    if (ARV_IS_GC_ENUMERATION(node)) return DataType::ENUMERATION;    //  ENUM first -> is a specialization of INTEGER
    if (ARV_IS_GC_INTEGER(node)) return DataType::INTEGER;
    if (ARV_IS_GC_FLOAT(node)) return DataType::DOUBLE;
    if (ARV_IS_GC_BOOLEAN(node)) return DataType::BOOLEAN;
    if (ARV_IS_GC_STRING(node)) return DataType::STRING;
    if (ARV_IS_GC_COMMAND(node)) return DataType::COMMAND;
    return DataType::UNDEFINED;
}

AccessMode AravisGenICamCamera::getAccessMode(ArvGcFeatureNode* node)
{
    switch (arv_gc_feature_node_get_actual_access_mode(node))
    {
        case ARV_GC_ACCESS_MODE_UNDEFINED: return AccessMode::UNDEFINED;
        case ARV_GC_ACCESS_MODE_WO: return AccessMode::WRITEONLY;
        case ARV_GC_ACCESS_MODE_RO: return AccessMode::READONLY;
        case ARV_GC_ACCESS_MODE_RW: return AccessMode::READWRITE;
        default: return AccessMode::UNDEFINED;
    }
}

VisibilityLevel AravisGenICamCamera::getVisibility(ArvGcFeatureNode* node)
{
    switch (arv_gc_feature_node_get_visibility(node))
    {
        case ARV_GC_VISIBILITY_BEGINNER: return VisibilityLevel::BASIC;
        case ARV_GC_VISIBILITY_EXPERT: return VisibilityLevel::INTERMEDIATE;
        case ARV_GC_VISIBILITY_GURU: return VisibilityLevel::ADVANCED;
        case ARV_GC_VISIBILITY_INVISIBLE: return VisibilityLevel::HIDDEN;
        default: return VisibilityLevel::UNDEFINED;
    }
}

void AravisGenICamCamera::listAllFeatures(
    std::vector<std::shared_ptr<Parameter>>& features,
    ArvGc* genicam,
    const char* featureName,
    const char* parentCategory,
    std::vector<Log>& logs)
{
    if (!genicam)
    {
        logs.push_back({ "genicam is nullptr", Log::Level::LVL_ERROR });

        return;
    }

    ArvGcNode* node = arv_gc_get_node(genicam, featureName);
    if (!node)
    {
        logs.push_back({ "Can't get node " + std::string(featureName), Log::Level::LVL_ERROR });

        return;
    }

    ArvGcFeatureNode* featureNode = ARV_GC_FEATURE_NODE(node);
    if (!featureNode)
    {
        logs.push_back({ "Can't get featureNode " + std::string(featureName), Log::Level::LVL_ERROR });

        return;
    }

    ArvGcAccessMode access = arv_gc_feature_node_get_actual_access_mode(featureNode);
    if (access == ARV_GC_ACCESS_MODE_UNDEFINED)
    {
        logs.push_back({ "Undefined access for '" + std::string(featureName) + "'. Skipping.", Log::Level::LVL_INFO});

        return;
    }

    ArvGcVisibility visibilty = arv_gc_feature_node_get_visibility(featureNode);
    if (visibilty == ARV_GC_VISIBILITY_UNDEFINED)
    {
        logs.push_back({ "Visibility is undefined '" + std::string(featureName) + "'. Skipping.", Log::Level::LVL_INFO });

        return;
    }
    else if (visibilty == ARV_GC_VISIBILITY_INVISIBLE)
    {
        logs.push_back({ "Visibility invisible for '" + std::string(featureName) + "'. Skipping.", Log::Level::LVL_INFO });

        return;
    }

    GError* error = nullptr;

    // skip unimplemented
    if (!arv_gc_feature_node_is_implemented(featureNode, &error))
    {
        if (error)
        {
            std::string errorMsg = (error ? error->message : "unknown error");
            g_error_free(error);

            logs.push_back({ "Error checking if feature is implemented for " + std::string(featureName) + " ->" + errorMsg, Log::Level::LVL_ERROR });
        }
        else
        {
            logs.push_back({ "Feature node '" + std::string(featureName) + "' is not implemented" + "'. Skipping.", Log::Level::LVL_INFO });
        }

        return;
    }

    // skip unavailable
    if (!arv_gc_feature_node_is_available(featureNode, &error))
    {
        if (error)
        {
            std::string errorMsg = (error ? error->message : "unknown error");
            g_error_free(error);

            logs.push_back({ "Error checking if feature is available for " + std::string(featureName) + " ->" + errorMsg, Log::Level::LVL_ERROR });
        }
        else
        {
            logs.push_back({ "Feature node '" + std::string(featureName) + "' is not available. Skipping.", Log::Level::LVL_INFO });
        }

        return;
    }

    // Explore category contents
    if (ARV_IS_GC_CATEGORY(node))
    {
        ArvGcCategory* category = ARV_GC_CATEGORY(node);
        const GSList* children = arv_gc_category_get_features(category);
        for (const GSList* iter = children; iter != nullptr; iter = iter->next)
        {
            listAllFeatures(
                features,
                genicam,
                (const char*)iter->data,
                featureName,
                logs
            );
        }

        return;
    }


    std::shared_ptr<Parameter> param;

    std::string name = featureName;

    std::string displayName = arv_gc_feature_node_get_display_name(featureNode) ? arv_gc_feature_node_get_display_name(featureNode) : featureName;

    DataType valueType = getFeatureType(node);

    AccessMode accessMode = getAccessMode(featureNode);

    VisibilityLevel visibiltyMode = getVisibility(featureNode);

    const char* arv_descr = arv_gc_feature_node_get_description(featureNode);
    std::optional<std::string> description;
    if (arv_descr)
    {
        description = arv_descr;
    }

    std::string category = parentCategory;

    switch (valueType)
    {
        case DataType::UNDEFINED:
            // It early returns in the method, so it cannot happen
            break;

        case DataType::INTEGER:
        {
            ArvGcInteger* integerNode = ARV_GC_INTEGER(node);

            int rawMin = static_cast<int>(arv_gc_integer_get_min(integerNode, nullptr));
            int rawMax = static_cast<int>(arv_gc_integer_get_max(integerNode, nullptr));
            int rawInc = static_cast<int>(arv_gc_integer_get_inc(integerNode, nullptr));
            int rawVal = static_cast<int>(arv_gc_integer_get_value(integerNode, nullptr));

            // Normalize Min/Max
            std::optional<int> min;
            std::optional<int> max;

            // Aravis invalid range sentinel = {0, -1}
            if (!(rawMin == 0 && rawMax == -1) && rawMin <= rawMax)
            {
                min = rawMin;
                max = rawMax;
            }

            // Normalize Increment
            std::optional<int> increment;

            // Aravis sentinel for "no increment"
            if (rawInc != std::numeric_limits<int>::min() && rawInc > 0)
            {
                increment = rawInc;
            }

            // Normalize value (always valid)
            int value = rawVal;

            // Safe unit
            const char* unitCStr = arv_gc_integer_get_unit(integerNode);
            std::optional<std::string> unit;
            if (unitCStr)
            {
                unit = unitCStr;
            }

            param = ParameterInt::create(
                std::move(name),
                std::move(category),
                std::move(accessMode),
                std::move(visibiltyMode),
                std::move(value),
                std::move(min),
                std::move(max),
                std::move(increment),
                std::move(displayName),
                std::move(unit),
                std::move(description)
                );

            break;
        }

        case DataType::DOUBLE:
        {
            ArvGcFloat* floatNode = ARV_GC_FLOAT(node);

            double rawMin = arv_gc_float_get_min(floatNode, nullptr);
            double rawMax = arv_gc_float_get_max(floatNode, nullptr);
            double rawInc = arv_gc_float_get_inc(floatNode, nullptr);
            double rawVal = arv_gc_float_get_value(floatNode, nullptr);
            
            // Normalize Min/Max
            std::optional<double> min;
            std::optional<double> max;

            // Aravis invalid range sentinel = {0, -1}
            if (!(rawMin == 0.0 && rawMax == -1.0) && rawMin <= rawMax)
            {
                min = rawMin;
                max = rawMax;
            }

            // Normalize Increment
            std::optional<double> increment;

            // Aravis sentinel for "no increment"
            if (rawInc != std::numeric_limits<double>::min() && rawInc > 0)
            {
                increment = rawInc;
            }

            // Normalize value (always valid)
            double value = rawVal;

            // Safe unit
            const char* unitCStr = arv_gc_float_get_unit(floatNode);
            std::optional<std::string> unit;
            if (unitCStr)
            {
                unit = unitCStr;
            }

            param = ParameterDouble::create(
                std::move(name),
                std::move(category),
                std::move(accessMode),
                std::move(visibiltyMode),
                std::move(value),
                std::move(min),
                std::move(max),
                std::move(increment),
                std::move(displayName),
                std::move(unit),
                std::move(description)
            );

            break;
        }

        case DataType::ENUMERATION:
        {
            ArvGcEnumeration* enumNode = ARV_GC_ENUMERATION(node);

            std::unordered_map<int, std::string> intValues;

            const GSList* entries = arv_gc_enumeration_get_entries(enumNode);
            for (const GSList* iter = entries; iter != nullptr; iter = iter->next)
            {
                ArvGcEnumEntry* entry = ARV_GC_ENUM_ENTRY(iter->data);
                const char* entryName = arv_gc_feature_node_get_name(ARV_GC_FEATURE_NODE(entry));
                int entryValue = static_cast<int>(arv_gc_enum_entry_get_value(entry, NULL));

                if (entryName)
                {
                    std::string str_entry_name(entryName);
                    intValues[entryValue] = std::string(entryName);
                }
            }

            int value = static_cast<int>(arv_gc_enumeration_get_int_value(enumNode, nullptr));

            param = ParameterEnum::create(
                std::move(name),
                std::move(category),
                std::move(accessMode),
                std::move(visibiltyMode),
                std::move(intValues),
                std::move(value),
                std::move(displayName),
                std::nullopt,
                std::move(description)
            );

            break;
        }

        case DataType::BOOLEAN:
        {
            bool value = static_cast<bool>(arv_gc_boolean_get_value(ARV_GC_BOOLEAN(node), nullptr));

            param = ParameterBool::create(
                std::move(name),
                std::move(category),
                std::move(accessMode),
                std::move(visibiltyMode),
                std::move(value),
                std::move(displayName),
                std::nullopt,
                std::move(description)
            );

            break;
        }

        case DataType::STRING:
        {
            std::string value = arv_gc_string_get_value(ARV_GC_STRING(node), nullptr);

            param = ParameterString::create(
                std::move(name),
                std::move(category),
                std::move(accessMode),
                std::move(visibiltyMode),
                std::move(value),
                std::move(displayName),
                std::nullopt,
                std::move(description)
            );

            break;
        }

        case DataType::COMMAND:
        {
            param = ParameterCommand::create(
                std::move(name),
                std::move(category),
                std::move(accessMode),
                std::move(visibiltyMode),
                std::move(displayName),
                std::nullopt,
                std::move(description)
            );

            break;
        }
    }

    if (param)
    {
        features.push_back(param);
    }
};
#endif