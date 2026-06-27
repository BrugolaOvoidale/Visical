#pragma once
#include <opencv2/core/types.hpp>
#include "../BoardPattern.hpp"


/**
 * @brief Base class for defining calibration pattern.
 * 
 * This class stores the fundamental specification of a pattern
 * used for board detection.
 */
class PatternParameters
{
public:
    //-----------------------------------------------------------------------------
    // Structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Base class for defining calibration pattern geometry parameters.
     *
     * This class stores the fundamental specification of a geometric pattern
     * used for board detection.
     */
    class Geometry
    {
    public:
        //-----------------------------------------------------------------------------
        // Constructor/Destructor
        //-----------------------------------------------------------------------------

        virtual ~Geometry() = default;


        //-----------------------------------------------------------------------------
        // Class operators
        //-----------------------------------------------------------------------------

        /**
         * @brief Returns the grid size of the pattern.
         *
         * For chessboards, this represents the number of internal corners
         * (e.g., a standard 8x8 board has 7x7 internal corners).
         */
        cv::Size patternSize() const;

    protected:
        // Protected constructor for specialized derived classes.
        Geometry(const cv::Size& patternSize);

    private:
        // Inner corners (chessboard) or centers (circles)
        cv::Size patternSize_;
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    virtual ~PatternParameters() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the type of the board pattern. */
    BoardPattern patternType() const;

    /** @brief Returns the base geometrical informations of the board pattern. */
    virtual const Geometry& getGeometry() const = 0;

protected:
    // Protected constructor for specialized derived classes.
    PatternParameters(BoardPattern type);

private:
    // Pattern specification
    BoardPattern type_;
};
