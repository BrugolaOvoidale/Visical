#include "ParameterOwner.hpp"
#include <UtilityFunctions.hpp>
#include "ParameterUtils.hpp"


std::shared_ptr<ParameterOwnerInfo> ParameterOwner::getParameterOwnerView() const
{
    return std::static_pointer_cast<ParameterOwnerInfo>(
        std::make_shared<ParameterOwnerInfoImpl>(getSharedParameterOwner())
    );
}

TaskResult ParameterOwner::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    const std::string& value)
{
    std::shared_ptr<Parameter> param = getCurrentWritableParam(paramId, categoryId);
    if (!param)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " does not exist in cache", Log::Level::LVL_ERROR } };
    }
    if (!param->isWritable())
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " is not writable", Log::Level::LVL_ERROR } };
    }
    const DataType dataType = param->dataType();
    if (dataType != DataType::STRING && dataType != DataType::ENUMERATION)
    {
        return { false,  { "Parameter " + paramId + " in category " + categoryId + " is neither string nor enumaration", Log::Level::LVL_ERROR } };
    }

    bool isDirty = getDefaultParameter(paramId, categoryId)->getValue<std::string>() != value;

    param->setValue(value);
    
    markParamAsDirty(param->name(), param->category(), isDirty);

    return true;
}

TaskResult ParameterOwner::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    int value)
{
    std::shared_ptr<Parameter> param = getCurrentWritableParam(paramId, categoryId);
    if (!param)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " does not exist in cache", Log::Level::LVL_ERROR } };
    }
    if (!param->isWritable())
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " is not writable", Log::Level::LVL_ERROR } };
    }
    const DataType dataType = param->dataType();
    if (dataType != DataType::INTEGER && dataType != DataType::ENUMERATION)
    {
        return { false,  { "Parameter " + paramId + " in category " + categoryId + " is neither int nor enumaration", Log::Level::LVL_ERROR } };
    }

    bool isDirty = getDefaultParameter(paramId, categoryId)->getValue<int>() != value;

    param->setValue(value);
    
    markParamAsDirty(param->name(), param->category(), isDirty);

    return true;
}

TaskResult ParameterOwner::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    double value)
{
    std::shared_ptr<Parameter> param = getCurrentWritableParam(paramId, categoryId);
    if (!param)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " does not exist in cache", Log::Level::LVL_ERROR } };
    }
    if (!param->isWritable())
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " is not writable", Log::Level::LVL_ERROR } };
    }
    if (param->dataType() != DataType::DOUBLE)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " is not double", Log::Level::LVL_ERROR } };
    }

    bool isDirty = getDefaultParameter(paramId, categoryId)->getValue<double>() != value;

    param->setValue(value);
    
    markParamAsDirty(param->name(), param->category(), isDirty);

    return true;
}

TaskResult ParameterOwner::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    bool value)
{
    std::shared_ptr<Parameter> param = getCurrentWritableParam(paramId, categoryId);
    if (!param)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " does not exist in cache", Log::Level::LVL_ERROR } };
    }
    if (!param->isWritable())
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " is not writable", Log::Level::LVL_ERROR } };
    }
    if (param->dataType() != DataType::BOOLEAN)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " is not bool", Log::Level::LVL_ERROR } };
    }

    bool isDirty = getDefaultParameter(paramId, categoryId)->getValue<bool>() != value;

    param->setValue(value);
    
    markParamAsDirty(param->name(), param->category(), isDirty);

    return true;
}

TaskResult ParameterOwner::executeCommand(
    const std::string& paramId,
    const std::string& categoryId)
{
    return true;
}

TaskResult ParameterOwner::setParameter(
    const std::string& paramId,
    const std::string& categoryId,
    const std::variant<std::monostate, std::string, int, double, bool>& value)
{
    std::shared_ptr<Parameter> param = getCurrentWritableParam(paramId, categoryId);
    if (!param)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " does not exist in cache", Log::Level::LVL_ERROR } };
    }
    if (!param->isWritable())
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " is not writable", Log::Level::LVL_ERROR } };
    }

    TaskResult setRes;

    switch (param->dataType())
    {
        case DataType::INTEGER:
            if (std::holds_alternative<int>(value))
            {
                setRes = setParameter(paramId, categoryId, std::get<int>(value));
            }
            else
            {
                setRes = { false, { "Parameter " + paramId + " in category " + categoryId + " requires an int value", Log::Level::LVL_ERROR } };
            }

            break;

        case DataType::DOUBLE:
            if (std::holds_alternative<double>(value))
            {
                setRes = setParameter(paramId, categoryId, std::get<double>(value));
            }
            else
            {
                setRes = { false, { "Parameter " + paramId + " in category " + categoryId + " requires a double value", Log::Level::LVL_ERROR } };
            }

            break;

        case DataType::BOOLEAN:
            if (std::holds_alternative<bool>(value))
            {
                setRes = setParameter(paramId, categoryId, std::get<bool>(value));
            }
            else
            {
                setRes = { false, { "Parameter " + paramId + " in category " + categoryId + " requires a bool value", Log::Level::LVL_ERROR } };
            }

            break;

        case DataType::STRING:
            if (std::holds_alternative<std::string>(value))
            {
                setRes = setParameter(paramId, categoryId, std::get<std::string>(value));
            }
            else
            {
                setRes = { false, { "Parameter " + paramId + " in category " + categoryId + " requires a string value", Log::Level::LVL_ERROR } };
            }

            break;

        case DataType::ENUMERATION:
            if (std::holds_alternative<int>(value) || std::holds_alternative<std::string>(value))
            {
                setRes = std::holds_alternative<int>(value) ? setParameter(paramId, categoryId, std::get<int>(value)) : setParameter(paramId, categoryId, std::get<std::string>(value));
            }
            else
            {
                setRes = { false, { "Parameter " + paramId + " in category " + categoryId + " requires a int or string value", Log::Level::LVL_ERROR } };
            }

            break;

        case DataType::COMMAND:
            setRes = executeCommand(paramId, categoryId);

            break;

        default:
            setRes = { false, { "Parameter " + paramId + " in category " + categoryId + " has an unsupported data type", Log::Level::LVL_ERROR } };
			
            break;
    }

    return setRes;
}

TaskResult ParameterOwner::setParameters(const std::vector<DTO>& params)
{
    std::vector<Log> logs;

    for (const auto& dto : params)
    {
        TaskResult r = setParameter(dto.paramId, dto.categoryId, dto.value);
        
        UtilityFunctions::moveInto(r.takeLogs(), logs);
	}

	return { true, std::move(logs) };
}

void ParameterOwner::saveParameters()
{
    std::vector<ParamKey> local_editedParams;

    {
        std::shared_lock lock(editedParamsMutex_);

        local_editedParams.reserve(editedParams_.size());

        for (const auto& pair : editedParams_) local_editedParams.push_back(std::move(pair));

		editedParams_.clear();
    }

    for (const auto& [paramId, categoryId] : local_editedParams)
    {
        std::shared_ptr<Parameter> currentP = getCurrentWritableParam(paramId, categoryId);
        if (currentP)
        {
            std::shared_ptr<Parameter> defaultP = getDefaultWritableParam(paramId, categoryId);
            if (currentP)
            {
                switch (defaultP->dataType())
                {
                    case DataType::INTEGER:
                        defaultP->setValue<int>(currentP->getValue<int>());

                        break;

                    case DataType::DOUBLE:
                        defaultP->setValue<double>(currentP->getValue<double>());

                        break;

                    case DataType::BOOLEAN:
                        defaultP->setValue<bool>(currentP->getValue<bool>());

                        break;

                    case DataType::STRING:
                        defaultP->setValue<std::string>(currentP->getValue<std::string>());

                        break;

                    case DataType::ENUMERATION:
                        defaultP->setValue<int>(currentP->getValue<int>());

                        break;

                    default:
						break;
                }
            }
        }
	}
}

TaskResult ParameterOwner::resetParameter(
    const std::string& paramId,
    const std::string& categoryId)
{
    std::shared_ptr<Parameter> param = getCurrentWritableParam(paramId, categoryId);
    if (!param)
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " does not exist in cache", Log::Level::LVL_ERROR } };
    }
    if (!param->isWritable())
    {
        return { false, { "Parameter " + paramId + " in category " + categoryId + " is not writable", Log::Level::LVL_ERROR } };
    }

    std::shared_ptr<ParameterInfo> defaultParam = getDefaultParameter(paramId, categoryId);
    switch (param->dataType())
    {
        case DataType::INTEGER:
            param->setValue(
                defaultParam->getValue<int>()
            );

            break;

        case DataType::DOUBLE:
            param->setValue(
                defaultParam->getValue<double>()
            );

            break;

        case DataType::ENUMERATION:
            param->setValue(
                defaultParam->getValue<int>()
            );

            break;

        case DataType::BOOLEAN:
            param->setValue(
                defaultParam->getValue<bool>()
            );

            break;

        case DataType::STRING:
            param->setValue(
                defaultParam->getValue<std::string>()
            );

            break;

        default:
            break;
    }
    
    markParamAsDirty(param->name(), param->category(), false);

    return true;
}

TaskResult ParameterOwner::resetDirtyParameters()
{
    std::vector<ParamKey> local_editedParams = getDirtyParams();

    std::vector<Log> logs;

    for (const auto& [paramId, categoryId] : local_editedParams)
    {
        TaskResult r = resetParameter(paramId, categoryId);

        UtilityFunctions::moveInto(r.takeLogs(), logs);
    }

    return { true, std::move(logs) };
}

bool ParameterOwner::isParameterDirty(
    const std::string& paramId,
    const std::string& categoryId) const
{
    std::shared_lock lock(editedParamsMutex_);

    return editedParams_.contains({ paramId, categoryId });
}

std::shared_ptr<ParameterInfo> ParameterOwner::getParameter(
    const std::string& paramId,
    const std::string& categoryId) const
{
    std::shared_lock lock(currentParametersMutex_);  // Shared (read) lock

    std::shared_ptr<Parameter> p = currentParameters_.getParameter(paramId, categoryId);

    if (!p)
        return nullptr;

    return p->getParameterView();
}

std::vector<std::shared_ptr<ParameterInfo>> ParameterOwner::getFilteredParams(
    const std::optional<std::string>& category,
    std::optional<VisibilityLevel> visibility,
    bool inAlphabeticalOrder) const
{
	std::vector<std::shared_ptr<Parameter>> filteredParams = currentParameters_.getFilteredParams(category, visibility, inAlphabeticalOrder);

    std::vector<std::shared_ptr<ParameterInfo>> views;
	views.reserve(filteredParams.size());

    for (const auto& param : filteredParams)
    {
        views.push_back(param->getParameterView());
	}

    return views;
}

std::shared_ptr<ParameterInfo> ParameterOwner::getDefaultParameter(
    const std::string& paramId,
    const std::string& categoryId) const
{
    std::shared_lock lock(defaultParametersMutex_);  // Shared (read) lock

    std::shared_ptr<Parameter> p = defaultParameters_.getParameter(paramId, categoryId);

    if (!p)
		return nullptr;

    return p->getParameterView();
}

std::vector<std::shared_ptr<ParameterInfo>> ParameterOwner::getDefaultFilteredParams(
    const std::optional<std::string>& category,
    std::optional<VisibilityLevel> visibility,
    bool inAlphabeticalOrder) const
{
    std::vector<std::shared_ptr<Parameter>> filteredParams = defaultParameters_.getFilteredParams(category, visibility, inAlphabeticalOrder);

    std::vector<std::shared_ptr<ParameterInfo>> views;
    views.reserve(filteredParams.size());

    for (const auto& param : filteredParams)
    {
        views.push_back(param->getParameterView());
    }

    return views;
}

std::vector<ParamKey> ParameterOwner::getDirtyParams() const
{
    std::shared_lock lock(editedParamsMutex_);

    return { editedParams_.begin(), editedParams_.end() };
}

std::vector<std::string> ParameterOwner::getAllCategories() const
{
    std::shared_lock lock(currentParametersMutex_);  // Shared (read) lock

    return currentParameters_.getAllCategories();
}

std::vector<VisibilityLevel> ParameterOwner::getAllVisibilityLevels() const
{
    std::shared_lock lock(currentParametersMutex_);  // Shared (read) lock

    return currentParameters_.getAllVisibilityLevels();
}

//////////////////////////////////////////////////////////////////////////////////////////////

void ParameterOwner::initParameters(const std::vector<std::shared_ptr<Parameter>>& params)
{
    std::vector<std::shared_ptr<Parameter>> defaultVec;
    defaultVec.reserve(params.size());

    std::vector<std::shared_ptr<Parameter>> currentVec;
    currentVec.reserve(params.size());

    for (const auto& p : params)
    {
        std::shared_ptr<Parameter> newP = ParameterUtils::clone(p);
        if (newP)
        {
            defaultVec.push_back(newP);
            currentVec.push_back(ParameterUtils::clone(newP));
        }
    }

    {
        std::unique_lock lock(defaultParametersMutex_);
        defaultParameters_ = ParameterMap(std::move(defaultVec));
    }

    {
        std::unique_lock lock(currentParametersMutex_);
        currentParameters_ = ParameterMap(std::move(currentVec));
    }
}

void ParameterOwner::initParameters(const std::vector<std::shared_ptr<ParameterInfo>>& params)
{
    std::vector<std::shared_ptr<Parameter>> defaultVec;
    defaultVec.reserve(params.size());

    std::vector<std::shared_ptr<Parameter>> currentVec;
    currentVec.reserve(params.size());

    for (const auto& p : params)
    {
        std::shared_ptr<Parameter> newP = ParameterUtils::cloneToParameter(p);
        if (newP)
        {
            defaultVec.push_back(newP);
            currentVec.push_back(ParameterUtils::clone(newP));
        }
    }

    {
        std::unique_lock lock(defaultParametersMutex_);
        defaultParameters_ = ParameterMap(std::move(defaultVec));
    }

    {
        std::unique_lock lock(currentParametersMutex_);
        currentParameters_ = ParameterMap(std::move(currentVec));
    }
}

std::shared_ptr<Parameter> ParameterOwner::getDefaultWritableParam(
    const std::string& paramId,
    const std::string& categoryId) const
{
    std::shared_lock lock(defaultParametersMutex_);  // Shared (read) lock

    return defaultParameters_.getParameter(paramId, categoryId);
}

std::shared_ptr<Parameter> ParameterOwner::getCurrentWritableParam(
    const std::string& paramId,
    const std::string& categoryId) const
{
    std::shared_lock lock(currentParametersMutex_);  // Shared (read) lock

    return currentParameters_.getParameter(paramId, categoryId);
}

std::shared_ptr<ParameterInfo> ParameterOwner::getRequiredParameter(
    const std::string& paramId,
    const std::string& categoryId) const
{
    std::shared_ptr<ParameterInfo> p = getParameter(paramId, categoryId);

    if (!p)
    {
        throw std::logic_error("Missing parameter: " + paramId + ", " + categoryId);
    }

    return p;
}

void ParameterOwner::markParamAsDirty(
    const std::string& paramName,
    const std::string& categoryId,
    bool isDirty)
{
    std::unique_lock<std::shared_mutex> lock(editedParamsMutex_); // Acquire exclusive (write) lock

    if (isDirty)
    {
        editedParams_.insert({ paramName, categoryId });
    }
    else
    {
        editedParams_.erase({ paramName, categoryId });
    }
}

void ParameterOwner::clearAllParameters()
{
    {
        std::unique_lock lock(currentParametersMutex_);         // Exclusive (write) lock
        currentParameters_.clear();
    }

    {
        std::unique_lock lock(defaultParametersMutex_);        // Exclusive (write) lock
        defaultParameters_.clear();
    }

    {
        std::unique_lock lock(editedParamsMutex_);              // Exclusive (write) lock
        editedParams_.clear();
    }
}


////////////////////////////////////////////////////////////////


ParameterOwner::ParameterOwnerInfoImpl::ParameterOwnerInfoImpl(const std::shared_ptr<const ParameterOwner>& owner) : owner_(owner)
{
    if (!owner_)
    {
        throw (std::invalid_argument("owner is nullptr"));
    }
}

bool ParameterOwner::ParameterOwnerInfoImpl::isParameterDirty(
    const std::string& paramId,
    const std::string& categoryId) const
{
    return owner_->isParameterDirty(paramId, categoryId);
}

std::shared_ptr<ParameterInfo> ParameterOwner::ParameterOwnerInfoImpl::getParameter(
    const std::string& paramId,
    const std::string& categoryId) const
{
    return owner_->getParameter(paramId, categoryId);
}

std::vector<std::shared_ptr<ParameterInfo>> ParameterOwner::ParameterOwnerInfoImpl::getFilteredParams(
    const std::optional<std::string>& category,
    std::optional<VisibilityLevel> visibility,
    bool inAlphabeticalOrder) const
{
    return owner_->getFilteredParams(category, visibility, inAlphabeticalOrder);
}

std::shared_ptr<ParameterInfo> ParameterOwner::ParameterOwnerInfoImpl::getDefaultParameter(
    const std::string& paramId,
    const std::string& categoryId) const
{
    return owner_->getDefaultParameter(paramId, categoryId);
}

std::vector<std::shared_ptr<ParameterInfo>> ParameterOwner::ParameterOwnerInfoImpl::getDefaultFilteredParams(
    const std::optional<std::string>& category,
    std::optional<VisibilityLevel> visibility,
    bool inAlphabeticalOrder) const
{
    return owner_->getDefaultFilteredParams(category, visibility, inAlphabeticalOrder);
}

std::vector<std::string> ParameterOwner::ParameterOwnerInfoImpl::getAllCategories() const
{
    return owner_->getAllCategories();
}

std::vector<VisibilityLevel> ParameterOwner::ParameterOwnerInfoImpl::getAllVisibilityLevels() const
{
    return owner_->getAllVisibilityLevels();
}