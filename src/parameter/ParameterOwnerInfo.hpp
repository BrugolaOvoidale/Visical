#pragma once
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "ParameterDefs.hpp"


// Forward declarations
class ParameterInfo;


/**
 * @brief Abstract read-only interface for objects that manage and own a collection of parameters.
 */
class ParameterOwnerInfo
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	ParameterOwnerInfo() = default;

	~ParameterOwnerInfo() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

    /**
     * @brief Checks if a specific parameter has been modified from its default state.
     * @param paramId Unique identifier of the parameter.
     * @param categoryId The category the parameter belongs to.
     * @return true if the parameter is "dirty" (modified), false otherwise.
     */
    virtual bool isParameterDirty(
        const std::string& paramId,
        const std::string& categoryId
    ) const = 0;

    /**
     * @brief Retrieves a shared pointer to a specific parameter.
     * @param paramId Unique identifier of the parameter.
     * @param categoryId The category the parameter belongs to.
     * @return A shared_ptr to the ParameterInfo, or nullptr if not found.
     */
    virtual std::shared_ptr<ParameterInfo> getParameter(
        const std::string& paramId,
        const std::string& categoryId
    ) const = 0;

    /**
     * @brief Retrieves a list of parameters based on filtering criteria.
     * @param category (Optional) Filter by a specific category name.
     * @param visibility (Optional) Filter by a specific VisibilityLevel.
     * @param inAlphabeticalOrder If true, the resulting vector is sorted by parameter name.
     * @return A vector of shared pointers to the matching parameters.
     */
    virtual std::vector<std::shared_ptr<ParameterInfo>> getFilteredParams(
        const std::optional<std::string>& category = std::nullopt,
        std::optional<VisibilityLevel> visibility = std::nullopt,
        bool inAlphabeticalOrder = false
    ) const = 0;

    /**
     * @brief Retrieves the factory-default state of a specific parameter.
     * @param paramId Unique identifier of the parameter.
     * @param categoryId The category the parameter belongs to.
     * @return A shared_ptr to the default ParameterInfo.
     */
    virtual std::shared_ptr<ParameterInfo> getDefaultParameter(
        const std::string& paramId,
        const std::string& categoryId
    ) const = 0;

    /**
     * @brief Retrieves a list of default parameters based on filtering criteria.
     * See 'getFilteredParams' for more details.
     */
    virtual std::vector<std::shared_ptr<ParameterInfo>> getDefaultFilteredParams(
        const std::optional<std::string>& category = std::nullopt,
        std::optional<VisibilityLevel> visibility = std::nullopt,
        bool inAlphabeticalOrder = false
    ) const = 0;

    /** @brief Returns a list of all unique category identifiers managed by this owner. */
    virtual std::vector<std::string> getAllCategories() const = 0;

    /** @brief Returns all visibility levels currently utilized by the parameters. */
    virtual std::vector<VisibilityLevel> getAllVisibilityLevels() const = 0;
};