#include "CvImage.hpp"
#include <execution>
#include <opencv2/imgcodecs.hpp>


CvImage::CvImage()
{
}

CvImage::CvImage(const CvImage& other)
    : mat_(other.mat_),
    timestamp_(other.timestamp_),
    hasValidMat_(other.hasValidMat_),
    origin_(other.origin_)
{
}

CvImage::CvImage(CvImage&& other) noexcept
    : mat_(std::move(other.mat_)),
    timestamp_(other.timestamp_),
    hasValidMat_(other.hasValidMat_),
    origin_(other.origin_)
{
    other.hasValidMat_ = false;
}

CvImage::CvImage(const cv::Mat& mat)
{
    mat_ = mat;
    timestamp_ = std::chrono::system_clock::now();
    hasValidMat_ = true;
}

CvImage::CvImage(const std::string& filePath)
{
	read(filePath);
}

CvImage CvImage::clone() const
{
    CvImage cloned;

    cloned.mat_ = mat_.clone();
    cloned.timestamp_ = timestamp_;
    cloned.hasValidMat_ = hasValidMat_;
    cloned.origin_ = origin_;

    return cloned;
}

/////////////////////////////////////////////////////////////////////////////

CvImage CvImage::distanceTransform(
    const CvRegion& region,
    int imageWidth,
    int imageHeight,
    bool foreground,
    cv::DistanceTypes distanceType,
    int maskSize)
{
    cv::Size sz(imageWidth, imageHeight);

    // Let the region materialize itself in image space
    cv::Mat mask = region.toMask(sz);

    // cv::distanceTransform computes the distance of each pixel to the nearest ZERO pixel.
    //
    // foreground = false:
    //   We want distances for background pixels to the nearest foreground pixel.
    //   -> invert: foreground becomes 0 (the targets), background becomes non-zero.
    //
    // foreground = true:
    //   We want distances for foreground pixels to the nearest background pixel (i.e. the region border).
    //   -> do NOT invert: background is already 0 (the targets), foreground is non-zero.
    cv::Mat input = foreground ? mask : ~mask;

    // See cv::distanceTransform for maskSize
    cv::Mat distf;
    cv::distanceTransform(input, distf, distanceType, maskSize);

    return CvImage(distf);
}

/////////////////////////////////////////////////////////////////////////////

CvImage& CvImage::operator=(const CvImage& other)
{
    if (this == &other)
        return *this;

    mat_ = other.mat_;

    timestamp_ = other.timestamp_;

    hasValidMat_ = other.hasValidMat_;

    return *this;
}

CvImage CvImage::operator+(const CvImage& other) const
{
    cv::Mat result;

    cv::add(mat_, other.mat_, result);

    return CvImage(result);
}

CvImage CvImage::operator-(const CvImage& other) const
{
    cv::Mat result;

    cv::subtract(mat_, other.mat_, result);

    return CvImage(result);
}

CvImage CvImage::operator+(double value) const
{
    cv::Mat result;

    cv::add(mat_, value, result);

    return CvImage(result);
}

CvImage CvImage::operator-(double value) const
{
    cv::Mat result;

    cv::subtract(mat_, value, result);

    return CvImage(result);
}

CvImage CvImage::operator*(double factor) const
{
    cv::Mat result;

    mat_.convertTo(result, mat_.type(), factor);

    return CvImage(result);
}

CvImage CvImage::operator/(double factor) const
{
    cv::Mat result;

    mat_.convertTo(result, mat_.type(), 1.0 / factor);

    return CvImage(result);
}

/////////////////////////////////////////////////////////////////////////////

int CvImage::width() const
{
    return mat_.cols;
}

int CvImage::height() const
{
    return mat_.rows;
}

cv::Size CvImage::size() const
{
    return mat_.size();
}

int CvImage::channels() const
{
    return mat_.channels();
}

CvImage::Timestamp CvImage::timestamp() const
{
    CvImage::Timestamp imageTimestamp;

    // Get milliseconds
    auto duration = timestamp_.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() % 1000;

    // Get std::tm
    std::time_t timeT = std::chrono::system_clock::to_time_t(timestamp_);
    std::tm* tm = std::localtime(&timeT);

    imageTimestamp.milliSecond = static_cast<int>(millis);
    imageTimestamp.second = tm->tm_sec;
    imageTimestamp.minute = tm->tm_min;
    imageTimestamp.hour = tm->tm_hour;
    imageTimestamp.day = tm->tm_mday;
    imageTimestamp.month = tm->tm_mon + 1; // tm_mon is 0-11
    imageTimestamp.year = tm->tm_year + 1900; // tm_year is years since 1900

    return imageTimestamp;
}

std::chrono::system_clock::time_point CvImage::timepoint() const
{
    return timestamp_;
}

CvImage CvImage::rotate(
    float angle,
    cv::InterpolationFlags interpolation,
    cv::BorderTypes borderType,
    const cv::Scalar& borderValue) const
{
    // Compute rotation matrix around center
    cv::Point2f center(mat_.cols / 2.0f, mat_.rows / 2.0f);
    cv::Mat rot_mat = cv::getRotationMatrix2D(center, angle, 1.0);

    // Compute bounding box to prevent cropping
    cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), mat_.size(), angle).boundingRect2f();

    // Adjust transformation matrix
    rot_mat.at<double>(0, 2) += bbox.width / 2.0 - mat_.cols / 2.0;
    rot_mat.at<double>(1, 2) += bbox.height / 2.0 - mat_.rows / 2.0;

    cv::Mat rotated;
    cv::warpAffine(mat_, rotated, rot_mat, bbox.size(), interpolation, borderType, borderValue);

    return fromOperation(this, rotated);
}

CvImage CvImage::mirror(FlipAxis axis) const
{
    int flipCode = 0;

    switch (axis)
    {
        case FlipAxis::Y:
            flipCode = 0; // vertical
	        break;

        case FlipAxis::X:
		    flipCode = 1; // horizontal
            break;

	    case FlipAxis::XY:
            flipCode = -1; // both
            break;

        default:
		    throw std::invalid_argument("Unsupported mirror axis");
    }

    cv::Mat flipped;
    cv::flip(mat_, flipped, flipCode);

    return fromOperation(this, flipped);
}

CvRegion CvImage::threshold(
    double minVal,
    double maxVal) const
{
    cv::Mat binary;

    cv::threshold(mat_, binary, minVal, maxVal, cv::ThresholdTypes::THRESH_BINARY);

    return CvRegion(binary, cv::Rect(origin_, mat_.size()));
}

CvImage CvImage::toBGR() const
{
    const int ch = channels();

    if (ch == 3)
        return fromOperation(this, mat_.clone());

    cv::Mat bgrMat;

    if (ch == 1)
        cv::cvtColor(mat_, bgrMat, cv::COLOR_GRAY2BGR);
    else if (ch == 4)
        cv::cvtColor(mat_, bgrMat, cv::COLOR_BGRA2BGR);
    else
        throw std::runtime_error("Unsupported channel count: " + std::to_string(ch));

    return fromOperation(this, bgrMat);
}

CvImage CvImage::toGray() const
{
    const int ch = channels();

    if (ch == 1)
        return fromOperation(this, mat_.clone());

    cv::Mat bgrMat;

    if (ch == 3)
        cv::cvtColor(mat_, bgrMat, cv::COLOR_BGR2GRAY);
    else if (ch == 4)
        cv::cvtColor(mat_, bgrMat, cv::COLOR_BGRA2GRAY);
    else
        throw std::runtime_error("Unsupported channel count: " + std::to_string(ch));

    return fromOperation(this, bgrMat);
}

CvImage CvImage::crop(const cv::Rect& roi) const
{
    // Clamp in global coordinates
    cv::Rect clamped = roi & cv::Rect(origin_, mat_.size());

    // Translate to local coordinates for mat_ indexing
    cv::Rect local(
        clamped.x - origin_.x,
        clamped.y - origin_.y,
        clamped.width,
        clamped.height);

    CvImage result = fromOperation(this, mat_(local).clone());

    result.origin_ = clamped.tl();  // override: origin is the clamped global position
    return result;
}

CvImage CvImage::reduceDomain(const CvRegion& region) const
{
    const cv::Rect& regBB = region.boundingBox();
    // 1. crop image to region bbox
    cv::Mat cropped = mat_(regBB);

    // 2. apply local mask (already aligned to bbox)
    cv::Mat masked;
    cropped.copyTo(masked, region.mask());

    CvImage result = fromOperation(this, masked);
    result.origin_ = origin_ + regBB.tl();  // carry global offset
    return result;
}

void CvImage::minMaxGray(
    const CvRegion& region,
    double percent,
    std::vector<double>* minVals,
    std::vector<double>* maxVals,
    std::vector<double>* ranges) const
{
    CV_Assert(mat_.channels() == 1);

    const std::vector<CvRegion>& comps = region.connectedComponents();
    size_t n = comps.size();

    if (minVals)
    {
        minVals->assign(n, 0);
    }
    if (maxVals)
    {
        maxVals->assign(n, 0);
    }
    if (ranges)
    {
        ranges->assign(n, 0);
    }


    auto minMaxGrayTask = [&](int i)
        {
            double minV, maxV, rangeV;

            switch (mat_.depth())
            {
            case CV_8U:
                minMaxGrayHistogram(comps[i], percent, minV, maxV, rangeV);
                break;

            case CV_16U:
            case CV_16S:
            case CV_32S:
            case CV_32F:
            case CV_64F:
                minMaxGrayRank(comps[i], percent, minV, maxV, rangeV);
                break;

            default:
                CV_Error(cv::Error::StsUnsupportedFormat,
                    "Unsupported image type");
            }

            if (minVals) (*minVals)[i] = minV;
            if (maxVals) (*maxVals)[i] = maxV;
            if (ranges) (*ranges)[i] = rangeV;
        };

    // Parallel processing per component
    if (n > 3)
    {
        std::vector<int> indices(n);
        std::iota(indices.begin(), indices.end(), 0);

        std::for_each(std::execution::par, indices.begin(), indices.end(), minMaxGrayTask);
    }
    else
    {
        for (size_t i = 0 ; i < n ; ++i)
        {
            minMaxGrayTask(i);
        }
    }
}

void CvImage::meanStdDev(
    const CvRegion& region,
    std::vector<double>* meanValues,
    std::vector<double>* deviation) const
{
    const std::vector<CvRegion>& connectedregions = region.connectedComponents();

    if (meanValues) meanValues->clear();
    if (deviation) deviation->clear();

    for (const auto& r : connectedregions)
    {
        // Extract ROI from image
        cv::Mat roi = mat_(r.boundingBox());

        cv::Scalar meanS, stdS;

        // Use LOCAL mask (already bbox-aligned)
        cv::meanStdDev(roi, meanS, stdS, r.mask());

        if (meanValues)
            meanValues->push_back(meanS[0]);

        if (deviation)
            deviation->push_back(stdS[0]);
    }
}

CvImage CvImage::derivateGauss(double sigma) const
{
    // Build the 1D Gaussian derivative kernel analytically
    // G'(x) = -x/σ² * exp(-x²/2σ²)
    int halfSize = std::max(1, static_cast<int>(std::ceil(3.0 * sigma)));
    int ksize = 2 * halfSize + 1;

    // 1D Gaussian kernel (for smoothing direction)
    cv::Mat gaussKernel(1, ksize, CV_64F);
    // 1D Gaussian derivative kernel (for derivative direction)  
    cv::Mat gaussDerivKernel(1, ksize, CV_64F);

    double sum = 0.0;
    for (int i = 0; i < ksize; ++i)
    {
        double x = i - halfSize;
        double g = std::exp(-(x * x) / (2.0 * sigma * sigma));
        gaussKernel.at<double>(0, i) = g;
        gaussDerivKernel.at<double>(0, i) = -x / (sigma * sigma) * g;
        sum += g;
    }
    // Normalize Gaussian kernel
    gaussKernel /= sum;
    // Note: derivative kernel does NOT need normalization by sum
    // (it integrates to zero by definition)

    cv::Mat src;
    mat_.convertTo(src, CV_64F);

    // Compute dG/dx * I: derive in X, smooth in Y
    cv::Mat gradX, gradY;
    cv::sepFilter2D(src, gradX, CV_64F,
        gaussDerivKernel,       // horizontal: derivative
        gaussKernel.t());       // vertical: smoothing

    // Compute dG/dy * I: smooth in X, derive in Y
    cv::sepFilter2D(src, gradY, CV_64F,
        gaussKernel,            // horizontal: smoothing
        gaussDerivKernel.t());  // vertical: derivative

    cv::Mat grad;
    cv::magnitude(gradX, gradY, grad);

    return fromOperation(this, grad);
}

CvImage CvImage::scaleImageMax() const
{
    cv::Mat src = mat_;
    if (src.empty())
        return CvImage();

    cv::Mat srcFloat;
    if (src.depth() == CV_32F || src.depth() == CV_64F)
    {
        srcFloat = src;
    }
    else
    {
        src.convertTo(srcFloat, CV_32F);
    }

    cv::Mat flat;
    if (srcFloat.channels() > 1)
    {
        flat = srcFloat.reshape(1);
    }
    else
    {
        flat = srcFloat;
	}

    double minv, maxv;
    cv::minMaxLoc(flat, &minv, &maxv);
    if (maxv <= minv)
    {
        // constant image -> map to zeros
        cv::Mat out = cv::Mat::zeros(src.size(), CV_8UC1);
        return fromOperation(this, out);
    }

    cv::Mat scaled;
    srcFloat = (srcFloat - static_cast<float>(minv)) / static_cast<float>(maxv - minv); // 0..1
    srcFloat = srcFloat * 255.0f;
    srcFloat.convertTo(scaled, CV_8U);

    // If multi-channel, collapse to CV_8UC1
    if (scaled.channels() > 1)
        cv::cvtColor(scaled, scaled, cv::COLOR_BGR2GRAY);

    return fromOperation(this, scaled);
}

CvImage CvImage::bitwiseNot() const
{
    cv::Mat result;

    cv::bitwise_not(mat_, result);

    return fromOperation(this, result);
}

void CvImage::read(const std::string& path)
{
    cv::Mat loaded = cv::imread(path, cv::ImreadModes::IMREAD_UNCHANGED);
    if (loaded.data)
    {
        mat_ = loaded;

        timestamp_ = std::chrono::system_clock::now();
        hasValidMat_ = true;
    }
}

bool CvImage::write(const std::string& path) const
{
    return cv::imwrite(path, mat_);
}

const cv::Mat& CvImage::mat() const
{
    return mat_;
}

///////////////////////////////////////////////////////////

CvImage CvImage::fromOperation(
    const CvImage* src,
    const cv::Mat& mat)
{
    CvImage img;

    img.mat_ = mat;
    img.timestamp_ = src->timestamp_;
    img.hasValidMat_ = true;
    img.origin_ = src->origin_;

    return img;
}

void CvImage::minMaxGrayHistogram(
    const CvRegion& region,
    double percent,
    double& minVal,
    double& maxVal,
    double& range) const
{
    const cv::Rect& box = region.boundingBox();
    const cv::Mat& mask = region.mask();


    // Histogram on stack
    int hist[256];
    memset(hist, 0, sizeof(hist));

    int total = 0;


    // Build histogram only inside bounding box

    for (int y = box.y; y < box.y + box.height; y++)
    {
        const uchar* imgRow = mat_.ptr<uchar>(y) + box.x;
        const uchar* maskRow = mask.ptr<uchar>(y - box.y); // local row, no x offset
        for (int x = 0; x < box.width; x++)
        {
            if (maskRow[x])
            {
                hist[imgRow[x]]++;
                total++;
            }
        }
    }

    if (total == 0)
    {
        minVal = maxVal = range = 0;
        return;
    }


    // percent == 0 -> direct min/max

    if (percent == 0.0)
    {
        int minI = 0;
        int maxI = 255;

        while (hist[minI] == 0) minI++;
        while (hist[maxI] == 0) maxI--;

        minVal = minI;
        maxVal = maxI;
        range = maxI - minI;

        return;
    }


    // percent == 50 -> median

    if (percent == 50.0)
    {
        int target = total / 2;

        int cum = 0;

        for (int i = 0; i < 256; i++)
        {
            cum += hist[i];

            if (cum >= target)
            {
                minVal = maxVal = i;
                range = 0;
                return;
            }
        }
    }


    // General percentile trimming

    int trim = int(total * percent / 100.0);

    if (trim > total / 2)
        trim = total / 2;


    // Min percentile

    int cum = 0;
    int minI = 0;

    for (int i = 0; i < 256; i++)
    {
        cum += hist[i];

        if (cum > trim)
        {
            minI = i;
            break;
        }
    }


    // Max percentile

    cum = 0;
    int maxI = 255;

    for (int i = 255; i >= 0; i--)
    {
        cum += hist[i];

        if (cum > trim)
        {
            maxI = i;
            break;
        }
    }

    minVal = minI;
    maxVal = maxI;
    range = maxI - minI;
}

void CvImage::minMaxGrayRank(
    const CvRegion& region,
    double percent,
    double& minVal,
    double& maxVal,
    double& range) const
{
    const cv::Rect& box = region.boundingBox();
    const cv::Mat& mask = region.mask();

    std::vector<double> values;
    values.reserve(box.area());

    // Collect values

    for (int y = box.y; y < box.y + box.height; y++)
    {
        const uchar* maskRow = mask.ptr<uchar>(y - box.y);

        for (int x = 0; x < box.width; x++)
        {
            if (!maskRow[x])
                continue;

            double v;

            switch (mat_.depth())
            {
            case CV_32F:
                v = mat_.ptr<float>(y)[x + box.x];
                break;

            case CV_64F:
                v = mat_.ptr<double>(y)[x + box.x];
                break;

            case CV_16U:
                v = mat_.ptr<uint16_t>(y)[x + box.x];
                break;

            case CV_16S:
                v = mat_.ptr<int16_t>(y)[x + box.x];
                break;

            case CV_32S:
                v = mat_.ptr<int32_t>(y)[x + box.x];
                break;

            default:
                CV_Error(cv::Error::StsUnsupportedFormat,
                    "Unsupported type");
            }

            values.push_back(v);
        }
    }

    if (values.empty())
    {
        minVal = maxVal = range = 0;
        return;
    }


    // Percent = 0 -> direct min/max

    if (percent == 0)
    {
        auto mm = std::minmax_element(values.begin(), values.end());

        minVal = *mm.first;
        maxVal = *mm.second;
        range = maxVal - minVal;

        return;
    }


    // Percentile trimming

    int total = (int)values.size();
    int trim = int(total * percent / 100.0);

    trim = std::min(trim, total / 2);

    std::nth_element(
        values.begin(),
        values.begin() + trim,
        values.end());

    minVal = values[trim];

    std::nth_element(
        values.begin(),
        values.begin() + total - trim - 1,
        values.end());

    maxVal = values[total - trim - 1];

    range = maxVal - minVal;
}
