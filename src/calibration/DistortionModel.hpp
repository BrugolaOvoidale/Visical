#pragma once
#include <optional>
#include <opencv2/core/mat.hpp>
#include <enum/EnumTraits.hpp>


/**
 * @brief Manages camera lens distortion coefficients .
 *
 * This class wraps OpenCV's distortion coefficient conventions. It ensures that the
 * underlying 'cv::Mat' (of type 'CV_64F') matches the expected size for the chosen model type.
 */
class DistortionModel
{
public:
    //-----------------------------------------------------------------------------
    // Enums and structs
    //-----------------------------------------------------------------------------

    /**
     * @brief Supported mathematical models for lens distortion.
     */
    enum class Type {
        STANDARD,           ///< Brown-Conrady: k1, k2, p1, p2, k3 (5 coefficients)
        RATIONAL,           ///< + k4, k5, k6 (8 coefficients)
        THIN_PRISM,         ///< + s1, s2, s3, s4 (12 coefficients)
        TILTED              ///< + tauX, tauY (14 coefficients)
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------
    
    /**
     * @brief Initializes a model of the given type with all coefficients set to zero.
     * @param distType The target distortion model.
     */
    DistortionModel(Type distType);

    /**
	* @brief Initializes a model from an existing matrix of coefficients, validiting it. See 'validateDistCoeffs()' for details.
    * @param distCoeffs A 1xN or Nx1 cv::Mat of type CV_64F.
    */
    DistortionModel(const cv::Mat& distCoeffs);

    ~DistortionModel() = default;


    //-----------------------------------------------------------------------------
    // Construction from factories
    //-----------------------------------------------------------------------------
    
    /**
     * @brief Factory for the Standard Brown-Conrady model (5 coeffs).
     * @param k1,k2,k3 Radial coefficients.
     * @param p1,p2 Tangential coefficients.
     */
    static DistortionModel standard(
        double k1, double k2, double p1, double p2, double k3
    );

    /**
     * @brief Factory for the Rational model (8 coeffs).
     */
    static DistortionModel rational(
        double k1, double k2, double p1, double p2, double k3,
        double k4, double k5, double k6
    );


    /**
     * @brief Factory for the Thin-Prism model (12 coeffs).
     */
    static DistortionModel thinPrism(
        double k1, double k2, double p1, double p2, double k3,
        double k4, double k5, double k6,
        double s1, double s2, double s3, double s4
    );

    /**
     * @brief Factory for the Tilted Sensor model (14 coeffs).
     */
    static DistortionModel tilted(
        double k1, double k2, double p1, double p2, double k3,
        double k4, double k5, double k6,
        double s1, double s2, double s3, double s4,
        double tx, double ty
    );


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Maps a model type to the required number of OpenCV coefficients. */
    static int distortionTypeToCoeffsCount(Type distType);

    /** @brief Maps a coefficient count to the corresponding model type. */
    static std::optional<Type> coeffsCountToDistortionType(size_t coeffs);

    /**
     * @brief Validates if the provided matrix matches the current model's requirements.
     * @throw std::invalid_argument if the matrix is malformed.
     */
    static void validateDistCoeffs(const cv::Mat& distCoeffs);

    /** @brief Returns the underlying matrix of coefficients. */
    const cv::Mat& coeffs() const;

    /** @brief Returns the current model type. */
    Type type() const;

    /** @brief Returns the number of coefficients in the current model. */
    int coeffsCount() const;

private:
    // Internal helper to allocate the correctly sized cv::Mat.
    void createDistortionCoefficients();

private:
    // Distortion coefficients (k1,k2,p1,p2,k3[[,k4,k5,k6[,s1,s2,s3,s4[,tx,ty]]]])
    cv::Mat distCoeffs_;

    Type distType_;
};


/** @brief String representation mapping for DistortionModel::Type. */
template <>
struct EnumTraits<DistortionModel::Type> {
    static constexpr std::array<std::pair<DistortionModel::Type, const char*>, 4> values{ {
    {DistortionModel::Type::STANDARD, "standard"},
    {DistortionModel::Type::RATIONAL, "rational"},
    {DistortionModel::Type::THIN_PRISM, "thin_prism"},
    {DistortionModel::Type::TILTED, "tilted"}
    } };
};