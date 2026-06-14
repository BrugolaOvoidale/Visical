#pragma once
#include <string>
#include <enum/EnumTraits.hpp>


/**
 * @brief High-level calibration parameters
 *
 * This class encapsulates OpenCV calibration flags in a user-friendly
 * interface.
 */
class CalibrationParameters
{
public:
    //-----------------------------------------------------------------------------
    // Enums and structs
    //-----------------------------------------------------------------------------

    // Solver algorithm
    enum class SolverType {
        SVD,    // Default: Singular Value Decomposition
        QR,     // Faster, potentially less precise
        LU      // Much faster, potentially less precise
    };

    // Principal point handling
    enum class PrincipalPointMode {
        OPTIMIZE,           // Optimize freely
        FIX_AT_CENTER,      // Fix at image center
        FIX_AT_INITIAL      // Fix at provided initial value (requires USE_INITIAL_GUESS, otherwise OpenCV will use its own estimate)
    };

    // Focal length handling
    enum class FocalLengthMode {
        OPTIMIZE_BOTH,      // Optimize fx and fy independently
        FIX_ASPECT_RATIO,   // Optimize one, keep fx/fy ratio
        FIX_BOTH            // Don't optimize (requires USE_INITIAL_GUESS, otherwise OpenCV will use its own estimate)
    };

    // Focal length handling
    enum class TangentialDistortionMode {
        OPTIMIZE,      // Estimate p1, p2 normally
        FORCE_ZERO,    // Force p1 = p2 = 0
        FIX            // Do not optimize (requires USE_INITIAL_GUESS, otherwise OpenCV will use its own estimate)
    };

    //
    struct Defaults {
        static constexpr SolverType solver{ SolverType::SVD };
        static constexpr PrincipalPointMode ppMode{ PrincipalPointMode::OPTIMIZE };
        static constexpr FocalLengthMode focalMode{ FocalLengthMode::OPTIMIZE_BOTH };
        static constexpr TangentialDistortionMode tangentialMode{ TangentialDistortionMode::OPTIMIZE };
        static constexpr bool useInitialGuess{ true };
        static constexpr bool fixK1{ false };
        static constexpr bool fixK2{ false };
        static constexpr bool fixK3{ false };
        static constexpr bool fixK4{ false };
        static constexpr bool fixK5{ false };
        static constexpr bool fixK6{ false };
        static constexpr bool fixS1S2S3S4{ false };
        static constexpr bool fixTauXTauY{ false };
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------
    
    CalibrationParameters() = default;

    ~CalibrationParameters() = default;


public:
    // The algorithm used to solve the linear system.
    SolverType solver{ Defaults::solver };

    // How the principal point is treated during optimization.
    PrincipalPointMode ppMode{ Defaults::ppMode };

    // How focal length parameters are constrained.
    FocalLengthMode focalMode{ Defaults::focalMode };

    // How tangential distortion (p) is handled.
    TangentialDistortionMode tangentialMode{ Defaults::tangentialMode };

    // Whether to use the camera matrix and distortion coefficients provided in the input.
    bool useInitialGuess{ Defaults::useInitialGuess };

    // Flags to fix K1 radial distortion coefficient
    bool fixK1{ Defaults::fixK1 };

    // Flags to fix K2 radial distortion coefficient
    bool fixK2{ Defaults::fixK2 };

    // Flags to fix K3 radial distortion coefficient
    bool fixK3{ Defaults::fixK3 };

    // Flags to fix K4 radial distortion coefficient
    bool fixK4{ Defaults::fixK4 };

    // Flags to fix K5 radial distortion coefficient
    bool fixK5{ Defaults::fixK5 };

    // Flags to fix K6 radial distortion coefficient
    bool fixK6{ Defaults::fixK6 };

    // If true, fixes the thin prism distortion coefficients (s1, s2, s3, s4). */
    bool fixS1S2S3S4{ Defaults::fixS1S2S3S4 };

    // If true, fixes the tilted sensor coefficients (tauX, tauY). */
    bool fixTauXTauY{ Defaults::fixTauXTauY };
};


/** @brief String representation mapping for CalibrationParameters::SolverType. */
template <>
struct EnumTraits<CalibrationParameters::SolverType> {
    static constexpr std::array<std::pair<CalibrationParameters::SolverType, const char*>, 3> values{ {
    {CalibrationParameters::SolverType::SVD, "svd"},
    {CalibrationParameters::SolverType::QR, "qr"},
    {CalibrationParameters::SolverType::LU, "lu"}
    } };
};

/** @brief String representation mapping for CalibrationParameters::PrincipalPointMode. */
template <>
struct EnumTraits<CalibrationParameters::PrincipalPointMode> {
    static constexpr std::array<std::pair<CalibrationParameters::PrincipalPointMode, const char*>, 3> values{ {
    {CalibrationParameters::PrincipalPointMode::OPTIMIZE, "optimize"},
    {CalibrationParameters::PrincipalPointMode::FIX_AT_CENTER, "fix_at_center"},
    {CalibrationParameters::PrincipalPointMode::FIX_AT_INITIAL, "fix_at_initial"}
    } };
};

/** @brief String representation mapping for CalibrationParameters::FocalLengthMode. */
template <>
struct EnumTraits<CalibrationParameters::FocalLengthMode> {
    static constexpr std::array<std::pair<CalibrationParameters::FocalLengthMode, const char*>, 3> values{ {
    {CalibrationParameters::FocalLengthMode::OPTIMIZE_BOTH, "optimize_both"},
    {CalibrationParameters::FocalLengthMode::FIX_ASPECT_RATIO, "fix_aspect_ratio"},
    {CalibrationParameters::FocalLengthMode::FIX_BOTH, "fix_both"}
    } };
};

/** @brief String representation mapping for CalibrationParameters::TangentialDistortionMode. */
template <>
struct EnumTraits<CalibrationParameters::TangentialDistortionMode> {
    static constexpr std::array<std::pair<CalibrationParameters::TangentialDistortionMode, const char*>, 3> values{ {
        {CalibrationParameters::TangentialDistortionMode::OPTIMIZE,   "optimize"},
        {CalibrationParameters::TangentialDistortionMode::FORCE_ZERO, "force_zero"},
        {CalibrationParameters::TangentialDistortionMode::FIX,        "fix"}
    } };
};