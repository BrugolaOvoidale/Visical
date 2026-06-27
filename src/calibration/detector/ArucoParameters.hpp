#pragma once
#include <opencv2/objdetect/charuco_detector.hpp>
#include "PatternParameters.hpp"


/**
 * @brief Configuration parameters for detecting ArUco calibration patterns.
 */
class ArucoParameters : public PatternParameters
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Configuration parameters for detecting ArUco calibration patterns.
     */
    struct Detection
    {
    public:
        //-----------------------------------------------------------------------------
        // Structs
        //-----------------------------------------------------------------------------

        /** @brief Default values for all ArUco parameters. */
        struct Defaults {
            static inline const cv::aruco::DetectorParameters arucoDetector;
        };


        //-----------------------------------------------------------------------------
        // Struct members
        //-----------------------------------------------------------------------------

        // Parameters used by cv::aruco::ArucoDetector.
        cv::aruco::DetectorParameters arucoDetector{ Defaults::arucoDetector };
    };

    /**
     * @brief Configuration parameters for refining an ArUco detection.
     */
    struct Refine
    {
    public:
        //-----------------------------------------------------------------------------
        // Enums and structs
        //-----------------------------------------------------------------------------

        /** @brief Default values for all ArUco parameters. */
        struct Defaults {
            static inline const cv::aruco::RefineParameters arucoRefine;
        };


        //-----------------------------------------------------------------------------
        // Struct members
        //-----------------------------------------------------------------------------
        // Parameters used by cv::aruco::ArucoDetector.
        cv::aruco::RefineParameters arucoRefine{ Defaults::arucoRefine };
    };

    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    ~ArucoParameters() = default;


    //-----------------------------------------------------------------------------
    // Class members
    //-----------------------------------------------------------------------------

    /** @brief Parameters used for detecting the ArUco */
    Detection detection;

    /** @brief Parameters used for refining detected corners. */
    Refine refine;

protected:
    ArucoParameters(BoardPattern type);
};
