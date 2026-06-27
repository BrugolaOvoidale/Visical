#pragma once
#include "ArucoParameters.hpp"


/**
 * @brief Configuration parameters for detecting ChArUco calibration patterns.
 */
class CharucoParameters : public ArucoParameters
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Geometrical configuration parameters for ChArUco pattern.
     */
    class Geometry : public PatternParameters::Geometry
    {
    public:
        //-----------------------------------------------------------------------------
        // Constructor/Destructor
        //-----------------------------------------------------------------------------

        /**
         * @brief Constructs parameters by a CharucoBoard object.
         * @param charuco Geometrical parameters for a ChArUco board.
         */
        Geometry(cv::aruco::CharucoBoard board);

        ~Geometry() = default;


        //-----------------------------------------------------------------------------
        // Class members
        //-----------------------------------------------------------------------------

        // Planar chessboard where the (ArUco) markers are placed inside the white squares of a chessboard.
        cv::aruco::CharucoBoard charuco;
    };

    /**
     * @brief Configuration parameters for detecting ChArUco calibration patterns.
     */
    struct Detection : public ArucoParameters::Detection
    {
    public:
        //-----------------------------------------------------------------------------
        // Structs
        //-----------------------------------------------------------------------------

        /** @brief Default values for all ChArUco parameters. */
        struct Defaults : public ArucoParameters::Detection::Defaults {
            static inline const cv::aruco::CharucoParameters charuco;
        };


        //-----------------------------------------------------------------------------
        // Struct members
        //-----------------------------------------------------------------------------

        // ChArUco-detection specific parameters.
        cv::aruco::CharucoParameters charuco{ Defaults::charuco };
    };

    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs parameters by providing the geometry of the board.
     * @param g Geometrical parameters of the ChArUco board.
     */
    CharucoParameters(Geometry g);

    ~CharucoParameters() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Geometrical parameters of the ChArUco board. */
    const Geometry& getGeometry() const override;

    //-----------------------------------------------------------------------------
    // Class members
    //-----------------------------------------------------------------------------

    /** @brief Geometrical parameters of the ChArUco board. */
    Geometry geometry;

    /** @brief Parameters used for detecting the ChArUco board */
    Detection detection;
};


struct CharucoDictionaries {};

template <>
struct EnumTraits<CharucoDictionaries> {
    using ValueType = cv::aruco::PredefinedDictionaryType;

    static constexpr std::array<std::pair<cv::aruco::PredefinedDictionaryType, const char*>, 16> values{ {
    {cv::aruco::PredefinedDictionaryType::DICT_4X4_50, "DICT_4X4_50"},
    {cv::aruco::PredefinedDictionaryType::DICT_4X4_100, "DICT_4X4_100"},
    {cv::aruco::PredefinedDictionaryType::DICT_4X4_250, "DICT_4X4_250"},
    {cv::aruco::PredefinedDictionaryType::DICT_4X4_1000, "DICT_4X4_1000"},
    {cv::aruco::PredefinedDictionaryType::DICT_5X5_50, "DICT_5X5_50"},
    {cv::aruco::PredefinedDictionaryType::DICT_5X5_100, "DICT_5X5_100"},
    {cv::aruco::PredefinedDictionaryType::DICT_5X5_250, "DICT_5X5_250"},
    {cv::aruco::PredefinedDictionaryType::DICT_5X5_1000, "DICT_5X5_1000"},
    {cv::aruco::PredefinedDictionaryType::DICT_6X6_50, "DICT_6X6_50"},
    {cv::aruco::PredefinedDictionaryType::DICT_6X6_100, "DICT_6X6_100"},
    {cv::aruco::PredefinedDictionaryType::DICT_6X6_250, "DICT_6X6_250"},
    {cv::aruco::PredefinedDictionaryType::DICT_6X6_1000, "DICT_6X6_1000"},
    {cv::aruco::PredefinedDictionaryType::DICT_7X7_50, "DICT_7X7_50"},
    {cv::aruco::PredefinedDictionaryType::DICT_7X7_100, "DICT_7X7_100"},
    {cv::aruco::PredefinedDictionaryType::DICT_7X7_250, "DICT_7X7_250"},
    {cv::aruco::PredefinedDictionaryType::DICT_7X7_1000, "DICT_7X7_1000"},
    } };
};


struct CharucoCornerRefineMethod {};

template <>
struct EnumTraits<CharucoCornerRefineMethod> {
    using ValueType = cv::aruco::CornerRefineMethod;

    static constexpr std::array<std::pair<cv::aruco::CornerRefineMethod, const char*>, 3> values{ {
    {cv::aruco::CornerRefineMethod::CORNER_REFINE_NONE, "CORNER_REFINE_NONE"},
    {cv::aruco::CornerRefineMethod::CORNER_REFINE_SUBPIX, "CORNER_REFINE_SUBPIX"},
    {cv::aruco::CornerRefineMethod::CORNER_REFINE_CONTOUR, "CORNER_REFINE_CONTOUR"},
    } };
};