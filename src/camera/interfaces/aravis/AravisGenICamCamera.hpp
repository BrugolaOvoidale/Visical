#ifdef WITH_GENICAM
#pragma once
#include <arv.h>
#include "../../ICamera.hpp"


/**
 * @brief Concrete implementation of ICamera for GenICam-compliant cameras using the Aravis library.
 */
class AravisGenICamCamera : public ICamera
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~AravisGenICamCamera() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    static std::shared_ptr<AravisGenICamCamera> create(
        const std::string& serialNmb,
        const std::string& interfaceId
    );

private:
    AravisGenICamCamera() = default;

    AravisGenICamCamera(
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

    TaskResult flushBuffers();

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

    // Maps an Aravis Node type to the internal DataType enum.
    DataType getFeatureType(ArvGcNode* node);

    // Maps GenICam access rights (RW, RO, WO) to internal AccessMode.
    AccessMode getAccessMode(ArvGcFeatureNode* node);

    // Maps GenICam visibility (Beginner, Expert, Guru) to internal VisibilityLevel.
    VisibilityLevel getVisibility(ArvGcFeatureNode* node);

    /**
     * @brief Recursively traverses the GenICam category tree to find features.
     * @param[out] features List to populate with found parameters.
     * @param genicam Pointer to the Aravis GenICam object.
     * @param featureName Name of the current node.
     * @param parentCategory String path of the parent category for logging/grouping.
     * @param[out] logs Collection of logs for recording discovery issues.
     */
    void listAllFeatures(
        std::vector<std::shared_ptr<Parameter>>& features,
        ArvGc* genicam,
        const char* featureName,
        const char* parentCategory,
        std::vector<Log>& logs
    );

private:
    // High-level Aravis camera control.
    ArvCamera* arv_cameraHandle_{ nullptr };

    // Low-level device communication handle.
    ArvDevice* arv_deviceHandle_{ nullptr };

    // Handles buffer queues and image data.
    ArvStream* arv_streamHandle_{ nullptr };

    // Internal flag tracking if the stream is active.
    bool acquiring_{ false };
};

#endif