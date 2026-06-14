#include "ParameterRegistry.hpp"


std::vector<std::shared_ptr<ParameterInfo>> ParameterRegistry::getAllParameters() const
{
	return parameters_.getFilteredParams();
}

std::shared_ptr<ParameterInfo> ParameterRegistry::getParameter(
	const std::string& name,
	const std::string& category
) const
{
	return parameters_.getParameter(name, category);
}

/////////////////////////////////////////////////

void ParameterRegistry::registerParameter(const std::shared_ptr<ParameterInfo>& parameter)
{
	if (!parameter) return;

	parameters_.addParameter(parameter);
}
