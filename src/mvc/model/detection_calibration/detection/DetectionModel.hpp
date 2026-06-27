#pragma once
#include <evaluator/Evaluator.hpp>
#include <cv/CvImage.hpp>
#include "DetectionModelMessages.hpp"
#include "DetectionResultMap.hpp"
#include "../common/CalibrationStageModel.hpp"


// Forward declarations
class CameraManager;
class SharedCameraIntrinsics;
class PatternParameters;
class Detector;
class ICameraProxy;


/**
 * @brief The central engine for board detection and evaluation.
 *
 * This class coordinates camera/file streams, image processing, and board evaluation logic.
 * It operates asynchronously by inheriting from AsyncWorker and manages system parameters
 * via ParameterOwner. It supports both live camera sessions and file-based batch processing.
 *
 * Requires 'std::shared_ptr' management.
 */
class DetectionModel : public CalibrationStageModel, public std::enable_shared_from_this<DetectionModel>
{
public:
    //-----------------------------------------------------------------------------
    // Enums and aliases
    //-----------------------------------------------------------------------------

    /** @brief Defines the source of the detection data. */
    enum class SessionType {
        UNDEFINED,  ///< Initial state.
        FILE,       ///< Processing images from disk.
        CAMERA      ///< Processing images from a live camera stream.
    };

    using ParameterOwner::setParameter;


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~DetectionModel() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create and initialize a DetectionModel instance.
     * 
     * Note that this class takes no responsibility on shutting down the CameraManager.
     * 
     * @param cameraManager Pointer to the system camera controller.
     * @param camIntrinsics Shared camera calibration parameters.
     * @param logger Optional logger for system messages.
     * @return A shared_ptr to the initialized model.
     * @throws If cameraManager or camIntrinsics are nullptr.
     */
    static std::shared_ptr<DetectionModel> create(
        const std::shared_ptr<CameraManager>& cameraManager,
        const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
        const std::shared_ptr<MessageLogger>& logger = nullptr
    );


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Gracefully stops session and clear the internal resources.
     */
    void shutdown() override;

    /**
     * @brief Initiates a background task to process a list of image files.
     * @param filePaths List of absolute paths to images.
     * @returns REJECTED if file paths is empty, NO_ERRORS otherwise.
     */
    TaskEnqueueResult startFileSession(const std::vector<std::string>& filePaths);

    /**
     * @brief Starts a live session from a specific camera.
     * @param cameraId The camera serial number.
     * @param snap If true, takes a single snapshot instead of a continuous stream.
     * @returns REJECTED if camera Id is empty, NO_ERRORS otherwise.
     */
    TaskEnqueueResult startCameraSession(
        const std::string& cameraId,
        bool snap = false
    );

    /** 
     * @brief Requests the termination of the current session.
     * @returns ALREADY_DONE if session is not running, ALREADY_ACTIVE if a stop request is in progress, NO_ERRORS otherwise.
     */
    TaskEnqueueResult stopSession();

    /** @brief Returns true if a session is on. */
    bool isSessionOn() const;

    /** @brief Returns true if the current CAMERA session is from a camera live stream. */
    bool isLiveSession() const;

    /** @brief Returns true if a session is starting. */
    bool isStartingSession() const;

    /** @brief Returns the current session type. */
    SessionType sessionType() const;

    /** @brief Returns the optional Id of the currently active camera. */
    std::optional<std::string> getWorkingCameraId() const;

    /** @brief Retrieves a result using the Map-specific Id. */
    std::optional<std::shared_ptr<DetectionResult>> getDetectionResult(DetectionResultMap::Id id) const;

    /** @brief Retrieves a result using the global DetectionResult Id. */
    std::optional<std::shared_ptr<DetectionResult>> getDetectionResult(DetectionResult::Id id) const;

    /** @brief Retrieves a temporary result from the live buffer. */
    std::optional<std::shared_ptr<DetectionResult>> getLiveDetectionResult(DetectionResult::Id id) const;

    /** @brief Returns the most recent result from the live stream. */
    std::shared_ptr<DetectionResult> getLatestLiveDetectionResult() const;

    /** @brief Returns all currently stored detection results. */
    std::map<DetectionResultMap::Id, std::shared_ptr<DetectionResult>> getAllDetectionResults() const;

    /** @brief Returns a list of all boards that have been detected. See 'Board::isDetected'. */
    std::vector<std::shared_ptr<EvaluatedBoard>> getDetectedBoards() const;

    /** @brief Accesses the raw images associated with the detection results. */
    std::map<DetectionResultMap::Id, std::shared_ptr<const CvImage>> getAllOriginalImages() const;

    /** @brief Gets the result of the latest sequence evaluation. */
    const std::shared_ptr<EvaluatedBoardSequence>& getEvaluatedSequence();

    /** @brief Returns the count of total detections in the current session. */
    size_t countDetectionResults() const;

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
        bool value
    ) override;

    TaskResult resetParameter(
        const std::string& paramId,
        const std::string& categoryId
    ) override;

    TaskResult resetDirtyParameters() override;

    /**
     * @brief Removes a specific result.
     * @return The Id of the next available result. See 'BidirectionalObjectRegistry::remove'.
     */
    std::optional<DetectionResultMap::Id> removeDetectionResults(DetectionResultMap::Id toRemoveId);

    /** @brief Clears all stored results and original images. */
    void removeAllDetectionsResults();

    /**
     * @brief Transfers a result from the volatile live buffer to the persistent registry.
     * @param frameId The Id of the live frame to promote.
     * @return The new Id in the persistent registry.
     */
    std::optional<DetectionResultMap::Id> promoteToSnapshot(DetectionResult::Id frameId);

private:
	// Private constructor used by the factories.
    DetectionModel(
        const std::shared_ptr<CameraManager>& cameraManager,
        const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
        const std::shared_ptr<MessageLogger>& logger = nullptr
    );

    // Initializes internal resources.
    void init();

    //  Sets the active camera by Id.
    void setWorkingCameraId(const std::string& cameraId);

    // Clears current session-related resources.
    void clearSession();

    // Initiates a live acquisition task.
    TaskEnqueueResult startLive(const std::string& cameraId);

    // Takes a single snapshot from the active camera.
    TaskEnqueueResult takeSnapshot(const std::string& cameraId);

    // Enqueue a board detection from a live captured frame task.
    TaskEnqueueResult findBoardFromLiveTask(const std::string& cameraId);

    // Schedules the next live detection.
    void scheduleNextLiveDetection(
        const std::shared_ptr<ICameraProxy>& cam,
        const std::shared_ptr<Detector>& detector
    );

	// Finalizes the live detection loop.
    void finalizeLiveDetectionLoop(const std::shared_ptr<ICameraProxy>& cam);

    // Enqueue a board detection from a snapshot captured frame task.
    TaskEnqueueResult findBoardFromSnapTask(const std::shared_ptr<const CvImage>& image);

    // Enqueue a re-detection of all stored boards task.
    TaskEnqueueResult rerunAllBoards();

    // Enqueue a re-evaluation of all stored boards task.
    TaskEnqueueResult reEvaluateAllBoards() override;

    // Enqueue a re-evaluation of all stored boards task, specifying which plugin.
    TaskEnqueueResult reEvaluateAllBoards(
        const std::string& id,
        bool enabled = true
    ) override;

    // Enqueue a re-evaluation of all stored boards sequence task.
    TaskEnqueueResult reEvaluateSequence() override;

    // Implementation of re-evaluation sequence logic, with specific plugin.
    TaskEnqueueResult reEvaluateSequence(
        const std::string& id,
        bool enabled = true
    ) override;

    // Implementation of the live loop logic.
    void doFindBoardFromLive(
        const std::shared_ptr<ICameraProxy>& cam,
        const std::shared_ptr<Detector>& detector
    );

    // Implementation of the board detection from image files on disk.
    TaskResult doFindBoardFromImages(const std::vector<std::string>& filePaths);

    // Implementation of board detection from a captured frame.
    TaskResult doFindBoardFromSnap(const std::shared_ptr<const CvImage>& originalImage);

    // Implementation of re-detection logic.
    TaskResult doRerunAllBoards();

    // Implementation of re-evaluation all logic.
    TaskResult doReEvaluateAllBoards();

    // Implementation of re-evaluation all logic, with specific plugin.
    TaskResult doReEvaluateAllBoardsSingleCheck(
        const std::string& id,
        bool enabled
    );
    
    // Implementation of re-evaluation sequence logic.
    TaskResultP<std::shared_ptr<EvaluatedBoardSequence>> doReEvaluateSequence();

    // Enqueue a re-evaluation of all stored boards sequence task, specifying which plugin.
    TaskResultP<std::shared_ptr<EvaluatedBoardSequence>> doReEvaluateSequenceSingleCheck(
        const std::string& id,
        bool enabled
    );

    // Applies preprocessing to the image before detection.
    CvImage preprocessImage(const CvImage& input);

    // Persists a detection result and its non-preprocessed image and returns its assigned Id.
    DetectionResultMap::Id storeDetectionResult(
        const std::shared_ptr<DetectionResult>& result,
        const std::shared_ptr<const CvImage>& originalImage
    );

    // Persists the evaluation result of the whole sequence.
    void storeLatestEvaluatedSequence(const std::shared_ptr<EvaluatedBoardSequence>& evalSequence);

    // Update a detection result with a new one, keeping the the Map-specific Id.
    bool updateDetectionResult(DetectionResult::Id id, const std::shared_ptr<DetectionResult>& board);

    // Update a detection result with a new one, keeping the the Map-specific Id.
    bool updateDetectionResult(DetectionResultMap::Id id, const std::shared_ptr<DetectionResult>& board);

	// Temporary (see 'liveRetention_') store a live detection result in the volatile buffer,
    // associating it with its global DetectionResult Id.
    void storeLiveDetectionResult(
        const std::shared_ptr<DetectionResult>& result,
        const std::shared_ptr<const CvImage>& originalImage
    );

	// Removes expired detection results from the live buffer based on the defined retention time.
    void evictExpiredLiveDetectionResults(const std::chrono::steady_clock::time_point& now);

    // Create the parameters of this domain
    void createDomainParameters();

    const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& getBoardPluginsFactory() const override;
    
    const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& getSequencePluginsFactory() const override;

    TaskResultP<std::shared_ptr<PatternParameters>> getDetectionParameters() const;

    std::shared_ptr<const ParameterOwner> getSharedParameterOwner() const override;


    //-----------------------------------------------------------------------------
    // Messages
    //-----------------------------------------------------------------------------

    // Catch camera start grab message and start the live acquisition task.
    void cameraStartGrab(const MessageTask& message);
    void onCameraStartGrab(const MessageTask& message);

    // Catch camera stop grab message and stop the live acquisition task.
    void internalCameraStopGrab(const MessageTask& message);
    void externalCameraStopGrab(const MessageTask& message);
    void onCameraStopGrab(const MessageTask& message);

    // Catch camera take snapshot message and start the snapshot acquisition task.
    void cameraTakeSnap(const TaskResultP<std::shared_ptr<const CvImage>>& snapResult);
    void onCameraTakeSnap(const TaskResultP<std::shared_ptr<const CvImage>>& snapResult);

private:
    // Indicates the type of the current session.
    std::atomic<SessionType> sessionType_{ SessionType::UNDEFINED };

    // Control flag to indicate if a current CAMERA session is from a camera live stream.
    std::atomic<bool> isLiveSession_{ false };

	// Control flag to indicate if a session is currently active.
    std::atomic<bool> isSessionOn_{ false };

    // Control flag to indicate if a session start is requested.
    std::atomic<bool> requestStartSession_{ false };

	// Control flag to indicate if a session termination is requested.
    std::atomic<bool> requestStopSession_{ false };

    // Control flag for the re-run boards request
    std::atomic<bool> rerunRequested_{ false };

	// Control flag to indicate if a re-run boards task is currently running.
    std::atomic<bool> isRerunningBoard_{ false };

	// Control flag if the board sequence changed, therefore the latest evaluation sequence is no longer valid.
    std::atomic<bool> isEvalSequenceInvalid_{ false };

	// Camera manager.
    std::shared_ptr<CameraManager> cameraManager_;

	// Shared camera intrinsics.
    std::shared_ptr<SharedCameraIntrinsics> camIntrinsics_;

    //
    mutable std::shared_mutex currCameraIdMutex_;
    std::optional<std::string> currCameraId_;

    //
    mutable std::shared_mutex detectionsMutex_;
    DetectionResultMap detections_;

    //
    mutable std::shared_mutex originalImagesMutex_;
    std::map<DetectionResultMap::Id, std::shared_ptr<const CvImage>> originalImages_;

    //
    mutable std::shared_mutex lastEvalSequenceMutex_;
    std::shared_ptr<EvaluatedBoardSequence> lastEvalSequence_;

	// Volatile buffer for live detection results, which are not persisted in the main
    // registry until promoted. This allows to keep the main registry clean from
    // transient detection results that are only relevant for the live stream, and to
    // implement a retention policy to automatically evict old live results after a
    // certain time.
    mutable std::shared_mutex liveResultsMutex_;

    // Ordered by time (oldest first)
    std::deque<std::shared_ptr<DetectionResult>> liveQueue_;

    std::map<DetectionResult::Id, std::shared_ptr<DetectionResult>> liveMap_;

    // Ordered by time (oldest first)
    std::deque<std::shared_ptr<const CvImage>> liveOrigImgsQueue_;

    std::map<DetectionResult::Id, std::shared_ptr<const CvImage>> liveOrigImgsMap_;

	// Defines how long a live detection result should be kept in the volatile buffer
    // before being automatically evicted. This allows to ensure that the live buffer
    // does not grow indefinitely with old results, while still keeping recent detections
    // available for retrieval and potential promotion to the main registry.
    // The retention time can be adjusted based on the expected frequency of detections
    // and the desired responsiveness of the live stream. For example, a retention time
    // of 1000 milliseconds means that a live detection result will be automatically
    // removed from the volatile buffer if it is older than 1 second, ensuring that only
    // recent detections are kept for quick access and promotion.
    const std::chrono::milliseconds liveRetention_ = std::chrono::milliseconds(1000);

	// Evaluator for single board evaluation.
    std::shared_ptr<Evaluator<std::shared_ptr<Board>>> boardEval_;

	// Evaluator for sequence of boards evaluation.
    std::shared_ptr<Evaluator<std::vector<std::shared_ptr<Board>>>> boardSeqEval_;
};
