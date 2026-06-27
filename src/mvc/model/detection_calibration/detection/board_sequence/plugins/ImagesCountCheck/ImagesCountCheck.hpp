#pragma once
#include "../BoardSequencePlugin.hpp"


/**
 * @brief Check how many boards are in the sequence.
 */
class ImagesCountCheck : public Plugin<std::vector<std::shared_ptr<Board>>>
{
public:
    //-----------------------------------------------------------------------------
    // Structs and constants
    //-----------------------------------------------------------------------------

    /** @brief Default values for the plugin parameters */
    struct Defaults {
        static constexpr size_t minImagesCount{ 20 };
    };

    static constexpr std::string_view ID{ "images_count" };
    static constexpr std::string_view NAME{ "Images Count Check" };
    static constexpr std::string_view DESCRIPTION{ "Check how many images" };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ImagesCountCheck() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<ImagesCountCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    ImagesCountCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(
        const std::vector<std::shared_ptr<Board>>& boards,
        const std::unordered_map<std::string, std::shared_ptr<PluginResult>>& producersResults = {}
    ) const override;

    void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

public:
	// Minimum number of images required for the sequence to be considered valid.
    size_t minImagesCount{ Defaults::minImagesCount };
};
