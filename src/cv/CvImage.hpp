#pragma once
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <chrono>
#include "CvRegion.hpp"


/**
 * @brief High-level wrapper for OpenCV Mat with integrated metadata and analysis tools.
 *
 * This class handles image data (cv::Mat) alongside acquire timestamps. It provides
 * a suite of operators for image arithmetic, geometric transformations, and
 * analysis over specific CvRegion areas.
 */
class CvImage
{
public:
    //-----------------------------------------------------------------------------
    // Enums and structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Defines how contour boundaries are interpreted when extracted
     *        from the region mask.
     */
    enum class FlipAxis {
        X,
        Y,
        XY
    };

    /**
     * @brief Human-readable breakdown of the image acquire time.
     */
    struct Timestamp
    {
        int milliSecond;
        int second;
        int minute;
        int hour;
        int day;
        int month;
        int year;
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /** @brief Default constructor. Internal mask is empty. */
    CvImage();

    /** 
     * @brief Construct from an existing CvImage.
     * 
     * Copy other metadata 
     */
    CvImage(const CvImage& other);

    /**
     * @brief Move constructor from an existing CvImage.
     * 
     * Copy other metadata 
     */
    CvImage(CvImage&& other) noexcept;

    /**
     * @brief Construct from an existing cv::Mat. */
    CvImage(const cv::Mat& mat);

    /** @brief Load an image from a file path. */
    CvImage(const std::string& filePath);

    /** @brief Returns a deep copy of the image and its metadata. */
    CvImage clone() const;

    ~CvImage() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------

    /**
     * @brief Computes the distance from each pixel to the nearest region boundary.
     * @param region The source region for distance calculation.
     * @param imageWidth Output image width.
     * @param imageHeight Output image height.
     * @param foreground If true, computes distance within the region; otherwise, outside it.
     * @param distanceType Type of distance.
     * @param maskSize Size of the distance transform mask.
     * @return A CV_32F image where pixel values represent distances.
     */
    static CvImage distanceTransform(
        const CvRegion& region,
        int imageWidth,
        int imageHeight,
        bool foreground = false,
        cv::DistanceTypes distanceType = cv::DIST_L2,
        int maskSize = 3
    );


    //-----------------------------------------------------------------------------
    // Logical operators
    //-----------------------------------------------------------------------------

    CvImage& operator=(const CvImage& other);
    CvImage operator+(const CvImage& other) const;
    CvImage operator-(const CvImage& other) const;
    CvImage operator+(double value) const;
    CvImage operator-(double value) const;
    CvImage operator*(double factor) const;
    CvImage operator/(double factor) const;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the number of columns. */
    int width() const;

    /** @brief Returns the number of rows. */
    int height() const;

    /** @brief Returns the image dimensions as a cv::Size. */
    cv::Size size() const;

    /** @brief Returns the number of image channels. */
    int channels() const;

    /**
     * @brief Timestamp semantics for CvImage.
     *
     * The timestamp represents the time at which the image content was acquired
     * (i.e., when the underlying scene was captured or the data was generated),
     * NOT when this CvImage object was created or processed.
     *
     * This distinction is important because an image can undergo many transformations
     * (e.g., binarization, rotation, scaling, rectification) without changing the
     * moment in time that the original scene represents.
     *
     * Therefore:
     * - Creating a CvImage from new data (e.g., camera capture, file read, synthetic image)
     *   assigns a new timestamp.
     * - Copying or cloning a CvImage preserves the timestamp, since the image content
     *   represents the same scene.
     * - Image processing operations do NOT modify the timestamp, as they only transform
     *   the representation of the same underlying data.
     *
     * In short:
     *   If two CvImage instances represent the same visual content (even if transformed),
     *   they share the same timestamp.
     */
    Timestamp timestamp() const;

    /** @brief Returns the timestamp as a system clock time point. See timestamp() for more details.*/
    std::chrono::system_clock::time_point timepoint() const;

    /**
     * @brief Rotates the image around its center.
     * @param angle Rotation angle in degrees (clockwise).
     * @param interpolation Interpolation method.
     * @param borderType Pixel extrapolation method.
     * @param borderValue Value used for pixels outside the original image.
     * @return A CvImage.
     */
    CvImage rotate(
        float angle,
        cv::InterpolationFlags interpolation = cv::InterpolationFlags::INTER_NEAREST_EXACT,
        cv::BorderTypes borderType = cv::BorderTypes::BORDER_CONSTANT,
        const cv::Scalar& borderValue = cv::Scalar(0)
    ) const;

    /**
     * @brief Flips the image.
     * @param axis FlipAxis X, Y or XY.
     * @return A CvImage.
     */
    CvImage mirror(FlipAxis axis) const;

    /**
    * @brief Performs a simple range thresholding.
    * @return A CvRegion.
    */
    CvRegion threshold(
        double minVal,
        double maxVal
    ) const;

    /**
     * @brief Convert to BGR format. Important note, it assumes the input could
     * only be in BGR or Gray-scale format.
     * @return A CvImage.
     */
    CvImage toBGR() const;

    /**
     * @brief Convert to gray format. Important note, it assumes the input could
     * only be in BGR or Gray-scale format.
     * @return A CvImage.
     */
    CvImage toGray() const;

    /**
     * @brief Crop the image as the ROI size.
     * @return A CvImage.
     */
    CvImage crop(const cv::Rect& roi) const;

    /**
     * @brief Masks the image using the bounding box of a region.
     * @param region The region defining the new domain.
     */
    CvImage reduceDomain(const CvRegion& region) const;

    /**
    * @brief Computes min, max, and range within a region, ignoring outliers by percentage.
    * @param region The area of interest.
    * @param percent Percentage of pixels to ignore at the extremes (0-100).
    * @param[out] minVals Vector of minimum values per channel.
    * @param[out] maxVals Vector of maximum values per channel.
    * @param[out] ranges Vector of (max - min) per channel.
    */
    void minMaxGray(
        const CvRegion& region,
        double percent,
        std::vector<double>* minVals,
        std::vector<double>* maxVals,
        std::vector<double>* ranges
    ) const;

    /**
    * @brief Computes mean and standard deviation for each connected component in a region.
    * @param region Input region (can contain multiple components).
    * @param[out] meanValues List of mean values per component.
    * @param[out] deviation Optional pointer to store deviations per component.
    */
    void meanStdDev(
        const CvRegion& region,
        std::vector<double>* meanValues,
        std::vector<double>* deviations
    ) const;

    /**
    * @brief Computes image gradients using a Gaussian derivative.
    * @param sigma Standard deviation for the Gaussian kernel.
    */
    CvImage derivateGauss(double sigma) const;

    /** @brief Scales pixel values to the 0-255 range and converts to CV_8UC1. */
    CvImage scaleImageMax() const;

    /** @brief Performs a bitwise inversion of the image. */
    CvImage bitwiseNot() const;

    /** @brief Loads image data from disk into the current object. */
    void read(const std::string& path);

    /** @brief Writes the current image to disk. @return True on success. */
    bool write(const std::string& path) const;

    /** @brief Returns the internal cv::Mat. */
    const cv::Mat& mat() const;

private:
    // Internal factory to create a new object from operations.
    static CvImage fromOperation(
        const CvImage* src,
        const cv::Mat& mat
    );

    // Internal helper to compute stats via histogram methods.
    void minMaxGrayHistogram(
        const CvRegion& region,
        double percent,
        double& minVal,
        double& maxVal,
        double& range
    ) const;

    // Internal helper to compute stats via sorting/rank methods.
    void minMaxGrayRank(
        const CvRegion& region,
        double percent,
        double& minVal,
        double& maxVal,
        double& range
    ) const;

private:
    // Image data
    cv::Mat mat_;

    // False means 'mat_' was never assigned and by consequence 'timestamp_' is not valid yet
    bool hasValidMat_{ false };

    // Global offset of this image's (0,0)
    cv::Point origin_ = { 0, 0 };

    // Creation time
    std::chrono::system_clock::time_point timestamp_;
};