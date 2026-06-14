#include "CalibrationStageControllerSettings.hpp"


std::string CalibrationStageControllerSettings::getDefaultModelParamsPath() const
{
    return getConfigDirectory() +
        PATH_SEP +
        getModuleDirImpl() +
        PATH_SEP +
        getModelParamsFilePathImpl();
}

std::string CalibrationStageControllerSettings::getDefaultBoardPluginsPath() const
{
    return getConfigDirectory() +
        PATH_SEP +
        getModuleDirImpl() +
        PATH_SEP +
        BOARD_PLUGINS;
}

std::string CalibrationStageControllerSettings::getDefaultSequencePluginsPath() const
{
    return getConfigDirectory() +
        PATH_SEP +
        getModuleDirImpl() +
        PATH_SEP +
        SEQUENCE_PLUGINS;
}

void CalibrationStageControllerSettings::setBoardPluginsPath(const std::string& path)
{
    singleBoardPluginsPath_ = path;
}

const std::string& CalibrationStageControllerSettings::getBoardPluginsPath() const
{
    return singleBoardPluginsPath_;
}

void CalibrationStageControllerSettings::setSequencePluginsPath(const std::string& path)
{
    boardSeqPluginsPath_ = path;
}

const std::string& CalibrationStageControllerSettings::getSequencePluginsPath() const
{
    return boardSeqPluginsPath_;
}