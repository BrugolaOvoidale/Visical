#pragma once
#include <opencv2/core/types.hpp>
#include "../BoardPattern.hpp"


/**
 * @brief Base class for defining calibration pattern detection parameters.
 * 
 * This class stores the fundamental specification of a geometric pattern
 * used for board detection.
 */
class PatternParams
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    virtual ~PatternParams() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the type of the board pattern. */
    BoardPattern patternType() const;

    /**
     * @brief Returns the grid size of the pattern.
     * 
     * For chessboards, this represents the number of **internal corners** * (e.g., a standard 8x8 board has 7x7 internal corners).
     */
    cv::Size patternSize() const;

protected:
    // Protected constructor for specialized derived classes.
    PatternParams(
        BoardPattern type,
        const cv::Size& patternSize
    );

private:
    // Pattern specification
    BoardPattern type_;

    // Inner corners (chessboard) or centers (circles)
    cv::Size patternSize_;
};
