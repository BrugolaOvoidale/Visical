#pragma once
#include <atomic>
#include <wx/image.h>
#include <nlohmann/json.hpp>
#include <mvc/model/detection_calibration/BoardEvaluationEngine.hpp>
#include "../../BaseUtility.hpp"


// Forward declarations
class Board;
class CvPainter;


/**
 * @brief Utility class for managing calibration stage logic, UI rendering state, and plugin serialization.
 *
 * This class extends BaseUtility to provide specific tools for board image conversion,
 * drawing toggles for visualization (WCS, marks, boards), and static methods to handle
 * the persistence of evaluation plugins.
 */
class CalibrationStageUtility : public BaseUtility
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    CalibrationStageUtility();

    ~CalibrationStageUtility();


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Loads evaluation plugin configurations from a JSON file.
     * @param path File system path to the JSON configuration.
     * @return A TaskResult containing a vector of PluginDTO objects on success.
     */
    static TaskResultP<std::vector<BoardEvaluationEngine::PluginDTO>> loadEvaluationPlugins(const std::string& path);

    /**
     * @brief Saves the current evaluation plugin contexts to a JSON file.
     * @param path Target file system path.
     * @param cfg Vector of shared pointers to plugin context information.
     * @return TaskResult indicating success or failure of the I/O operation.
     */
    static TaskResult saveEvaluationPlugins(
        const std::string& path,
        const std::vector<std::shared_ptr<PluginContextInfo>>& cfg
    );

    /**
     * @brief Converts a Board's internal image representation to a format compatible with wxWidgets.
     * @param board The board object containing the image data.
     * @return A wxImage ready for UI rendering.
     */
    wxImage convertBoardImageToWx(const std::shared_ptr<Board>& board) const;

    /** @brief Manage visibility of the board contour. */
    void drawBoard(bool draw);

    /** @brief Return whether the board contour should be drawn. */
    bool shouldDrawBoard() const;

    /** @brief Manage visibility of the marks contours. */
    void drawMarks(bool draw);

    /** @brief Return whether the marks contours should be drawn. */
    bool shouldDrawMarks() const;

    /** @brief Manage visibility of the WCS axes. */
    void drawWCS(bool draw);

    /** @brief Return whether the WCS axes should be drawn. */
    bool shouldDrawWCS() const;

protected:
    // Internal helper to parse a JSON object into a PluginDTO.
    static BoardEvaluationEngine::PluginDTO parsePluginConfig(const nlohmann::json& pluginJson);

    // Converts a PluginContextInfo object into an ordered JSON structure.
    static nlohmann::ordered_json toJson(const PluginContextInfo& context);

protected:
    std::unique_ptr<CvPainter> painter_;

private:
    std::atomic<bool> drawBoard_{ false };

    std::atomic<bool> drawMarks_{ false };

    std::atomic<bool> drawWCS_{ false };
};
