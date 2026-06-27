#pragma once
#include <enum/EnumTraits.hpp>


/** * @brief Supported calibration board topologies. */
enum class BoardPattern {
    CHESSBOARD,             ///< Standard square grid.
    SYMMETRIC_CIRCLES,      ///< Grid of circles aligned in straight rows and columns.
    ASYMMETRIC_CIRCLES,     ///< Grid of circles where rows are offset (staggered).
    CHARUCO,                ///< ArUco markers placed inside chessboard squares.
    APRIL_TAG               ///< Grid of AprilTag aligned in straight rows and columns.
};

/** @brief String representation mapping for BoardPattern. */
template <>
struct EnumTraits<BoardPattern> {
    static constexpr std::array<std::pair<BoardPattern, const char*>, 5> values{ {
    {BoardPattern::CHESSBOARD, "chessboard"},
    {BoardPattern::SYMMETRIC_CIRCLES, "symmetric_circles"},
    {BoardPattern::ASYMMETRIC_CIRCLES, "asymmetric_circles"},
    {BoardPattern::CHARUCO, "charuco"},
    {BoardPattern::APRIL_TAG, "april_tag"},
    } };
};
