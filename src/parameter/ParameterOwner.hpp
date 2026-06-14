#pragma once
#include <variant>
#include <unordered_set>
#include <shared_mutex>
#include <task_result/TaskResult.hpp>
#include "ParameterOwnerInfo.hpp"
#include "ParameterMap.hpp"
#include "Parameter.hpp"


/**
 * @brief Base class for objects that manage a collection of configuration parameters.
 * 
 * This class provides a framework for handling parameters organized by
 * Id and Category. It maintains a distinction between "Default" and "Current"
 * values, tracks "dirty" states for modified parameters, and is designed to be
 * thread-safe using shared mutexes.
 * This is an abstract base class; derived classes must implement
 * pure virtual methods.
 * The most dervied classes must inherit from 'std::enable_shared_from_this'.
 */
class ParameterOwner
{
public:
	//-----------------------------------------------------------------------------
	// Enums and structs
	//-----------------------------------------------------------------------------

	/**
	 * @brief DTO for a single parameter.
	 */
	struct DTO
	{
		std::string paramId;
		std::string categoryId;  // Full category path like "preprocessing.filters"
		std::variant<std::monostate, std::string, int, double, bool> value;
	};


	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	ParameterOwner() = default;

	virtual ~ParameterOwner() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	std::shared_ptr<ParameterOwnerInfo> getParameterOwnerView() const;

	/**
	 * @brief Updates a string parameter value.
	 * @param paramId Unique identifier for the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @param value The new value to set.
	 * @return TaskResult indicating success or specific failure reasons.
	 */
	virtual TaskResult setParameter(
		const std::string& paramId,
		const std::string& categoryId,
		const std::string& value
	);

	/**
	 * @brief Updates an int parameter value.
	 * @param paramId Unique identifier for the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @param value The new value to set.
	 * @return TaskResult indicating success or specific failure reasons.
	 */
	virtual TaskResult setParameter(
		const std::string& paramId,
		const std::string& categoryId,
		int value
	);

	/**
	 * @brief Updates a double parameter value.
	 * @param paramId Unique identifier for the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @param value The new value to set.
	 * @return TaskResult indicating success or specific failure reasons.
	 */
	virtual TaskResult setParameter(
		const std::string& paramId,
		const std::string& categoryId,
		double value
	);

	/**
	 * @brief Updates an boolean parameter value.
	 * @param paramId Unique identifier for the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @param value The new value to set.
	 * @return TaskResult indicating success or specific failure reasons.
	 */
	virtual TaskResult setParameter(
		const std::string& paramId,
		const std::string& categoryId,
		bool value
	);

	/**
	 * @brief Triggers a command-type parameter.
	 * @param paramId Unique identifier for the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @return TaskResult indicating success or specific failure reasons.
	 */
	virtual TaskResult executeCommand(
		const std::string& paramId,
		const std::string& categoryId
	);

	/**
	 * @brief Updates a parameter value.
	 * @param paramId Unique identifier for the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @param value The new value to set.
	 * @return TaskResult indicating success or specific failure reasons.
	 */
	virtual TaskResult setParameter(
		const std::string& paramId,
		const std::string& categoryId,
		const std::variant<std::monostate, std::string, int, double, bool>& value
	);

	/**
	 * @brief Updates a set of parameters.
	 * @param params Vector of DTO objects.
	 * @return TaskResult indicating success or specific failure reasons.
	 */
	virtual TaskResult setParameters(const std::vector<DTO>& params);

	/**
	 * @brief All dirty parameters are saved as default.
	 */
	virtual void saveParameters();

	/**
	 * @brief Reverts a parameter to its default value.
	 * @param paramId Unique identifier for the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @return TaskResult indicating success or specific failure reasons.
	 */
	virtual TaskResult resetParameter(
		const std::string& paramId,
		const std::string& categoryId
	);

	/** @brief Reverts all modified parameters to their default values. */
	virtual TaskResult resetDirtyParameters();

	/**
	 * @brief Checks if a specific parameter has been modified from its default state.
	 * @param paramId Unique identifier of the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @return true if the parameter is "dirty" (modified), false otherwise.
	 */
	bool isParameterDirty(
		const std::string& paramId,
		const std::string& categoryId
	) const;

	/**
	 * @brief Retrieves a shared pointer to a specific parameter.
	 * @param paramId Unique identifier of the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @return A shared_ptr to the ParameterInfo, or nullptr if not found.
	 */
	std::shared_ptr<ParameterInfo> getParameter(
		const std::string& paramId,
		const std::string& categoryId
	) const;

	/**
	 * @brief Retrieves a list of parameters based on filtering criteria.
	 * @param category (Optional) Filter by a specific category name.
	 * @param visibility (Optional) Filter by a specific VisibilityLevel.
	 * @param inAlphabeticalOrder If true, the resulting vector is sorted by parameter name.
	 * @return A vector of shared pointers to the matching parameters.
	 */
	std::vector<std::shared_ptr<ParameterInfo>> getFilteredParams(
		const std::optional<std::string>& category = std::nullopt,
		std::optional<VisibilityLevel> visibility = std::nullopt,
		bool inAlphabeticalOrder = false
	) const;


	/**
	 * @brief Retrieves the factory-default state of a specific parameter.
	 * @param paramId Unique identifier of the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @return A shared_ptr to the default ParameterInfo.
	 */
	std::shared_ptr<ParameterInfo> getDefaultParameter(
		const std::string& paramId,
		const std::string& categoryId
	) const;

	/**
	 * @brief Retrieves a list of default parameters based on filtering criteria.
	 * See 'getFilteredParams' for more details.
	 */
	std::vector<std::shared_ptr<ParameterInfo>> getDefaultFilteredParams(
		const std::optional<std::string>& category = std::nullopt,
		std::optional<VisibilityLevel> visibility = std::nullopt,
		bool inAlphabeticalOrder = false
	) const;

	/**
	 * @brief Retrieves a list of dirty parameters.
	 */
	std::vector<ParamKey> getDirtyParams() const;

	/** @brief Returns a list of all unique category identifiers managed by this owner. */
	std::vector<std::string> getAllCategories() const;

	/** @brief Returns all visibility levels currently utilized by the parameters. */
	std::vector<VisibilityLevel> getAllVisibilityLevels() const;

protected:
	// Internal helper to initialize parameters from a vector of Parameter objects.
	// Parameters are cloned to ensure the owner has its own copy of the data.
	// This is typically called by derived classes during their construction.
	void initParameters(const std::vector<std::shared_ptr<Parameter>>& params);

	// Internal helper to initialize parameters from a vector of Parameter objects.
	// Parameters are cloned to ensure the owner has its own copy of the data.
	// This is typically called by derived classes during their construction.
	void initParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params);

	// Internal helper to get a writable version of a defaultparameter.
	std::shared_ptr<Parameter> getDefaultWritableParam(
		const std::string& paramId,
		const std::string& categoryId
	) const;

	// Internal helper to get a writable version of a parameter.
	std::shared_ptr<Parameter> getCurrentWritableParam(
		const std::string& paramId,
		const std::string& categoryId
	) const;

	/**
	 * @brief Retrieves a shared pointer to a specific parameter, throws if not found.
	 * @param paramId Unique identifier of the parameter.
	 * @param categoryId The category the parameter belongs to.
	 * @return A shared_ptr to the ParameterInfo, or throws if not found.
	 * #@throws std::logic_error if parameter is not found.
	 */
	std::shared_ptr<ParameterInfo> getRequiredParameter(
		const std::string& paramId,
		const std::string& categoryId
	) const;

	// Manually flags a parameter as modified.
	void markParamAsDirty(
		const std::string& paramId,
		const std::string& categoryId,
		bool isDirty = true
	);

	// Clears all parameter definitions and reset states.
	void clearAllParameters();

	// Must be implemented by derived classes to return a shared reference to themselves.
	// This is to avoid diamond inheritance problem with 'std::enable_shared_from_this'
	virtual std::shared_ptr<const ParameterOwner> getSharedParameterOwner() const = 0;

protected:
	// Concrete implementation of ParameterOwnerInfo used to provide a read-only interface to a ParameterOwner.
	class ParameterOwnerInfoImpl : public virtual ParameterOwnerInfo
	{
	public:
		/** @throw std::invalid_argument if owner is nullptr. */
		ParameterOwnerInfoImpl(const std::shared_ptr<const ParameterOwner>& owner);

		bool isParameterDirty(
			const std::string& paramId,
			const std::string& categoryId
		) const override;

		std::shared_ptr<ParameterInfo> getParameter(
			const std::string& paramId,
			const std::string& categoryId
		) const override;

		std::vector<std::shared_ptr<ParameterInfo>> getFilteredParams(
			const std::optional<std::string>& category = std::nullopt,
			std::optional<VisibilityLevel> visibility = std::nullopt,
			bool inAlphabeticalOrder = false
		) const override;

		std::shared_ptr<ParameterInfo> getDefaultParameter(
			const std::string& paramId,
			const std::string& categoryId
		) const override;

		std::vector<std::shared_ptr<ParameterInfo>> getDefaultFilteredParams(
			const std::optional<std::string>& category = std::nullopt,
			std::optional<VisibilityLevel> visibility = std::nullopt,
			bool inAlphabeticalOrder = false
		) const override;

		std::vector<std::string> getAllCategories() const override;

		std::vector<VisibilityLevel> getAllVisibilityLevels() const override;

	private:
		const std::shared_ptr<const ParameterOwner> owner_;
	};

private:
	// Protects default value map.
	mutable std::shared_mutex defaultParametersMutex_;
	// Storage for original/default parameters.
	ParameterMap defaultParameters_;

	// Protects current value map.
	mutable std::shared_mutex currentParametersMutex_;
	// Storage for live parameter values.
	ParameterMap currentParameters_;

	// Protects the set of dirty parameter keys.
	mutable std::shared_mutex editedParamsMutex_;
	std::unordered_set<ParamKey, ParamKeyHash> editedParams_;
};
