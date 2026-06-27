#include "DistortionModel.hpp"
#include <stdexcept>


DistortionModel::DistortionModel(DistortionModel::Type distType)
	: distType_(distType)
{
    createDistortionCoefficients();
}

DistortionModel::DistortionModel(const cv::Mat& distCoeffs)
{
    validateDistCoeffs(distCoeffs);

    distType_ = coeffsCountToDistortionType(distCoeffs.total()).value();

    distCoeffs_ = distCoeffs.clone();
}

/////////////////////////////////////////////////////////

DistortionModel DistortionModel::standard(
    double k1, double k2, double p1, double p2, double k3)
{
    cv::Mat distCoeffs = cv::Mat::zeros(distortionTypeToCoeffsCount(DistortionModel::Type::STANDARD), 1, CV_64F);

    distCoeffs.at<double>(0) = k1;
    distCoeffs.at<double>(1) = k2;
    distCoeffs.at<double>(2) = p1;
    distCoeffs.at<double>(3) = p2;
    distCoeffs.at<double>(4) = k3;

    return DistortionModel(distCoeffs);
}

DistortionModel DistortionModel::rational(
    double k1, double k2, double p1, double p2, double k3,
    double k4, double k5, double k6)
{
    cv::Mat distCoeffs = cv::Mat::zeros(distortionTypeToCoeffsCount(DistortionModel::Type::RATIONAL), 1, CV_64F);

    distCoeffs.at<double>(0) = k1;
    distCoeffs.at<double>(1) = k2;
    distCoeffs.at<double>(2) = p1;
    distCoeffs.at<double>(3) = p2;
    distCoeffs.at<double>(4) = k3;
    distCoeffs.at<double>(5) = k4;
    distCoeffs.at<double>(6) = k5;
    distCoeffs.at<double>(7) = k6;

    return DistortionModel(distCoeffs);
}

DistortionModel DistortionModel::thinPrism(
    double k1, double k2, double p1, double p2, double k3,
    double k4, double k5, double k6,
    double s1, double s2, double s3, double s4)
{
    cv::Mat distCoeffs = cv::Mat::zeros(distortionTypeToCoeffsCount(DistortionModel::Type::THIN_PRISM), 1, CV_64F);

    distCoeffs.at<double>(0) = k1;
    distCoeffs.at<double>(1) = k2;
    distCoeffs.at<double>(2) = p1;
    distCoeffs.at<double>(3) = p2;
    distCoeffs.at<double>(4) = k3;
    distCoeffs.at<double>(5) = k4;
    distCoeffs.at<double>(6) = k5;
    distCoeffs.at<double>(7) = k6;
    distCoeffs.at<double>(8) = s1;
    distCoeffs.at<double>(9) = s2;
    distCoeffs.at<double>(10) = s3;
    distCoeffs.at<double>(11) = s4;

    return DistortionModel(distCoeffs);
}

DistortionModel DistortionModel::tilted(
    double k1, double k2, double p1, double p2, double k3,
    double k4, double k5, double k6,
    double s1, double s2, double s3, double s4,
    double tx, double ty)
{
    cv::Mat distCoeffs = cv::Mat::zeros(distortionTypeToCoeffsCount(DistortionModel::Type::TILTED), 1, CV_64F);

    distCoeffs.at<double>(0) = k1;
    distCoeffs.at<double>(1) = k2;
    distCoeffs.at<double>(2) = p1;
    distCoeffs.at<double>(3) = p2;
    distCoeffs.at<double>(4) = k3;
    distCoeffs.at<double>(5) = k4;
    distCoeffs.at<double>(6) = k5;
    distCoeffs.at<double>(7) = k6;
    distCoeffs.at<double>(8) = s1;
    distCoeffs.at<double>(9) = s2;
    distCoeffs.at<double>(10) = s3;
    distCoeffs.at<double>(11) = s4;
    distCoeffs.at<double>(12) = tx;
    distCoeffs.at<double>(13) = ty;

    return DistortionModel(distCoeffs);
}

/////////////////////////////////////////////////////////

size_t DistortionModel::distortionTypeToCoeffsCount(DistortionModel::Type distModel)
{
    switch (distModel)
    {
        case DistortionModel::Type::STANDARD:       return 5;

        case DistortionModel::Type::RATIONAL:       return 8;

        case DistortionModel::Type::THIN_PRISM:     return 12;

        case DistortionModel::Type::TILTED:         return 14;
    }
}

std::optional<DistortionModel::Type> DistortionModel::coeffsCountToDistortionType(size_t coeffs)
{
    switch (coeffs)
    {
        case 5:     return DistortionModel::Type::STANDARD;

        case 8:     return DistortionModel::Type::RATIONAL;

        case 12:    return DistortionModel::Type::THIN_PRISM;

        case 14:    return DistortionModel::Type::TILTED;

        default:    return std::nullopt;
    }
}

void DistortionModel::validateDistCoeffs(const cv::Mat& distCoeffs)
{
    // 1. Check that Mat is not empty
    if (distCoeffs.empty())
    {
        throw std::invalid_argument("Distortion coefficients Mat is empty");
    }

    // 2. Check dimensions, should be a column or row vector
    if (distCoeffs.rows != 1 && distCoeffs.cols != 1)
    {
        throw std::invalid_argument("Distortion coefficients must be a 1D vector");
    }

    // 3. Check data depth (should be floating point)
    if (distCoeffs.depth() != CV_32F && distCoeffs.depth() != CV_64F)
    {
        throw std::invalid_argument("Distortion coefficients must be CV_32F or CV_64F");
    }

    // 4. Get the total number of coefficients
    const size_t numCoeffs = distCoeffs.total();

    // 5. Infer distortion type from coefficient count
    // OpenCV supports: 5, 8, 12, or 14 coefficients
    std::optional<DistortionModel::Type> distType = coeffsCountToDistortionType(numCoeffs);
    if (!distType.has_value())
    {
        throw std::invalid_argument(
            "Invalid number of distortion coefficients: " +
            std::to_string(numCoeffs) +
            ". Expected 5, 8, 12, or 14"
        );
    }

    // 6. Optional: validate coefficient values (e.g., check for NaN/Inf)
    for (size_t i = 0; i < numCoeffs; ++i)
    {
        double val = distCoeffs.at<double>(static_cast<int>(i));
        if (std::isnan(val) || std::isinf(val))
        {
            throw std::invalid_argument("Distortion coefficient contains NaN or Inf");
        }
    }
}

const cv::Mat& DistortionModel::coeffs() const
{
    return distCoeffs_;
}

DistortionModel::Type DistortionModel::type() const
{
    return distType_;
}

size_t DistortionModel::coeffsCount() const
{
    return distortionTypeToCoeffsCount(distType_);
}

/////////////////////////////////////////////////////////

void DistortionModel::createDistortionCoefficients()
{
    distCoeffs_ = cv::Mat::zeros(coeffsCount(), 1, CV_64F);
}