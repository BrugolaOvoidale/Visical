#pragma once
#include "../BoardPlugin.hpp"


/**
 * @brief Plugin to verify if a detected board occupies a sufficient area of the image.
 * 
 * The plugin estimates the effective visible board area while compensating
 * for perspective distortion caused by board tilt relative to the camera.
 * 
 * score = ((board_area * cos(tilt_angle)) / image_area) / expected_coverage
 */
class BoardSizeCheck : public Plugin<std::shared_ptr<Board>>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr double expectedCoverage{ 0.2 };
    };

    static constexpr std::string_view ID{ "board_size" };
    static constexpr std::string_view NAME{ "Board Size Check" };
    static constexpr std::string_view DESCRIPTION{
        R"(Check if the detected board in image is at least the desired portion of the image
        
        score = ((board_area * cos(tilt_angle)) / image_area) / expected_coverage
        )"
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~BoardSizeCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<BoardSizeCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    BoardSizeCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(const std::shared_ptr<Board>& board) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
    // Minimum required ratio of board area to image area. Valid range: (0, 1].
    double expectedCoverage{ Defaults::expectedCoverage };
};
