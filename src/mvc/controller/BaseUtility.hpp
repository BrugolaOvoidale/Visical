#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include <task_result/TaskResult.hpp>
#include <parameter/ParameterOwner.hpp>


// Forward declarations
class ParameterInfo;


/**
 * @brief Utility provider for parameter serialization and JSON manipulation.
 *
 * BaseUtility serves as the foundation for handling the conversion between
 * internal ParameterInfo objects and persistent JSON storage.
 */
class BaseUtility
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    BaseUtility() = default;

    virtual ~BaseUtility() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Loads model parameters from a JSON file.
     * @param path The filesystem path to the .json file.
     * @return A TaskResult containing a vector of DTOs on success.
     */
    static TaskResultP<std::vector<ParameterOwner::DTO>> loadModelParams(const std::string& path);

    /**
     * @brief Serializes and saves a collection of parameters to a JSON file.
     * @param path The destination filesystem path.
     * @param params Shared pointers to the parameters to be saved.
     * @return TaskResult indicating success or specific failure details.
     */
    static TaskResult saveModelParams(
        const std::string& path,
        const std::vector<std::shared_ptr<ParameterInfo>>& params
    );

protected:
    /**
     * @brief Converts a JSON basic type to a C++ variant.
     * @param j The JSON node (expected to be a primitive type).
     * @return A variant containing monostate, string, int, double, or bool.
     */
    static std::variant<std::monostate, std::string, int, double, bool> jsonToVariant(const nlohmann::json& j);

    /**
     * @brief Assigns a parameter's value to a specific JSON node.
     */
    static void assignParamValue(nlohmann::json& node, const std::shared_ptr<ParameterInfo>& param);
    static void assignParamValue(nlohmann::ordered_json& node, const std::shared_ptr<ParameterInfo>& param);

    /**
     * @brief Traverses or creates a nested JSON structure based on a category string.
     * @param root The root JSON object.
     * @param category A string (e.g., "Camera/Calibration/Intrinsics").
     * @return Reference to the specific category node.
     */
    static nlohmann::json& getCategoryNode(nlohmann::json& root, const std::string& category);
    static nlohmann::ordered_json& getCategoryNode(nlohmann::ordered_json& root, const std::string& category);

    /**
     * @brief Batch writes parameters into the provided JSON root.
     */
    static void writeParams(nlohmann::json& root, const std::vector<std::shared_ptr<ParameterInfo>>& params);
    static void writeParams(nlohmann::ordered_json& root, const std::vector<std::shared_ptr<ParameterInfo>>& params);

    /**
     * @brief Recursively flattens a nested JSON structure into a flat vector of DTOs.
     * @param node The starting JSON node.
     * @param category The current accumulated category path.
     * @param[out] configs The vector to populate with flattened data.
     */
    static void flattenJson(
        const nlohmann::json& node,
        const std::string& category,
        std::vector<ParameterOwner::DTO>& configs
    );

    /**
     * @brief Deep-extracts parameter data from a JSON hierarchy.
     */
    static void extractParameters(
        const nlohmann::json& node,
        const std::string& categoryPath,
        std::vector<ParameterOwner::DTO>& parameters
    );

    /**
     * @brief Splits a category string into individual tokens.
     */
    static std::vector<std::string> splitCategory(const std::string& category);

    /** @brief Converts a single ParameterInfo object into a JSON representation. */
    static nlohmann::json parameterValueToJson(const ParameterInfo& param);
};
