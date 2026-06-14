#pragma once
#include <evaluator/Evaluator.hpp>
#include <cv/CvImage.hpp>
#include <calibration/CameraIntrinsicsRegistry.hpp>
#include <calibration/DistortionModelRegistry.hpp>
#include <calibration/calibrator/CalibrationParameters.hpp>
#include "CalibrationModelMessages.hpp"
#include "BoardMap.hpp"
#include "../common/CalibrationStageModel.hpp"


// Forward declarations
class SharedCameraIntrinsics;
class CameraIntrinsics;


/**
 * @brief The central engine for board detection and evaluation.
 *
 * This class manages the calibration process, performing camera calibration,
 * and providing interfaces for undistorting images and retrieving calibration results.
 * It integrates with evaluators for both individual calibrated boards and calibrated
 * camera model, allowing for comprehensive evaluation and feedback during the calibration
 * process.
 * The model also manages parameters related to calibration and provides mechanisms for
 * logging messages throughout the process.
 *
 * Requires 'std::shared_ptr' management.
 */
class CalibrationModel : public CalibrationStageModel, public std::enable_shared_from_this<CalibrationModel>
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~CalibrationModel() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create and initialize a CalibrationModel instance.
     * @param camIntrinsics Shared camera calibration parameters.
     * @param logger Optional logger for system messages.
     * @return A shared_ptr to the initialized model.
     */
    static std::shared_ptr<CalibrationModel> create(
        const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
        const std::shared_ptr<MessageLogger>& logger = nullptr
    );


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Replaces the current working dataset with a new set of boards.
     * @param boards Vector of shared pointers to Board objects.
     */
    void setWorkingDataset(const std::vector<std::shared_ptr<Board>>& boards);

    /**
     * @brief High-level command to trigger the camera calibration process.
     * @return REJECTED if working dataset is empty, NO_ERRORS otherwise.
     */
    TaskEnqueueResult calibrateCamera();

    /**
     * @brief Undistorts an image using the current calibration parameters.
     * @param image The input image.
     * @return A task result containing the undistorted CvImage.
     */
    TaskResultP<CvImage> undistort(const CvImage& image) const;

    /** @brief Gets the read-only interfaces of the calibration-related parameters. */
    std::vector<std::shared_ptr<ParameterInfo>> getCalibrationParametersInfo();

    /** @brief Retrieves a board using the Map-specific Id. */
    std::optional<std::shared_ptr<Board>> getBoard(BoardMap::Id id) const;

    /** @brief Returns the current dataset. */
    std::map<BoardMap::Id, std::shared_ptr<Board>> getWorkingDataset() const;

    /** @brief Returns the total number of boards in the current dataset. */
    size_t countBoards() const;

    /**
     * @brief Removes a specific board.
     * @return The Id of the next available board. See 'ObjectRegistry::remove'.
     */
    std::optional<BoardMap::Id> removeBoard(BoardMap::Id toRemoveId);

    /** @brief Clears all boards from the working dataset. */
    void clearDataset();

    /** @brief Returns the result of the most recent successful calibration run. */
    std::optional<CalibrationResult> getLastCalibrationResult() const;

private:
    // Private constructor used by the factories.
    CalibrationModel(
        const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics,
        const std::shared_ptr<MessageLogger>& logger = nullptr
    );

    // Initializes internal resources.
    void init();

    // The actual worker function executed on a background thread for calibration.
    TaskResultP<CalibrationResult> calibrateCameraTask();

    // Enqueue a re-evaluation of all stored boards task.
    TaskEnqueueResult reEvaluateAllBoards() override;

    // Enqueue a re-evaluation of all stored boards task, specifying which plugin.
    TaskEnqueueResult reEvaluateAllBoards(
        const std::string& pluginId,
        bool enabled = true
    ) override;

    // Enqueue a re-evaluation of all stored boards sequence task.
    TaskEnqueueResult reEvaluateSequence() override;

    // Implementation of re-evaluation sequence logic, with specific plugin.
    TaskEnqueueResult reEvaluateSequence(
        const std::string& pluginId,
        bool enabled = true
    ) override;

    // Implementation of re-evaluation all logic.
    void doReEvaluateAllBoards();

    // Implementation of re-evaluation all logic, with specific plugin.
    void doReEvaluateAllBoardsSingleCheck(
        const std::string& id,
        bool enabled
    );

    // Implementation of re-evaluation sequence logic.
    void doReEvaluateSequence();

    // Enqueue a re-evaluation of all stored boards sequence task, specifying which plugin.
    void doReEvaluateSequenceSingleCheck(
        const std::string& id,
        bool enabled
    );

    // Create the parameters of this domain
    std::optional<CalibrationResult> createCalibrationResult(
        const std::shared_ptr<CameraIntrinsics>& camIntrinsics,
        std::vector<EvaluatedCalibratedBoardEntry> evalCalibBoards,
        EvaluatedCameraModel evalCamModel
    ) const;

    // Checks if current parameter values are valid.
    void createDomainParameters();

    const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& getBoardPluginsFactory() const override;

    const std::unordered_map<std::string_view, std::function<std::shared_ptr<PluginContext>()>>& getSequencePluginsFactory() const override;

    void validateCalibrationParameters();

    std::optional<CalibrationParameters> getCalibrationParameters();

    std::shared_ptr<const ParameterOwner> getSharedParameterOwner() const override;

    void onCamIntrinsicsChanged(const Message& msg);

private:
    // Shared camera intrinsics.
    std::shared_ptr<SharedCameraIntrinsics> camIntrinsics_;

    // Protects workingDataset_.
    mutable std::shared_mutex workingDatasetMutex_;

    // The collection of boards currently being processed.
    BoardMap workingDataset_;

    // Protects lastCalibResult_.
    mutable std::shared_mutex lastCalibResultMutex_;
    std::optional<CalibrationResult> lastCalibResult_;

    // Cache of the last calibration output.
    std::shared_ptr<Evaluator<CalibratedBoard>> boardEval_;

    // Evaluates individual board quality.
    std::shared_ptr<Evaluator<CameraModel>> boardSeqEval_;

    CameraIntrinsicsRegistry camParamsRegistry_;

    DistortionModelRegistry distModelRegistry_;
};
