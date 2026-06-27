#pragma once
#include <async_worker/AsyncWorker.hpp>
#include <enum/EnumTraits.hpp>
#include <camera/ICameraDefs.hpp>
#include "CameraAssistantModelMessages.hpp"
#include "../BaseModel.hpp"


// Forward declarations
class CameraManager;
class ICameraProxy;
class ICameraInfo;
class ICamera;


/**
 * @brief High-level object for managing camera lifecycles and automated behaviors.
 * 
 * This class acts as a bridge between the ameraManager and the application logic.
 * Inheriting from AsyncWorker, it processes heavy operations in a background
 * thread to keep the main loop responsive.
 */
class CameraAssistantModel : public BaseModel, public AsyncWorker
{
public:
    //-----------------------------------------------------------------------------
    // Enums and structs
    //-----------------------------------------------------------------------------

    /** * @brief Defines the automation level for an individual camera. */
    enum class CameraBehaviour
    {
        NONE = -1,          ///< Manual control only.
        AUTO_CHECK,         ///< Verify camera presence automatically on registration.
        AUTO_CONNECT,       ///< Attempt connection as soon as the camera is discovered.
        AUTO_LIVE           ///< Attempt streaming upon successful connection.
    };

    /** * @brief Data Transfer Object for camera configuration. */
    struct CameraDTO
    {
        std::string id;                                         ///< Unique camera identifier.
        CameraInterface camInterface;                           ///< Hardware interface type.
        bool paramsPersistence{ false };                        ///< See ICamera parameters persistance.
        CameraBehaviour behaviour{ CameraBehaviour::NONE };     ///< Desired automation level.
    };

    /** * @brief Public summary of a managed camera's status and intent. */
    struct CameraDescriptorInfo {
        std::shared_ptr<ICameraInfo> camInfo;                   ///< Read-only interface provided by the camera driver.
        CameraBehaviour behaviour{ CameraBehaviour::NONE };     ///< Current assigned behavior.
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Initializes the model as a self-contained module. */
    CameraAssistantModel(const std::shared_ptr<MessageLogger>& logger = nullptr);

    /** @brief Initializes the model with an external CameraManager. */
    CameraAssistantModel(
        const std::shared_ptr<CameraManager>& cameraManager,
        const std::shared_ptr<MessageLogger>& logger = nullptr
    );

    ~CameraAssistantModel();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Gracefully stops and clear the internal resources.
     */
    void shutdown() override;

    /** 
     * @brief Asynchronously triggers a scan for all supported camera interfaces..
     * @returns NO_ERRORS.
     */
    TaskEnqueueResult autoDiscovery();

    /** 
     * @brief Asynchronously triggers a scan for a specific interface.
     * @returns NO_ERRORS.
     */
    TaskEnqueueResult discoverInterface(CameraInterface interfaceType);

    /** 
     * @brief Batch loads cameras from a configuration vector..
     * @returns REJECTED if vector is empty, NO_ERRORS otherwise.
     */
    TaskEnqueueResult loadCamerasConfig(const std::vector<CameraDTO>& cameraVec);

    /** 
    * @brief Verifies if a specific camera is still reachable.
    * @returns REJECTED if camera is not registered, see ICamera::check for more details.
    */
    TaskEnqueueResult checkCamera(const std::string& cameraId);

    /** 
     * @brief Initiates an asynchronous connection to the specified camera.
     * @returns REJECTED if camera is not registered, see ICamera::connect for more details.
     */
    TaskEnqueueResult connectCamera(const std::string& cameraId);

    /** 
     * @brief Disconnects the specified camera.
     * @returns REJECTED if camera is not registered, see ICamera::disconnect for more details.
     */
    TaskEnqueueResult disconnectCamera(const std::string& cameraId);

    /** 
     * @brief Retrieves the information interface for a specific camera.
     * @return Pointer to camera info, or nullptr if not registered.
     */
    std::shared_ptr<ICameraInfo> getCamera(const std::string& cameraId) const;

    /** @brief Returns a list of all currently managed cameras and their behaviors. */
    std::vector<CameraDescriptorInfo> getAllCameras() const;

    /**
     * @brief Set camera parameter.
     * @return false if camera is not registered, true otherwise.
     */
    TaskResult setCameraParameter(
        const std::string& cameraId,
        const std::string& paramId,
        const std::string& categoryId,
        const std::string& value
    );

    /**
     * @brief Set camera parameter.
     * @return false if camera is not registered, true otherwise.
     */
    TaskResult setCameraParameter(
        const std::string& cameraId,
        const std::string& paramId,
        const std::string& categoryId,
        int value
    );

    /**
     * @brief Set camera parameter.
     * @return false if camera is not registered, true otherwise.
     */
    TaskResult setCameraParameter(
        const std::string& cameraId,
        const std::string& paramId,
        const std::string& categoryId,
        double value
    );

    /**
     * @brief Set camera parameter.
     * @return false if camera is not registered, true otherwise.
     */
    TaskResult setCameraParameter(
        const std::string& cameraId,
        const std::string& paramId,
        const std::string& categoryId,
        bool value
    );

    /**
     * @brief Execute camera parameter.
     * @return false if camera is not registered, true otherwise.
     */
    TaskResult executeCameraCommand(
        const std::string& cameraId,
        const std::string& paramId,
        const std::string& categoryId
    );

    /**
     * @brief Reset camera parameter.
     * @return false if camera is not registered, true otherwise.
     */
    TaskResult resetCameraParameter(
        const std::string& cameraId,
        const std::string& paramId,
        const std::string& categoryId
    );

    /**
     * @brief See ICamera::disconnect for more details
     * @return false if camera is not registered, true otherwise.
     */
    TaskResult setCameraParamsPersistence(
        const std::string& cameraId,
        bool isPersistent
    );

    /** @brief Updates the automation behavior for a single camera. */
    bool setCameraBehaviour(
        const std::string& cameraId,
        CameraBehaviour beahviour
    );

    /** @brief Updates the default automation behavior for new cameras. */
    void setDefaultCameraBehaviour(CameraBehaviour beahviour);

    /** @brief Returns the current default behavior for new cameras. */
    CameraBehaviour getDefaultCameraBehaviour() const;

private:
    // Internal tracking structure for registered and pending cameras.
    struct CameraDescriptor {
        CameraDescriptor() = default;

        CameraDescriptor(const std::string& i, CameraBehaviour b) : id(i), behaviour(b) {}

        std::string id;
        CameraBehaviour behaviour{ CameraBehaviour::NONE };
    };

    // Catch Check camera message
    void doCameraCheck(const MessageTaskP<CameraState>& msg);
    void onCameraCheck(const MessageTaskP<CameraState>& msg);

    // Catch camera connection message
    void doCameraConnection(const MessageTask& msg);
    void onCameraConnection(const MessageTask& msg);

    // Catch camera disconnection message
    void doCameraDisconnection(const MessageTask& msg);
    void onCameraDisconnection(const MessageTask& msg);

	// Catch camera start live acquisition frame message
    void doCameraStartLive(const MessageTask& msg);
    void onCameraStartLive(const MessageTask& msg);

	// Catch camera stop live acquisition frame message
    void doCameraStopLive(const MessageTask& msg);
    void onCameraStopLive(const MessageTask& msg);

	// Catch camera snapshot acquisition frame message
    void doCameraTakeSnap(const MessageTask& msg);
    void onCameraTakeSnap(const MessageTask& msg);

    // Catch camera register message
    void doRegisterCamera(const Message& msg);
    void onRegisterCamera(const Message& msg);

	// Catch camera unregister message
    void doUnregisterCamera(const Message& msg);
    void onUnregisterCamera(const Message& msg);

    void init();

    TaskResult autoDiscoveryTask();

    TaskResult discoverInterfaceTask(CameraInterface interfaceType);

    TaskResult loadCamerasConfigTask(const std::vector<CameraDTO>& cameraVec);

    void addPendingRegistration(CameraDescriptor reg);

    void removePendingRegistration(const std::string& cameraId);

    void acceptPendingRegistration(const std::string& cameraId);

    bool hasPendingRegistration(const std::string& cameraId);

    void registerCamera(const CameraDescriptor& descr);

    std::optional<CameraDescriptor> getCameraDescriptor(const std::string& cameraId);

    void bindCameraMessages(const std::shared_ptr<ICameraProxy>& cam);

    void bindCameraMessages(const std::shared_ptr<ICamera>& cam);

private:
    std::shared_ptr<CameraManager> cameraManager_;

	std::atomic<CameraAssistantModel::CameraBehaviour> defaultCamBehaviour_{ CameraAssistantModel::CameraBehaviour::NONE };

    mutable std::shared_mutex registeredCamerasMutex_;
    std::unordered_map<std::string, CameraDescriptor> registeredCameras_;

    mutable std::shared_mutex pendingRegistrationsMutex_;
    std::unordered_map<std::string, CameraDescriptor> pendingRegistrations_;
};

template <>
struct EnumTraits<CameraAssistantModel::CameraBehaviour> {
    static constexpr std::array<std::pair<CameraAssistantModel::CameraBehaviour, const char*>, 4> values{ {
    {CameraAssistantModel::CameraBehaviour::NONE, "none"},
    {CameraAssistantModel::CameraBehaviour::AUTO_CHECK, "auto_check"},
    {CameraAssistantModel::CameraBehaviour::AUTO_CONNECT, "auto_connect"},
    {CameraAssistantModel::CameraBehaviour::AUTO_LIVE, "auto_live"}
    } };
};

static const std::vector<CameraAssistantModel::CameraBehaviour> CAM_BEHAVIOURS{
    CameraAssistantModel::CameraBehaviour::NONE,
    CameraAssistantModel::CameraBehaviour::AUTO_CHECK,
    CameraAssistantModel::CameraBehaviour::AUTO_CONNECT,
    CameraAssistantModel::CameraBehaviour::AUTO_LIVE
};