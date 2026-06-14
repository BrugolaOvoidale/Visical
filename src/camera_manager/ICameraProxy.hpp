#pragma once
#include <camera/ICamera.hpp>


/**
 * @brief Acts as proxy for an ICamera instance. See ICamera for full methods documentation.
 */
class ICameraProxy
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ICameraProxy(const std::shared_ptr<ICamera>& iCam);

    ~ICameraProxy() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Subscribes a class member function to a specific message tag.
     * See 'MessageDispatcher::subscribe' for more details.
     */
    template<typename MessageType, typename Class>
        requires std::derived_from<MessageType, Message>
    MessageSubscription<MessageType>& subscribe(
        const MessageTag<MessageType>& tag,
        void (Class::* method)(const MessageType&),
        Class* obj
    );

    /**
     * @brief Subscribes a generic callable (lambda, functor) to a specific message tag.
     * See 'MessageDispatcher::subscribe' for more details.
     */
    template<typename MessageType, typename Callable>
        requires std::derived_from<MessageType, Message>
    MessageSubscription<MessageType>& subscribe(
        const MessageTag<MessageType>& tag,
        Callable&& fn
    );

    /**
     * @brief Removes a previously registered class member subscription.
     * See 'MessageDispatcher::unsubscribe' for more details.
     */
    template<typename MessageType, typename Class>
        requires std::derived_from<MessageType, Message>
    void unsubscribe(
        const MessageTag<MessageType>& tag,
        void (Class::* method)(const MessageType&),
        Class* obj
    );

    TaskEnqueueResult check();

    TaskEnqueueResult connect();

    TaskEnqueueResult disconnect();

    TaskEnqueueResult startGrab(int timeout = -1);

    TaskEnqueueResult stopGrab();

    TaskEnqueueResult takeSnapshot(
        std::move_only_function<void(const TaskResultP<std::shared_ptr<const CvImage>>&)> func,
        int timeout = -1
    );

    template<typename Func, typename Class>
    TaskEnqueueResult takeSnapshot(
        Func&& func,
        Class* obj,
        int timeout = -1
    );

    std::shared_ptr<const CvImage> getLatestFrame();

    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        const std::string& value
    );

    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        int value
    );

    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        double value
    );

    TaskResult setParameter(
        const std::string& paramId,
        const std::string& categoryId,
        bool value
    );

    TaskResult executeCommand(
        const std::string& paramId,
        const std::string& categoryId
    );

    TaskResult resetParameter(
        const std::string& paramId,
        const std::string& categoryId
    );


    bool isParameterDirty(
        const std::string& paramId,
        const std::string& categoryId
    ) const;

    std::shared_ptr<ParameterInfo> getParameter(
        const std::string& paramId,
        const std::string& categoryId
    ) const;

    std::vector<std::shared_ptr<ParameterInfo>> getFilteredParams(
        const std::optional<std::string>& category = std::nullopt,
        std::optional<VisibilityLevel> visibility = std::nullopt,
        bool inAlphabeticalOrder = false
    ) const;


    std::shared_ptr<ParameterInfo> getDefaultParameter(
        const std::string& paramId,
        const std::string& categoryId
    ) const;

    std::vector<std::shared_ptr<ParameterInfo>> getDefaultFilteredParams(
        const std::optional<std::string>& category = std::nullopt,
        std::optional<VisibilityLevel> visibility = std::nullopt,
        bool inAlphabeticalOrder = false
    ) const;

    std::vector<std::string> getAllCategories() const;

    std::vector<VisibilityLevel> getAllVisibilityLevels() const;

    bool isDetected() const;

    bool isConnected() const;

    bool isGrabbing() const;

    bool disconnectionRequested() const;

    CameraState state() const;

    void setParametersPersistence(bool arePersistent);

    bool areParametersPersistent() const;

    std::shared_ptr<ICameraInfo> getCameraView() const;

    const std::string& interfaceId() const;

    const std::string& serialNumber() const;

    CameraInterface interfaceType() const;

    TaskResultP<AcquisitionMode> acquisitionMode() const;

private:
    const std::shared_ptr<ICamera> iCam_;
};

#include "ICameraProxy.inl"
