#pragma once
#include "ArucoParameters.hpp"


/**
 * @brief Configuration parameters for detecting AprilTag calibration patterns.
 */
class AprilTagParameters : public ArucoParameters
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Geometrical configuration parameters for AprilTag pattern.
     */
    class Geometry : public PatternParameters::Geometry
    {
    public:
        //-----------------------------------------------------------------------------
        // Constructor/Destructor
        //-----------------------------------------------------------------------------

        /**
         * @brief Constructs parameters by a GridBoard object.
         * @param charuco Geometrical parameters for a AprilTag board.
         */
        Geometry(
            cv::aruco::GridBoard board,
            cv::aruco::PredefinedDictionaryType dict
        );

        ~Geometry() = default;


        //-----------------------------------------------------------------------------
        // Class operators
        //-----------------------------------------------------------------------------

        cv::aruco::PredefinedDictionaryType dictionary() const;


        //-----------------------------------------------------------------------------
        // Class members
        //-----------------------------------------------------------------------------

        // Planar board with grid arrangement of markers
        cv::aruco::GridBoard aprilTag;

    private:
        // Dictionary that defines this board
        cv::aruco::PredefinedDictionaryType dict_;
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs parameters by providing providing the geometry of the board.
     * @param g Geometrical parameters of the AprilTag.
     */
    AprilTagParameters(Geometry g);

    ~AprilTagParameters() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Geometrical parameters of the AprilTag. */
    const Geometry& getGeometry() const override;

    //-----------------------------------------------------------------------------
    // Class members
    //-----------------------------------------------------------------------------

    /** @brief Geometrical parameters of the AprilTag. */
    Geometry geometry;
};


struct AprilTagDictionaries {};

template <>
struct EnumTraits<AprilTagDictionaries> {
    using ValueType = cv::aruco::PredefinedDictionaryType;

    static constexpr std::array<std::pair<cv::aruco::PredefinedDictionaryType, const char*>, 5> values{ {
    {cv::aruco::PredefinedDictionaryType::DICT_APRILTAG_16h5, "DICT_APRILTAG_16h5"},
    {cv::aruco::PredefinedDictionaryType::DICT_APRILTAG_25h9, "DICT_APRILTAG_25h9"},
    {cv::aruco::PredefinedDictionaryType::DICT_APRILTAG_36h10, "DICT_APRILTAG_36h10"},
    {cv::aruco::PredefinedDictionaryType::DICT_APRILTAG_36h11, "DICT_APRILTAG_36h11"},
    {cv::aruco::PredefinedDictionaryType::DICT_ARUCO_MIP_36h12, "DICT_ARUCO_MIP_36h12"}
    } };
};


struct AprilTagCornerRefineMethod {};

template <>
struct EnumTraits<AprilTagCornerRefineMethod> {
    using ValueType = cv::aruco::CornerRefineMethod;

    static constexpr std::array<std::pair<cv::aruco::CornerRefineMethod, const char*>, 4> values{ {
    {cv::aruco::CornerRefineMethod::CORNER_REFINE_NONE, "CORNER_REFINE_NONE"},
    {cv::aruco::CornerRefineMethod::CORNER_REFINE_SUBPIX, "CORNER_REFINE_SUBPIX"},
    {cv::aruco::CornerRefineMethod::CORNER_REFINE_CONTOUR, "CORNER_REFINE_CONTOUR"},
    {cv::aruco::CornerRefineMethod::CORNER_REFINE_APRILTAG, "CORNER_REFINE_APRILTAG"},
    } };
};