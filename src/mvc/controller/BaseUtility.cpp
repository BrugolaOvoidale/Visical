#include "BaseUtility.hpp"
#include <fstream>
#include <parameter/ParameterInfo.hpp>


TaskResultP<std::vector<ParameterOwner::DTO>> BaseUtility::loadModelParams(const std::string& path)
{
    std::vector<ParameterOwner::DTO> configs;

    try
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return {
                std::nullopt,
                { "Failed to open file: " + path, Log::Level::LVL_ERROR}
            };
        }

        nlohmann::json j;
        file >> j;

        flattenJson(j, "", configs);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const std::exception& e)
    {
        return {
            std::nullopt,
            { "Exception in " + path + ": " + e.what(), Log::Level::LVL_ERROR }
        };
    }

    return configs;
}

TaskResult BaseUtility::saveModelParams(
    const std::string& path,
    const std::vector<std::shared_ptr<ParameterInfo>>& params)
{
    try
    {
        nlohmann::json j;

        writeParams(j, params);

        std::ofstream file(path);
        if (!file.is_open())
        {
            return {
                false,
                { "Failed to open file: " + path, Log::Level::LVL_ERROR}
            };
        }

        file << j.dump(4);

        if (file.fail())
        {
            return {
                false,
                { "Failed to write file: " + path, Log::Level::LVL_ERROR}
            };
        }
    }
    catch (const nlohmann::json::type_error& e)
    {
        return {
            false,
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::out_of_range& e)
    {
        return {
            false,
            { "Missing required field in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////

std::variant<std::monostate, std::string, int, double, bool> BaseUtility::jsonToVariant(const nlohmann::json& j)
{
    if (j.is_boolean())
        return j.get<bool>();

    if (j.is_number_integer())
        return j.get<int>();

    if (j.is_number_float())
        return j.get<double>();

    if (j.is_string())
        return j.get<std::string>();

    return std::monostate{};  // null or unsupported types
}

void BaseUtility::assignParamValue(
    nlohmann::json& node,
    const std::shared_ptr<ParameterInfo>& param)
{
    const auto& name = param->name();

    switch (param->dataType())
    {
    case DataType::BOOLEAN:
        node[name] = param->getValue<bool>();
        break;

    case DataType::DOUBLE:
        node[name] = param->getValue<double>();
        break;

    case DataType::INTEGER:
        node[name] = param->getValue<int>();
        break;

    case DataType::ENUMERATION:
    case DataType::STRING:
        node[name] = param->getValue<std::string>();
        break;

    default:
        break;
    }
}

nlohmann::json& BaseUtility::getCategoryNode(
    nlohmann::json& root,
    const std::string& category)
{
    std::stringstream ss(category);
    std::string part;
    nlohmann::json* node = &root;

    while (std::getline(ss, part, '.'))
        node = &((*node)[part]);

    return *node;
}

void BaseUtility::writeParams(
    nlohmann::json& root,
    const std::vector<std::shared_ptr<ParameterInfo>>& params)
{
    for (const auto& param : params)
    {
        auto& catNode = getCategoryNode(root, param->category());
        assignParamValue(catNode, param);
    }
}

void BaseUtility::assignParamValue(
    nlohmann::ordered_json& node,
    const std::shared_ptr<ParameterInfo>& param)
{
    const auto& name = param->name();

    switch (param->dataType())
    {
    case DataType::BOOLEAN:
        node[name] = param->getValue<bool>();
        break;

    case DataType::DOUBLE:
        node[name] = param->getValue<double>();
        break;

    case DataType::INTEGER:
        node[name] = param->getValue<int>();
        break;

    case DataType::ENUMERATION:
    case DataType::STRING:
        node[name] = param->getValue<std::string>();
        break;

    default:
        break;
    }
}

nlohmann::ordered_json& BaseUtility::getCategoryNode(
    nlohmann::ordered_json& root,
    const std::string& category)
{
    std::stringstream ss(category);
    std::string part;
    nlohmann::ordered_json* node = &root;

    while (std::getline(ss, part, '.'))
        node = &((*node)[part]);

    return *node;
}

void BaseUtility::writeParams(
    nlohmann::ordered_json& root,
    const std::vector<std::shared_ptr<ParameterInfo>>& params)
{
    for (const auto& param : params)
    {
        auto& catNode = getCategoryNode(root, param->category());
        assignParamValue(catNode, param);
    }
}

void BaseUtility::flattenJson(
    const nlohmann::json& node,
    const std::string& category,
    std::vector<ParameterOwner::DTO>& configs)
{
    for (const auto& [key, value] : node.items())
    {
        if (value.is_object())
        {
            std::string nextCategory =
                category.empty() ? key : category + Parameter::CATEGORY_SEP + key;

            flattenJson(value, nextCategory, configs);
        }
        else
        {
            ParameterOwner::DTO p;
            p.paramId = key;
            p.categoryId = category;
            p.value = BaseUtility::jsonToVariant(value);
            configs.push_back(p);
        }
    }
}

// Recursively traverse nested parameter structure and extract parameters
void BaseUtility::extractParameters(
    const nlohmann::json& node,
    const std::string& categoryPath,
    std::vector<ParameterOwner::DTO>& parameters)
{
    if (!node.is_object())
        return;
    
    for (auto it = node.begin(); it != node.end(); ++it)
    {
        const std::string& key = it.key();
        const nlohmann::json& value = it.value();
        
        // Check if this is a parameter object (has "id" and "value")
        if (value.is_object() && value.contains("value"))
        {
            ParameterOwner::DTO param;
            param.paramId = key;
            param.categoryId = categoryPath;
            param.value = BaseUtility::jsonToVariant(value.at("value"));
            parameters.push_back(param);
        }
        // Otherwise, it's a nested category -> recurse
        else if (value.is_object())
        {
            std::string newPath = categoryPath.empty() 
                ? key 
                : categoryPath + Parameter::CATEGORY_SEP + key;
            extractParameters(value, newPath, parameters);
        }
    }
}

// Helper to split category string by separator
std::vector<std::string> BaseUtility::splitCategory(const std::string& category)
{
    std::vector<std::string> parts;

    if (Parameter::CATEGORY_SEP.empty())
    {
        if (!category.empty())
        {
            parts.push_back(category);
        }
        return parts;
    }

    size_t start = 0;
    size_t end = category.find(Parameter::CATEGORY_SEP);

    while (end != std::string::npos)
    {
        std::string part = category.substr(start, end - start);
        if (!part.empty())
        {
            parts.push_back(part);
        }
        start = end + Parameter::CATEGORY_SEP.length();
        end = category.find(Parameter::CATEGORY_SEP, start);
    }

    // Add the last part
    std::string lastPart = category.substr(start);
    if (!lastPart.empty())
    {
        parts.push_back(lastPart);
    }

    return parts;
}

// Helper to extract parameter value as a json-compatible type based on DataType
nlohmann::json BaseUtility::parameterValueToJson(const ParameterInfo& param)
{
    switch (param.dataType())
    {
    case DataType::BOOLEAN:     return param.getValue<bool>();
    case DataType::INTEGER:     return param.getValue<int>();
    case DataType::DOUBLE:      return param.getValue<double>();
    case DataType::STRING:      return param.getValue<std::string>();
    default:
                                return nullptr; // unknown/unsupported type
    }
}
