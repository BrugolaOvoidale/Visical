#include "CvPainter.hpp"
#include "CvImage.hpp"
#include "CvRegion.hpp"
#include "CvContour.hpp"


CvPainter::CvPainter(
    Colour defaultColour,
    int defaultThickness,
    cv::LineTypes defaultLineType)
    : colour_(defaultColour),
    thickness_(defaultThickness),
    lineType_(defaultLineType)
{
}

////////////////////////////////////////

uchar CvPainter::BGRtoGrayScale(const Colour& colour)
{
    return (0.299 * colour.red() + 0.587 * colour.green() + 0.114 * colour.blue());
}

////////////////////////////////////////

// Apply a LUT/colormap to single-channel data
CvPainter& CvPainter::applyLUT(
    CvImage& img,
    cv::ColormapTypes colormapType,
    bool normalize)
{
    cv::Mat grayMat = img.toGray().mat();
    cv::Mat processedMat;

    // Normalize if requested
    if (normalize)
    {
        cv::normalize(grayMat, processedMat, 0, 255, cv::NORM_MINMAX, CV_8U);
    }
    else
    {
        processedMat = grayMat;
    }

    // Apply colormap
    cv::Mat mat;
    cv::applyColorMap(processedMat, mat, colormapType);

    img = CvImage(mat);

    return *this;
}

CvPainter& CvPainter::drawPoint(
    CvImage& img,
    const cv::Point& pt)
{
    cv::Mat mat = img.mat();

    if (pt.x >= 0 && pt.x < mat.cols && pt.y >= 0 && pt.y < mat.rows)
    {
        // Check if the image has at least 3 channels (BGR)
        if (mat.channels() >= 3)
        {
            cv::Vec3b& pixel = mat.at<cv::Vec3b>(pt.y, pt.x);

            pixel[0] = colour_.blue();
            pixel[1] = colour_.green();
            pixel[2] = colour_.red();
        }
        else if (mat.channels() == 1)  // Grayscale image
        {
            mat.at<uchar>(pt.y, pt.x) = BGRtoGrayScale(colour_);
        }
    }

    return *this;
}

CvPainter& CvPainter::drawPoints(
    CvImage& img,
    const std::vector<cv::Point>& pts)
{
    for (const auto& p : pts)
    {
        drawPoint(img, p);
    }

    return *this;
}

CvPainter& CvPainter::drawPoints(
    CvImage& img,
    const std::vector<cv::Point2f>& pts)
{
    for (const auto& p : pts)
    {
        drawPoint(
            img,
            cv::Point(static_cast<int>(std::round(p.x)), static_cast<int>(std::round(p.y)))
        );
    }

    return *this;
}

CvPainter& CvPainter::drawLine(
    CvImage& img,
    const cv::Point& p1,
    const cv::Point& p2)
{
    cv::Mat mat = img.mat();
	const int channels = mat.channels();

	const cv::Scalar colorToUse =
        (channels >= 3) ?
        cv::Scalar(colour_.blue(), colour_.green(), colour_.red()) :
        cv::Scalar(BGRtoGrayScale(colour_));

    cv::line(
        mat, 
        p1, p2,
        colorToUse,
        thickness_,
        lineType_
    );

    return *this;
}

CvPainter& CvPainter::drawCircle(
    CvImage& img,
    const cv::Point& center,
    int radius) 
{
    cv::Mat mat = img.mat();
    const int channels = mat.channels();

    const cv::Scalar colorToUse =
        (channels >= 3) ?
        cv::Scalar(colour_.blue(), colour_.green(), colour_.red()) :
        cv::Scalar(BGRtoGrayScale(colour_));

    cv::circle(
        mat,
        center, radius,
        colorToUse,
        thickness_,
        lineType_
    );

    return *this;
}

CvPainter& CvPainter::drawRectangle(
    CvImage& img,
    const cv::Point& p1,
    const cv::Point& p2)
{
    cv::Mat mat = img.mat();
    const int channels = mat.channels();

    const cv::Scalar colorToUse =
        (channels >= 3) ?
        cv::Scalar(colour_.blue(), colour_.green(), colour_.red()) :
        cv::Scalar(BGRtoGrayScale(colour_));

    cv::rectangle(
        mat,
        p1, p2,
        colorToUse,
        thickness_,
        lineType_
    );

    return *this;
}

CvPainter& CvPainter::drawRegion(
    CvImage& img,
    const CvRegion& region,
    bool filled)
{
    cv::Mat mat = img.mat();
    const int channels = mat.channels();

    const cv::Scalar colorToUse =
        (channels >= 3) ?
        cv::Scalar(colour_.blue(), colour_.green(), colour_.red()) :
        cv::Scalar(BGRtoGrayScale(colour_));

    if (filled)
    {
        cv::fillPoly(
            mat,
            region.points(),
            colorToUse,
            lineType_
        );
    }
    else
    {
        cv::polylines(
            mat,
            region.points(),
            true,
            colorToUse,
            thickness_,
            lineType_);
    }

    return *this;
}

CvPainter& CvPainter::drawRegions(
    CvImage& img,
    const std::vector<CvRegion>& regions,
    bool filled)
{
    cv::Mat mat = img.mat();
    const int channels = mat.channels();

    const cv::Scalar colorToUse =
        (channels >= 3) ?
        cv::Scalar(colour_.blue(), colour_.green(), colour_.red()) :
        cv::Scalar(BGRtoGrayScale(colour_));

    for (const auto& r : regions)
    {
        if (filled)
        {
            cv::fillPoly(
                mat,
                r.points(),
                colorToUse,
                lineType_
            );
        }
        else
        {
            cv::polylines(
                mat,
                r.points(),
                true,
                colorToUse,
                thickness_,
                lineType_
            );
        }
    }

    return *this;
}

CvPainter& CvPainter::drawContour(
    CvImage& img,
    const CvContour& contour,
    bool filled)
{
    cv::Mat mat = img.mat();
    const int channels = mat.channels();

    const cv::Scalar colorToUse =
        (channels >= 3) ?
        cv::Scalar(colour_.blue(), colour_.green(), colour_.red()) :
        cv::Scalar(BGRtoGrayScale(colour_));

    const std::vector<cv::Point2f>& contPts = contour.points();
    std::vector<cv::Point> intPts;
    intPts.reserve(contPts.size());

    for (const auto& p : contPts)
    {
        intPts.emplace_back(
            static_cast<int>(std::round(p.x)),
            static_cast<int>(std::round(p.y))
        );
    }

    if (filled)
    {
        cv::fillPoly(
            mat,
            intPts,
            colorToUse,
            lineType_
        );
    }
    else
    {
        cv::polylines(
            mat,
            intPts,
            true,
            colorToUse,
            thickness_,
            lineType_);
    }

    return *this;
}

CvPainter& CvPainter::drawContours(
    CvImage& img,
    const std::vector<CvContour>& contours,
    bool filled)
{
    cv::Mat mat = img.mat();
    const int channels = mat.channels();

    const cv::Scalar colorToUse =
        (channels >= 3) ?
        cv::Scalar(colour_.blue(), colour_.green(), colour_.red()) :
        cv::Scalar(BGRtoGrayScale(colour_));

    std::vector<cv::Point> intPts;

    for (const auto& c : contours)
    {
        const std::vector<cv::Point2f>& contPts = c.points();

        for (const auto& p : contPts)
        {
            intPts.emplace_back(
                static_cast<int>(std::round(p.x)),
                static_cast<int>(std::round(p.y))
            );
        }
    }

    if (filled)
    {
        cv::fillPoly(
            mat,
            intPts,
            colorToUse,
            lineType_
        );
    }
    else
    {
        cv::polylines(
            mat,
            intPts,
            true,
            colorToUse,
            thickness_,
            lineType_
        );
    }

    return *this;
}

CvPainter& CvPainter::setColour(const Colour& c)
{
    colour_ = c;

    return *this;
}

CvPainter& CvPainter::setThickness(int t)
{
    thickness_ = t;

    return *this;
}

CvPainter& CvPainter::setLineType(cv::LineTypes lt)
{
    lineType_ = lt;

    return *this;
}

CvPainter::Colour CvPainter::getColour() const
{
    return colour_;
}

int CvPainter::getThickness() const
{
    return thickness_;
}

int CvPainter::getLineType() const
{
    return lineType_;
}


////////////////////////////////////////


CvPainter::Colour::Colour(unsigned int blue, unsigned int green, unsigned int red)
{
    set(blue, green, red);
}

////////////////////////////////////////

void CvPainter::Colour::set(unsigned int blue, unsigned int green, unsigned int red)
{
    // B
    if (blue > 255)
        blue_ = (uchar)255;
    else
        blue_ = (uchar)blue;

    // G
    if (green > 255)
        green_ = (uchar)255;
    else
        green_ = (uchar)green;

    // R
    if (red > 255)
        red_ = (uchar)255;
    else
        red_ = (uchar)red;
}

uchar CvPainter::Colour::blue() const
{
    return blue_;
}

uchar CvPainter::Colour::green() const
{
    return green_;
}

uchar CvPainter::Colour::red() const
{
    return red_;
}
