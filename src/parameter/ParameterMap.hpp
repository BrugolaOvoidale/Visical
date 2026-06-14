#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include "ParameterDefs.hpp"


// Forward declarations
class Parameter;


/**
 * @brief A structured unordered map for managing and filtering application parameters.
 * 
 * This class organizes parameters using a two-tier hierarchy: 'Category' (string)
 * and 'VisibilityLevel' (enum).
 */
class ParameterMap
{
public:
	//-----------------------------------------------------------------------------
	// Aliases
	//-----------------------------------------------------------------------------

	/** @brief Maps visibility levels to a collection of parameters. */
	using VisibilityInfoMap = std::unordered_map<VisibilityLevel, std::vector<std::shared_ptr<Parameter>>>;

	/** @brief Maps category names to their respective VisibilityInfoMap. */
	using CategoryInfoMap = std::unordered_map<std::string, VisibilityInfoMap>;


	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	/**
	 * @brief Constructs the map, optionally pre-populating it with parameters.
	 * @param params A vector of shared pointers to Parameter objects.
	 */
	ParameterMap(const std::vector<std::shared_ptr<Parameter>>& params = {});

	~ParameterMap() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

    /**
     * @brief Registers a new parameter in the map.
     * @details The parameter is automatically placed based on its internal
     * category and visibility attributes.
     * @param param The parameter to manage.
     */
    void addParameter(const std::shared_ptr<Parameter>& param);

    /**
     * @brief Retrieves a specific parameter by name and category.
     * @param paramName The unique name of the parameter.
     * @param category The category the parameter belongs to.
     * @return A shared pointer to the Parameter, or nullptr if not found.
     */
    std::shared_ptr<Parameter> getParameter(
        const std::string& paramName,
        const std::string& category
    ) const;

    /**
     * @brief Retrieves a subset of parameters based on criteria.
     * @param category If provided, only returns parameters from this category.
     * @param visibility If provided, only returns parameters with this visibility level.
     * @param inAlphabeticalOrder If true, the resulting vector is sorted by parameter name.
     * @return A vector of shared pointers matching the filters.
     */
    std::vector<std::shared_ptr<Parameter>> getFilteredParams(
        const std::optional<std::string>& category = std::nullopt,
        std::optional<VisibilityLevel> visibility = std::nullopt,
        bool inAlphabeticalOrder = false
    ) const;

    /** @brief Returns a list of all unique categories currently registered. */
    std::vector<std::string> getAllCategories() const;

    /** @brief Returns a list of all unique visibility levels present in the map. */
    std::vector<VisibilityLevel> getAllVisibilityLevels() const;

    /** @brief Clears the internal maps. */
    void clear();

private:
    // Internal storage using a nested map structure.
    CategoryInfoMap categoryMap_;
};