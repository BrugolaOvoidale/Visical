#pragma once
#include "PatternParameters.hpp"


/**
 * @brief Configuration parameters for detecting chessboard calibration patterns.
 */
class ChessboardParameters : public PatternParameters
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Geometrical configuration parameters for chessboard pattern.
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
         * @param sqSize Physical dimension of a single square in mm.
         */
        Geometry(
            int patternWidth,
            int patternHeight,
            float sqSize
        );
        Geometry(
            const cv::Size& patternSize,
            float sqSize
        );

        ~Geometry() = default;


        //-----------------------------------------------------------------------------
        // Class members
        //-----------------------------------------------------------------------------

        // Physical size in world units (mm)
        float squareSize;
    };

    /**
     * @brief Configuration parameters for detecting chessboard calibration patterns.
     */
    struct Detection
    {
    public:
        //-----------------------------------------------------------------------------
        // Enums and structs
        //-----------------------------------------------------------------------------

        /**
         * @brief Defines the trade-off between detection speed and robustness.
         */
        enum class SearchAccuracy {
            FAST,               ///< Prioritizes speed; may miss patterns in low-quality images.
            BALANCED,           ///< The default balance between performance and detection rate.
            EXHAUSTIVE          ///< Scans more thoroughly; best for noisy or difficult images.
        };

        /** @brief Default values for all chessboard parameters. */
        struct Defaults {
            static constexpr SearchAccuracy searchAccuracy{ SearchAccuracy::BALANCED };
            static constexpr bool adaptiveThreshold{ false };
            static constexpr bool normalizeImage{ false };
            static constexpr bool filterQuads{ false };
            static constexpr bool subpixelAccuracy{ false };
            static constexpr bool allowLargerBoards{ false };
        };


        //-----------------------------------------------------------------------------
        // Struct members
        //-----------------------------------------------------------------------------

        // Detection accuarcy/speed trade-off
        SearchAccuracy searchAccuracy{ Defaults::searchAccuracy };

        // Use adaptive thresholding for preprocessing
        bool adaptiveThreshold{ Defaults::adaptiveThreshold };

        // Normalize image brightness/contrast before detection
        bool normalizeImage{ Defaults::normalizeImage };

        // Filter detected quads based on geometric properties
        bool filterQuads{ Defaults::filterQuads };

        // Refine corner positions to sub-pixel accuarcy
        bool subpixelAccuracy{ Defaults::subpixelAccuracy };

        // Allow detection of large boards that may exceed image size
        bool allowLargerBoards{ Defaults::allowLargerBoards };
    };

    /**
     * @brief Configuration parameters for refining a chessboard detection.
     */
    struct Refine
    {
    public:
        //-----------------------------------------------------------------------------
        // Enums and structs
        //-----------------------------------------------------------------------------

        /**
         * @brief Defines the termination criteria for iterative algorithms.
         */
        enum class TermCriteriaType {
            COUNT,        ///< Use the maximum number of iterations or elements to compute
            EPSILON,      ///< Use the desired accuracy or change in parameters at which the iterative algorithm stops.
            BOTH          ///< Let OpenCV use both (cv::TermCriteria::COUNT + cv::TermCriteria::EPS).
        };

        /** @brief Default values for all chessboard parameters. */
        struct Defaults {
            static constexpr TermCriteriaType type{ TermCriteriaType::BOTH };
            static constexpr int maxCount{ 30 };
            static constexpr double epsilon{ 0.001 };
            static constexpr int windowWidth{ 11 };
            static constexpr int windowHeight{ 11 };
            static constexpr int zeroZoneWidth{ -1 };
            static constexpr int zeroZoneHeight{ -1 };
        };


        //-----------------------------------------------------------------------------
        // Struct members
        //-----------------------------------------------------------------------------

        // The type of termination criteria, see cv::TermCriteria::Type
        TermCriteriaType type{ Defaults::type };

        // The maximum number of iterations or elements to compute.
        int maxCount{ Defaults::maxCount };

        // The desired accuracy or change in parameters at which the iterative algorithm stops.
        double epsilon{ Defaults::epsilon };

        // Half of the side length of the search window
        cv::Size winSize{ Defaults::windowWidth, Defaults::windowHeight };

        // Refine corner positions to sub-pixel accuarcy
        cv::Size zeroZone{ Defaults::zeroZoneWidth, Defaults::zeroZoneHeight };
    };

    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs parameters by providing internal corner counts.
     * @param g Geometrical parameters of the chessboard.
     */
    ChessboardParameters(Geometry g);

    ~ChessboardParameters() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Geometrical parameters of the circleboard. */
    const Geometry& getGeometry() const override;


    //-----------------------------------------------------------------------------
    // Class members
    //-----------------------------------------------------------------------------

    /** @brief Geometrical parameters of the chessboard. */
    Geometry geometry;

    /** @brief Parameters used for detecting the chessboard */
    Detection detection;

    /** @brief Parameters used for refining detected corners. */
    Refine refine;
};

template <>
struct EnumTraits<ChessboardParameters::Detection::SearchAccuracy> {
    static constexpr std::array<std::pair<ChessboardParameters::Detection::SearchAccuracy, const char*>, 3> values{ {
    {ChessboardParameters::Detection::SearchAccuracy::FAST, "fast"},
    {ChessboardParameters::Detection::SearchAccuracy::BALANCED, "balanced"},
    {ChessboardParameters::Detection::SearchAccuracy::EXHAUSTIVE, "exhaustive"},
    } };
};

template <>
struct EnumTraits<ChessboardParameters::Refine::TermCriteriaType> {
    static constexpr std::array<std::pair<ChessboardParameters::Refine::TermCriteriaType, const char*>, 3> values{ {
    {ChessboardParameters::Refine::TermCriteriaType::COUNT, "count"},
    {ChessboardParameters::Refine::TermCriteriaType::EPSILON, "epsilon"},
    {ChessboardParameters::Refine::TermCriteriaType::BOTH, "both"},
    } };
};
