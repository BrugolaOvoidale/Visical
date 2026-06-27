#pragma once
#include "OverexposureCheckDebug.hpp"
#include "../BoardPlugin.hpp"


/**
 * @brief Plugin for validating the board overxposure.
 * 
 * This plugin identifies pixels within a specific intensity range and calculates
 * a quality score based on the ratio of overexposed area to the total board area.
 * 
 * totalScore = 1.0 - ((over exposed area / board area) / maxAreaRatio)
 */
class OverexposureCheck : public Plugin<std::shared_ptr<Board>, OverexposureCheckDebug>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr unsigned int minThreshold{ 240 };
		static constexpr unsigned int maxThreshold{ 255 };
		static constexpr double maxAreaRatio{ 0.15 };
    };

    static constexpr std::string_view ID{ "overexposure" };
    static constexpr std::string_view NAME{ "Overexposure Check" };
    static constexpr std::string_view DESCRIPTION{ 
        R"(Detects overexposed areas in the board region
        totalScore = 1.0 - ((over exposed area / board area) / maxAreaRatio)
        )"
    };

    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~OverexposureCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<OverexposureCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    OverexposureCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(
        const std::shared_ptr<Board>& board,
        const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults = {}
    ) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Lower bound of the overexposure intensity. Valid range: (0, 254]
    unsigned int minThreshold{ Defaults::minThreshold };
    // Upper bound of the overexposure intensity. .Valid range : (0, 255]
    unsigned int maxThreshold{ Defaults::maxThreshold };

    // The ratio threshold where the quality score drops to zero
	double maxAreaRatio{ Defaults::maxAreaRatio };
};
