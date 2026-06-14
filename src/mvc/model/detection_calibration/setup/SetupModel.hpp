#pragma once
#include <parameter/ParameterOwner.hpp>
#include <calibration/CameraIntrinsics.hpp>


// Forward declarations
class SharedCameraIntrinsics;


/**
 * @brief Manages the configuration and state of the camera setup and its intrinsics.
 * 
 * This class acts as a high-level controller for camera parameters, supporting
 * different operational modes (Automatic, Guided, Free).
 * It inherits from ParameterOwner to integrate with the application's parameters system.
 */
class SetupModel : public ParameterOwner, public std::enable_shared_from_this<SetupModel>
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	~SetupModel() = default;


	//-----------------------------------------------------------------------------
	// Construction from factories
	//-----------------------------------------------------------------------------

	/** @brief Initializes the model as a self-contained module. */
	static std::shared_ptr<SetupModel> create();

	/** @brief Initializes the model with an external SharedCameraIntrinsics. */
	static std::shared_ptr<SetupModel> create(const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics);


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

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

	TaskResult setParameter(
		const std::string& paramId,
		const std::string& categoryId,
		const std::variant<std::monostate, std::string, int, double, bool>& value
	) override;

	TaskResult setParameters(const std::vector<DTO>& params) override;

	TaskResult resetParameter(
		const std::string& paramId,
		const std::string& categoryId
	) override;

	TaskResult resetDirtyParameters() override;

	/**
	 * @brief Switches camera intrinsics to automatic mode.
	 * See CamIntrinsics::automatic() for more details.
	 */
	TaskResult automaticMode();

	/**
	 * @brief Switches camera intrinsics to from hardware mode.
	 * See CamIntrinsics::fromHardware() for more details.
	 */
	TaskResult fromHardwareMode();
	
	/**
	 * @brief Switches camera intrinsics to free mode.
	 * See CamIntrinsics::free() for more details.
	 */
	TaskResult freeMode();

	/** @brief Returns the underlying camera intrinsics object. */
	std::shared_ptr<SharedCameraIntrinsics> getCamIntrinsics() const;

	/** @brief Retrieves metadata for all camera intrinsic parameters. */
	std::vector<std::shared_ptr<ParameterInfo>> getCamIntrinsicsParameters() const;

	/** @brief Retrieves metadata for parameters related to the distortion model. */
	std::vector<std::shared_ptr<ParameterInfo>> getDistortionModelParameters() const;

	/** @brief Retrieves metadata for parameters related to the camera matrix (K). */
	std::vector<std::shared_ptr<ParameterInfo>> getCamMatrixParameters() const;

private:
	// Private constructor for self-contained creation.
	SetupModel();

	// Private constructor for dependency injection.
	SetupModel(const std::shared_ptr<SharedCameraIntrinsics>& camIntrinsics);

	// Internal initialization logic.
	void init();

	// Sets up the internal SharedCameraIntrinsics state.
	void buildCamIntrinsics();

	// Implementation of ParameterOwner interface for shared access.
	std::shared_ptr<const ParameterOwner> getSharedParameterOwner() const override;

private:
	// Managed camera intrinsics component.
	std::shared_ptr<SharedCameraIntrinsics> camIntrinsics_;

	// Controls recompute of camera intrinsics if parameters are beign setted in batch
	bool paramsBatchUpdating_{ false };
};