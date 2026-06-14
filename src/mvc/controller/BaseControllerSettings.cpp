#include "BaseControllerSettings.hpp"


std::string BaseControllerSettings::getConfigDirectory()
{
	return "config";
}

std::string BaseControllerSettings::getDefaultSettingsPath() const
{
	return getConfigDirectory() +
		PATH_SEP +
		getModuleDirImpl() +
		PATH_SEP +
		getDefaultSettingsFileName();
}

std::string BaseControllerSettings::getDefaultModelParamsPath() const
{
	return getConfigDirectory() +
		PATH_SEP +
		getModuleDirImpl() +
		PATH_SEP +
		getModelParamsFilePathImpl();
}

std::string BaseControllerSettings::getDefaultSettingsFileName() const
{
	return getDefaultSettingsFileNameImpl();
}

void BaseControllerSettings::setSettingsPath(const std::string& path)
{
	settingsPath_ = path;
}

const std::string& BaseControllerSettings::getSettingsPath() const
{
	return settingsPath_;
}

void BaseControllerSettings::setModelParamsPath(const std::string& path)
{
	modelParamsPath_ = path;
}

const std::string& BaseControllerSettings::getModelParamsPath() const
{
	return modelParamsPath_;
}