#pragma once
#include "ParameterInfoMap.hpp"


/**
 * @brief Abstract base class for managing and accessing parameters.
 *
 * This class acts as a central repository for ParameterInfo objects.
 * Derived classes must implement the registration logic to populate the registry.
 * It provides thread-safe-style lookup by name and category.
 */
class ParameterRegistry
{
public:
	//-----------------------------------------------------------------------------
	// Constructor/Destructor
	//-----------------------------------------------------------------------------

	ParameterRegistry() = default;

	virtual ~ParameterRegistry() = default;


	//-----------------------------------------------------------------------------
	// Class operators
	//-----------------------------------------------------------------------------

	/**
	 * @brief Retrieves a list of all registered parameters.
	 * @return A vector containing shared pointers to all ParameterInfo objects.
	 */
	std::vector<std::shared_ptr<ParameterInfo>> getAllParameters() const;

	/**
	 * @brief Searches for a specific parameter by name and category.
	 * @param name The unique identifier of the parameter.
	 * @param category The parameter category.
	 * @return A pointer to the found parameter, or nullptr if no match exists.
	 */
	std::shared_ptr<ParameterInfo> getParameter(
		const std::string& name,
		const std::string& category
	) const;

protected:
	// Adds a parameter to the internal map. See 'ParameterInfoMap::addParameter()' for more details
	void registerParameter(const std::shared_ptr<ParameterInfo>& parameter);

private:
	ParameterInfoMap parameters_;
};