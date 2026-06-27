#pragma once
#include "PatternParameters.hpp"


/**
 * @brief Configuration parameters for detecting circleboard calibration patterns.
 */
class CircleboardParameters : public PatternParameters
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Geometrical configuration parameters for circleboard pattern.
     */
    class Geometry : public PatternParameters::Geometry
    {
    public:
        //-----------------------------------------------------------------------------
        // Constructor/Destructor
        //-----------------------------------------------------------------------------

        /**
         * @brief Constructs parameters by providing internal corner counts.
         * @param patternWidth Number of inner corners along the width.
         * @param patternHeight Number of inner corners along the height.
         * @param markDiam Physical diameter of a single mark in mm.
         * @param centerDist Physical distance between marks centers in mm.
         */
        Geometry(
            int patternWidth,
            int patternHeight,
            float markDiam,
            float centerDist
        );
        Geometry(
            const cv::Size& patternSize,
            float markDiam,
            float centerDist
        );

        ~Geometry() = default;


        //-----------------------------------------------------------------------------
        // Class members
        //-----------------------------------------------------------------------------

        // Physical size in world units (mm)
        float centerDistance;

        // Physical size in world units (mm)
        float markDiameter;
    };

    /**
     * @brief Configuration parameters for detecting circleboard calibration patterns.
     */
    struct Detection
    {
    public:
        //-----------------------------------------------------------------------------
        // Enums and structs
        //-----------------------------------------------------------------------------

        /** @brief Default values for all circleboard parameters. */
        struct Defaults {
            static constexpr bool useClustering{ false };
        };


        //-----------------------------------------------------------------------------
        // Struct members
        //-----------------------------------------------------------------------------

        // Use clustering to improve detection robustness
        bool useClustering{ Defaults::useClustering };
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs parameters.
     * @param g Geometrical parameters of the circleboard.
     * @param isAsymmetric Set the 'BoardPattern' type.
     */
    CircleboardParameters(
        Geometry g,
        bool isAsymmetric
    );

    ~CircleboardParameters() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Geometrical parameters of the circleboard. */
    const Geometry& getGeometry() const override;


    //-----------------------------------------------------------------------------
    // Class members
    //-----------------------------------------------------------------------------

    /** @brief Geometrical parameters of the circleboard. */
    Geometry geometry;

    /** @brief Parameters used for detecting the circleboard */
    Detection detection;
};
