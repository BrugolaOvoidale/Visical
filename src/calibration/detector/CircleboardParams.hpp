#pragma once
#include "PatternParams.hpp"


/**
 * @brief Configuration parameters for detecting circleboard-style calibration patterns.
 */
class CircleboardParams : public PatternParams
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /** @brief Default values for all circleboard parameters. */
    struct Defaults {
        static constexpr bool useClustering{ false };
    };

    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs parameters.
     * @param patternWidth Number of inner corners along the width.
     * @param patternHeight Number of inner corners along the height.
     * @param markDiam Physical diameter of a single mark in mm.
     * @param centerDist Physical distance between marks centers in mm.
     * @param isAsymmetric Set the 'BoardPattern' type.
     */
    CircleboardParams(
        int patternWidth,
        int patternHeight,
        float markDiam,
        float centerDist,
		bool isAsymmetric
    );

    /** @overload */
    CircleboardParams(
        const cv::Size& patternSize,
        float markDiameter,
        float centerDistance,
		bool isAsymmetric
    );

    ~CircleboardParams() = default;

    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the physical size of a single mark. */
    float markDiameter() const;

    /** @brief Returns the physical distance between marks centers. */
    float centerDistance() const;

public:
    // Use clustering to improve detection robustness
    bool useClustering{ Defaults::useClustering };
    
private:
    // Physical size in world units (mm)
    float centerDistance_;

    // Physical size in world units (mm)
    float markDiameter_;
};
