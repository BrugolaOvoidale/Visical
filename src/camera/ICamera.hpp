#pragma once
#include <parameter/ParameterOwner.hpp>
#include <async_worker/AsyncWorker.hpp>
#include "ICameraMessages.hpp"
#include "CameraState.hpp"
#include "CameraStateMachine.hpp"
#include "ICameraDefs.hpp"
#include "ICameraInfo.hpp"


// Forward declarations
class CvImage;


/**
 * @brief Abstract base class for asynchronous camera management.
 * 
 * ICamera provides a thread-safe, state-machine-driven interface for camera control.
 * It inherits from AsyncWorker to perform long-running IO operations (connect, grab, etc.)
 * without blocking the main thread.
 * Derived classes must implement the protected pure virtual "SDK-specific" methods.
 * See AsyncWorker, ParameterOwner
 */
class ICamera : public AsyncWorker, public ParameterOwner, public std::enable_shared_from_this<ICamera>
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    /** @brief Maximum number of frames stored in the internal circular buffer. */
    static constexpr size_t MAX_BUFFER_SIZE = 30;


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    virtual ~ICamera();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
	* @brief Enqueues a hardware detection check. It is a hard requirement to run most of the other operations that the camera is detected first.
    * @returns NO_ERRORS.
    */
    TaskEnqueueResult check();

    /**
     * @brief Enqueues an asynchronous connection request.
	 * @returns REJECTED if camera is not detected, ALREADY_DONE if already connected, ALREADY_ACTIVE if connection is in progress, NO_ERRORS otherwise.
     */
    TaskEnqueueResult connect();

    /**
     * @brief Enqueues an asynchronous connection request.
	 * @returns REJECTED if camera is grabbing, ALREADY_DONE if already disconnected, ALREADY_ACTIVE if disconnection is in progress, NO_ERRORS otherwise.
     */
    TaskEnqueueResult disconnect();

    /**
    * @brief Runs a synchronous task to safely shut down the camera and its resources.
    * 
    * Assume that camera is always shutted down.
    * 
    * @returns false if shutdown is already in progress, NO_ERRORS otherwise.
    */
    TaskResult shutdown();

    /**
     * @brief Enqueues an asynchronous start grab request.
     * @param timeout Acquisition timeout in milliseconds (-1 for infinite).
     * @returns REJECTED if camera is not connected, ALREADY_DONE if already grabbing, ALREADY_ACTIVE if grabbing request is in progress, NO_ERRORS otherwise.
     */
    TaskEnqueueResult startGrab(int timeout = -1);

    /**
     * @brief Signals the acquisition loop to stop at the next possible opportunity.
     *
     * Note that this is a "best effort stop", meaning it guarantees that
     * the software loop is stopped, but it does not guarantee that the
     * underlying hardware interface has stopped sending frames.
     * In the latter case, a warning log will be attached.
     * 
     * @returns REJECTED if camera is not connected, ALREADY_DONE if not grabbing, ALREADY_ACTIVE if stop grabbing request is in progress, NO_ERRORS otherwise.
     */
    TaskEnqueueResult stopGrab();

    /**
     * @brief Enqueues an asynchronous single frame capture request.
     * @param func Callback to execute once the frame is captured or fails.
     * @param timeout Capture timeout in milliseconds (-1 for infinite).
     * @returns REJECTED if camera is not connected, NO_ERRORS otherwise.
     */
    TaskEnqueueResult takeSnapshot(
        std::move_only_function<void(const TaskResultP<std::shared_ptr<const CvImage>>&)> func,
        int timeout = -1
    );

    /** @overload */
    template<typename Func, typename Class>
    TaskEnqueueResult takeSnapshot(
        Func&& func,
        Class* obj,
        int timeout = -1
    );

    /** @brief Retrieves the most recent frame from the internal buffer. @return nullptr if no frame is available. */
    std::shared_ptr<const CvImage> getLatestFrame();

    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        const std::string& value
    ) override;

    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        int value
    ) override;

    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        double value
    ) override;

    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        bool value
    ) override;

    TaskResult executeCommand(
        const std::string& paramId,
        const std::string& categoryId
    ) override;

    TaskResult resetParameter(
        const std::string& paramId,
        const std::string& categoryId
    ) override;

    TaskResult resetDirtyParameters() override;

    bool isDetected() const;

    bool isConnected() const;

    bool isGrabbing() const;

    /** @brief Returns true if a disconnect command has been enqueued but not yet finished. */
    bool disconnectionRequested() const;

    /** @brief Returns the current internal state. */
    CameraState state() const;

    /** 
     * @brief Set whether parameters edited current parameters should be reverted to the ones
     * first loaded. 
     * On a side note, parameters still could not survive persistence through power-cycles, so
     * it heavily depends on the interface type.
     * E.g.: GenICam persitence is often applied after reconnection, but not through power-cycles, unless through specific GenICam command.
     */
    void setParametersPersistence(bool arePersistent);

    /** @brief Get whether edited current parameters should be reverted to the ones
     * first loaded. */
    bool areParametersPersistent() const;

    /** @brief Returns a lightweight view-only object containing camera metadata. */
    std::shared_ptr<ICameraInfo> getCameraView() const;

    /** @brief Returns the unique hardware serial number. */
    const std::string& interfaceId() const;

    /** @brief Returns unique identifier that its interface expect to use. */
    const std::string& serialNumber() const;

    /** @brief Returns the interface type. */
    CameraInterface interfaceType() const;

    /** @brief Get the current acquisition mode from the hardware. */
    virtual TaskResultP<AcquisitionMode> acquisitionMode() const = 0;

protected:
    ICamera() = default;

	// Protected constructor to be called by derived classes.
    // Note that derived class must create as shared pointers
    ICamera(
        const std::string& serialNmb,
        const std::string& interfaceId,
        CameraInterface interfaceType
    );

    // SDK-specific child class must implements these

    virtual TaskResult checkCamera() = 0;

    virtual TaskResult checkConnectionHealth() = 0;

    virtual TaskResult connectCamera() = 0;

    virtual TaskResult loadCameraParams(std::vector<std::shared_ptr<Parameter>>& features) = 0;

    virtual TaskResult disconnectCamera() = 0;

    virtual TaskResult setAcquisitionMode(AcquisitionMode acqMode) = 0;

    virtual TaskResult startFrameCapture() = 0;

    virtual TaskResult stopFrameCapture() = 0;

    virtual TaskResultP<std::shared_ptr<const CvImage>> acquireFrame(int timeout) = 0;

    virtual TaskResult setStringParameter(
        const std::string& key,
        const std::string& value
    ) = 0;

    virtual TaskResult setIntParameter(
        const std::string& key,
        int value
    ) = 0;

    virtual TaskResult setDoubleParameter(
        const std::string& key,
        double value
    ) = 0;

    virtual TaskResult setBoolParameter(
        const std::string& key,
        bool value
    ) = 0;

    virtual TaskResult executeCommand(
        const std::string& key
    ) = 0;

private:
	// Internal struct to hold pending snapshot requests
    struct SnapshotRequest
    {
        std::move_only_function<void(const TaskResultP<std::shared_ptr<const CvImage>>)> func;
        std::chrono::steady_clock::time_point deadline;
    };

    TaskResultP<CameraState> doCheck();

    TaskResult doConnect();

    TaskResult doDisconnect();

    TaskResult doRequestStopGrabLoop();

    TaskResult doShutdown();

    void scheduleNextGrab(
        int timeout,
        int consecutiveGrabFails = 0
    );

    void doNextGrab(
        int timeout,
        int consecutiveGrabFails
    );

    // This is a "best effort stop":
	// It guarantees that the software loop is stopped,
    // but it does not guarantee that the hardware has stopped sending frames.
    void finalizeGrabLoop();

    void doTakeSnapshot(
        std::move_only_function<void(const TaskResultP<std::shared_ptr<const CvImage>>&)> func,
        int timeout
    );

    TaskResult stopGrabSync();

    TaskResult disconnectSync();

    void addImageToBuffer(const std::shared_ptr<const CvImage>& newImage);

    void notDetectedState();

    void detectedState();
    
    void connectedState();

    void disconnectedState();

    void grabState();

    void idleState();

    std::shared_ptr<const ParameterOwner> getSharedParameterOwner() const override;

private:
    std::string serialNumber_;
    std::string interfaceId_;
    CameraInterface interfaceType_;

    mutable std::shared_mutex stateMutex_;
    CameraStateMachine stateMachine_;

    std::atomic<bool> isShuttingDown_ { false };
    std::atomic<bool> isStartingGrab_ { false };
    std::atomic<bool> isStoppingGrab_ { false };
    std::atomic<bool> isConnecting_ { false };
    std::atomic<bool> isDisconnecting_ { false };

    std::atomic<bool> requestStopGrabbing_ { false };

    std::atomic<bool> persistentParams_{ false };

    // Mutex for protecting imageBuffer
    mutable std::shared_mutex bufferMutex_;

    // Circular buffer for images
    std::deque<std::shared_ptr<const CvImage>> imageBuffer_;

    std::mutex snapshotRequestsMutex_;
    std::deque<SnapshotRequest> snapshotRequests_;

private:
    // Concrete implementation of ParameterInfo and ParameterOwnerInfoImpl used to provide a read-only interface to a ICamera.
    class ICameraInfoImpl : public ICameraInfo, public ParameterOwnerInfoImpl
    {
    public:
        /** @throw std::invalid_argument if cam is nullptr. */
        ICameraInfoImpl(const std::shared_ptr<const ICamera>& cam);

        bool isDetected() const override;

        bool isConnected() const override;

        bool isGrabbing() const override;

        bool disconnectionRequested() const override;

        CameraState state() const override;

        bool areParametersPersistent() const override;

        const std::string& interfaceId() const override;

        const std::string& serialNumber() const override;

        CameraInterface interfaceType() const override;

    private:
        const std::shared_ptr<const ICamera> cam_;
    };
};

#include "ICamera.inl"
