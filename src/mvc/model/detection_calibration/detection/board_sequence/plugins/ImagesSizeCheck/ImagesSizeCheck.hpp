#pragma once
#include "../BoardSequencePlugin.hpp"


/**
 * @brief Check wheter all images have same sizes.
 */
class ImagesSizeCheck : public Plugin<std::vector<std::shared_ptr<Board>>>
{
public:
    //-----------------------------------------------------------------------------
    // Constants
    //-----------------------------------------------------------------------------

    static constexpr std::string_view ID{ "images_size" };
    static constexpr std::string_view NAME{ "Images Size Check" };
    static constexpr std::string_view DESCRIPTION{ "Check wheter all images have same sizes" };


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Factory method to create a plugin instance.
     * @param threshold Initial score threshold.
     * @return A shared pointer to the newly created plugin.
     */
    static std::shared_ptr<ImagesSizeCheck> create(double threshold = DEFAULT_THRESHOLD);

private:
    // Private constructor to ensure instantiation via factory methods.
    ImagesSizeCheck(double threshold);

    std::shared_ptr<PluginResult> executeImpl(const std::vector<std::shared_ptr<Board>>& boards) const override;
    
	void validateParameters() const override;

    std::string getMessageForSeverity(EvaluationSeverity severity) const override;

};
