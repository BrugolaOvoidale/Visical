#pragma once
#include <chrono>
#include <atomic>
#include <string>
#include "board/EvaluatedBoard.hpp"


/**
 * @brief Base class for board detection outcomes.
 *
 * This class provides common metadata for any detection event, including a
 * unique session-wide Id, a high-resolution timestamp, and the processed board data.
 *
 * Nothe that this class cannot be instantiated directly. Use one of its specialized
 * subclasses depending on the data source.
 */
class DetectionResult
{
public:
    //-----------------------------------------------------------------------------
    // Enums and structs
    //-----------------------------------------------------------------------------

    /** @brief Defines the origin of the detection data. */
    enum class Source {
        FILE,               ///< Loaded from a static image on disk.
        CAMERA_SNAPSHOT,    ///< Captured as a single frame from a camera.
        CAMERA_LIVE         ///< Captured from a continuous video stream.
    };

    /**
     * @brief A 64-bit unique identifier for detection results.
     */
    class Id
    {
    public:
        explicit Id(std::uint64_t v) : value_(v) {}

        ~Id() = default;

        auto operator<=>(const Id& other) const = default;
        auto operator<=>(std::uint64_t other) const { return value_ <=> other; }
        bool operator==(std::uint64_t other) const { return value_ == other; }

        /** @brief Returns the numeric value of the Id. */
        std::uint64_t getValue() const { return value_; }

    private:
        std::uint64_t value_;
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    virtual ~DetectionResult() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the source type. */
    Source source() const;

    /** @brief Returns the shared pointer to the evaluated board analysis. */
    const std::shared_ptr<EvaluatedBoard>& evaluatedBoard() const;

    /** @brief Returns the point in time when the object was created. */
    const std::chrono::steady_clock::time_point& timestamp() const;

    /** @brief Returns the unique auto-incremented Id of this result. */
    Id id() const;

protected:
    // Protected constructor to be called by derived classes.
    DetectionResult(
        Source source,
        const std::shared_ptr<EvaluatedBoard>& evalBoard
    );

private:
    Source boardSource_;

    std::shared_ptr<EvaluatedBoard> evalBoard_;

    std::chrono::steady_clock::time_point timestamp_;

    Id id_;

    // Thread-safe counter used to assign unique Ids across all instances.
    static inline std::atomic<std::uint64_t> idCounter{ 0 };
};

/////////////////////////////////////////////////////////////

/**
 * @brief Represents a detection result sourced from an image file.
 */
class DetectionResultFile : public DetectionResult
{
public:
    /**
     * @brief Constructs a file-based detection result.
     * @param evalBoard Evaluated board data.
     * @param imagePath The filesystem path to the source image.
     */
    DetectionResultFile(
        const std::shared_ptr<EvaluatedBoard>& evalBoard,
        const std::string& imagePath
    );

    ~DetectionResultFile() = default;

    /** @brief Returns the file path of the original image. */
    const std::string& imagePath() const;

private:
    std::string imagePath_;
};

/////////////////////////////////////////////////////////////

/**
 * @brief Represents a detection result sourced from a manual camera snapshot.
 */
class DetectionResultSnap : public DetectionResult
{
public:
    DetectionResultSnap(const std::shared_ptr<EvaluatedBoard>& evalBoard);

    ~DetectionResultSnap() = default;
};

/////////////////////////////////////////////////////////////

/**
 * @brief Represents a detection result sourced from a live video stream.
 */
class DetectionResultLive : public DetectionResult
{
public:
    DetectionResultLive(const std::shared_ptr<EvaluatedBoard>& evalBoard);

    ~DetectionResultLive() = default;
};

