#pragma once
#include <opencv2/imgproc.hpp>


// Forward declarations
class CvImage;
class CvRegion;
class CvContour;


/**
 * @brief A stateful drawing utility for rendering shapes and regions onto images.
 * 
 * CvPainter maintains a persistent state (color, thickness, line type) to simplify
 * multiple drawing operations. It uses a fluent interface, allowing method calls
 * to be chained together.
 */
class CvPainter
{
public:
    //-----------------------------------------------------------------------------
    // Support classes
    //-----------------------------------------------------------------------------

    /** * @brief A BGR color representation.*/
    class Colour
    {
    public:
        Colour() = default;

        Colour(unsigned int blue, unsigned int green, unsigned int red);

        ~Colour() = default;

        void set(unsigned int blue, unsigned int green, unsigned int red);

        uchar blue() const;

        uchar green() const;

        uchar red() const;

    private:
        uchar blue_{ 0 };
        
        uchar green_{ 0 };

        uchar red_{ 0 };
	};


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs a painter with an initial state.
     * @param defaultColour Initial drawing color.
     * @param defaultThickness Initial line thickness.
     * @param defaultLineType Initial line algorithm.
     */
    CvPainter(
        Colour defaultColour = Colour(0 , 255, 0),
        int defaultThickness = 1,
        cv::LineTypes defaultLineType = cv::LineTypes::LINE_AA);


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Static utility to convert a BGR color to its grayscale equivalent.
     * @param colour The source color.
     * @return The grayscale intensity [0-255].
     */
    static uchar BGRtoGrayScale(const Colour& colour);

    /**
     * @brief Applies a pseudocolor lookup table to an image.
     * @param img The target image to modify.
     * @param colormapType The OpenCV colormap to apply.
     * @param normalize If true, the image is normalized to [0, 255] before applying the LUT.
     * @return Reference to this painter for chaining.
     */
    CvPainter& applyLUT(
        CvImage& img,
        cv::ColormapTypes colormapType = cv::ColormapTypes::COLORMAP_JET,
        bool normalize = true
    );

    /** @brief Draws a single pixel. */
    CvPainter& drawPoint(
        CvImage& img,
        const cv::Point& pt
    );

    /** @brief Draws a vector of pixels. */
    CvPainter& drawPoints(
        CvImage& img,
        const std::vector<cv::Point>& pt
    );
    CvPainter& drawPoints(
        CvImage& img,
        const std::vector<cv::Point2f>& pt
    );

    /** @brief Draws a line segment between two points. */
    CvPainter& drawLine(
        CvImage& img,
        const cv::Point& p1,
        const cv::Point& p2);

    /** @brief Draws a circle. */
    CvPainter& drawCircle(
        CvImage& img,
        const cv::Point& center,
        int radius
    );

    /** @brief Draws an upright rectangle. */
    CvPainter& drawRectangle(
        CvImage& img,
        const cv::Point& topLeft,
        const cv::Point& bottomRight
    );

    /**
     * @brief Renders a CvRegion onto the image.
     * @param region The region to draw.
     * @param filled If true, fills the region; if false, draws the boundary.
     */
    CvPainter& drawRegion(
        CvImage& img,
        const CvRegion& region,
        bool filled = false
    );

    /** @brief Renders multiple regions. */
    CvPainter& drawRegions(
        CvImage& img,
        const std::vector<CvRegion>& regions,
        bool filled = false);

    /** @brief Renders a CvContour onto the image. */
    CvPainter& drawContour(
        CvImage& img,
        const CvContour& contour,
        bool filled = false
    );

    /** @brief Renders multiple contours. */
    CvPainter& drawContours(
        CvImage& img,
        const std::vector<CvContour>& contours,
        bool filled = false
    );

    /** @brief Updates the painter's current color. */
    CvPainter& setColour(const Colour& c);

    /** @brief Updates the painter's current thickness. */
    CvPainter& setThickness(int t);

    /** @brief Updates the painter's current line type. */
    CvPainter& setLineType(cv::LineTypes lt);

    Colour getColour() const;

    int getThickness() const;

    int getLineType() const;

private:
    Colour colour_;
    
    int thickness_;

    int lineType_;
};