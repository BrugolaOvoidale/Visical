#include "CalibrationStageUtility.hpp"
#include <fstream>
#include <parameter/Parameter.hpp>
#include <evaluator/PluginContextInfo.hpp>
#include <evaluator/PluginInfo.hpp>
#include <calibration/Board.hpp>
#include <cv/CvPainter.hpp>
#include <CvToWxImageConverter.hpp>


CalibrationStageUtility::CalibrationStageUtility()
{
    painter_ = std::make_unique<CvPainter>();
}

CalibrationStageUtility::~CalibrationStageUtility() = default;

//////////////////////////////////////////////////

TaskResultP<std::vector<BoardEvaluationEngine::PluginDTO>>
CalibrationStageUtility::loadEvaluationPlugins(const std::string& path)
{
    std::vector<BoardEvaluationEngine::PluginDTO> configs;

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

        nlohmann::json j = nlohmann::json::parse(file);

        // Handle both array format and single plugin format
        if (j.is_array())
        {
            for (const auto& pluginJson : j)
            {
                configs.push_back(parsePluginConfig(pluginJson));
            }
        }
        else if (j.is_object())
        {
            // Single plugin configuration
            configs.push_back(parsePluginConfig(j));
        }
        else
        {
            return {
                std::nullopt,
                { "Invalid JSON format: expected object or array", Log::Level::LVL_ERROR}
            };
        }
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::type_error& e)
    {
        return {
            std::nullopt,
            { "JSON parse error in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
        };
    }
    catch (const nlohmann::json::out_of_range& e)
    {
        return {
            std::nullopt,
            { "Missing required field in " + path + ": " + e.what(), Log::Level::LVL_ERROR}
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

TaskResult CalibrationStageUtility::saveEvaluationPlugins(
    const std::string& path,
    const std::vector<std::shared_ptr<PluginContextInfo>>& cfg)
{
    try
    {
        nlohmann::ordered_json j = nlohmann::ordered_json::array();
        for (const auto& c : cfg)
        {
            j.push_back(toJson(*c));
        }

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
    catch (const std::exception& e)
    {
        return {
            false,
            { "Exception while saving config in " + path + ": " + e.what(), Log::Level::LVL_ERROR }
        };
    }

    return true;
}

wxImage CalibrationStageUtility::convertBoardImageToWx(const std::shared_ptr<Board>& board) const
{
    CvImage bgrImg = board->image().toBGR();

    //
    if (shouldDrawBoard() && board->isDetected())
    {
        painter_->setColour({ 0, 255, 0 }).drawContour(bgrImg, board->boardContour());
    }

    //
    if (shouldDrawMarks() && board->isDetected())
    {
        painter_->setColour({ 255, 255, 0 });

		const std::vector<CvContour>& marksContours = board->marksContours();

        for (const auto& m : marksContours)
        {
            painter_->drawContour(bgrImg, m);
        }
    }

    //
    if (shouldDrawWCS() && board->hasPose())
    {
        std::vector<cv::Point> intAxes2D;
        {
            const std::vector<cv::Point2f>& axes2D = board->axes();
            intAxes2D.reserve(axes2D.size());

            for (const auto& p : axes2D)
            {
                intAxes2D.emplace_back(
                    static_cast<int>(std::round(p.x)),
                    static_cast<int>(std::round(p.y))
                    );
            }
        }

        cv::Mat cvMat = bgrImg.mat();

        if (intAxes2D.size() >= 4)
        {
            cv::arrowedLine(cvMat, intAxes2D[0], intAxes2D[1], { 0, 0, 255 }, 2);
            cv::arrowedLine(cvMat, intAxes2D[0], intAxes2D[2], { 0, 255, 0 }, 2);
            cv::arrowedLine(cvMat, intAxes2D[0], intAxes2D[3], { 255, 0, 0 }, 2);

            int font = cv::FONT_HERSHEY_SIMPLEX;
            double fontScale = 0.6;
            int thickness = 2;

            auto drawAxisLabel = [&](const std::string& text,
                const cv::Point2f& origin,
                const cv::Point2f& tip,
                const cv::Scalar& color)
                {
                    cv::Point2f dir = tip - origin;
                    float length = std::sqrt(dir.dot(dir));

                    if (length > 0.0f)
                        dir /= length; // normalize

                    float offset = 20.0f; // distance beyond arrow tip
                    cv::Point2f textPos = tip + dir * offset;

                    cv::putText(cvMat,
                        text,
                        textPos,
                        font,
                        fontScale,
                        color,
                        thickness);
                };

            drawAxisLabel("X", intAxes2D[0], intAxes2D[1], { 0, 0, 255 });
            drawAxisLabel("Y", intAxes2D[0], intAxes2D[2], { 0, 255, 0 });
            drawAxisLabel("Z", intAxes2D[0], intAxes2D[3], { 255, 0, 0 });
        }
    }

    
    return CvToWxImageConverter::toWxImage(bgrImg);
}

void CalibrationStageUtility::drawBoard(bool draw)
{
    drawBoard_.store(draw);
}

bool CalibrationStageUtility::shouldDrawBoard() const
{
    return drawBoard_.load();
}

void CalibrationStageUtility::drawMarks(bool draw)
{
    drawMarks_.store(draw);
}

bool CalibrationStageUtility::shouldDrawMarks() const
{
    return drawMarks_.load();
}

void CalibrationStageUtility::drawWCS(bool draw)
{
    drawWCS_.store(draw);
}

bool CalibrationStageUtility::shouldDrawWCS() const
{
    return drawWCS_.load();
}

//////////////////////////////////////////////////

BoardEvaluationEngine::PluginDTO CalibrationStageUtility::parsePluginConfig(const nlohmann::json& pluginJson)
{
    BoardEvaluationEngine::PluginDTO config;

    config.id = pluginJson.at("id").get<std::string>();
    config.threshold = pluginJson.at("threshold").get<double>();
    config.enabled = pluginJson.at("enabled").get<bool>();

    if (pluginJson.contains("parameters") && pluginJson["parameters"].is_object())
    {
        extractParameters(pluginJson["parameters"], "", config.parameters);
    }

    return config;
}

nlohmann::ordered_json CalibrationStageUtility::toJson(const PluginContextInfo& context)
{
    const std::shared_ptr<PluginInfo>& pluginInfo = context.getPluginInfo();

    nlohmann::ordered_json parametersJson = nlohmann::ordered_json::object();

    for (const auto& param : context.getFilteredParams())
    {
        const std::vector<std::string> categoryParts = BaseUtility::splitCategory(param->category());

        // Navigate to the nested category location
        nlohmann::ordered_json* current = &parametersJson;
        for (const auto& part : categoryParts)
        {
            if (!current->contains(part))
            {
                (*current)[part] = nlohmann::ordered_json::object();
            }
            current = &(*current)[part];
        }

        // Add parameter at the leaf level, keyed by parameter id
        (*current)[param->name()] = {
            { "value", BaseUtility::parameterValueToJson(*param) }
        };
    }

    return {
        { "id",         pluginInfo->id() },
        { "threshold",  pluginInfo->getThreshold() },
        { "enabled",    context.isPluginEnabled() },
        { "parameters", parametersJson }
    };
}