#pragma once
#include "PatternParams.hpp"


/**
 * @brief Configuration parameters for detecting chessboard-style calibration patterns.
 */
class ChessboardParams : public PatternParams
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
        static constexpr SearchAccuracy  searchAccuracy{ SearchAccuracy::BALANCED };
        static constexpr bool adaptiveThreshold{ false };
        static constexpr bool normalizeImage{ false };
        static constexpr bool filterQuads{ false };
        static constexpr bool subpixelAccuracy{ false };
        static constexpr bool allowLargerBoards{ false };
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs parameters by providing internal corner counts.
     * @param patternWidth Number of inner corners along the width.
     * @param patternHeight Number of inner corners along the height.
     * @param squareSize Physical dimension of a single square in mm.
     */
    ChessboardParams(
        int patternWidth,
        int patternHeight,
        float squareSize
    );

    /** @overload */
    ChessboardParams(
        const cv::Size& patternSize,
        float squareSize
    );

    ~ChessboardParams() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the physical size of a square side. */
    float squareSize() const;

public:
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

private:
    // Physical size in world units (mm)
    float squareSize_;
};


template <>
struct EnumTraits<ChessboardParams::SearchAccuracy> {
    static constexpr std::array<std::pair<ChessboardParams::SearchAccuracy, const char*>, 3> values{ {
    {ChessboardParams::SearchAccuracy::FAST, "fast"},
    {ChessboardParams::SearchAccuracy::BALANCED, "balanced"},
    {ChessboardParams::SearchAccuracy::EXHAUSTIVE, "exhaustive"},
    } };
};