#pragma once
#include <string>
#include <chrono>
#include <source_location>
#include <enum/EnumTraits.hpp>


/**
 * @brief Encapsulates a single log message with associated metadata.
 * 
 * This class automatically captures the "what" (message), "who" (context),
 * "where" (source file/line), "when" (timestamp), and severity "level" of an event.
 */
class Log
{
public:
    //-----------------------------------------------------------------------------
    // Enums and support classes
    //-----------------------------------------------------------------------------

    /** @brief Severity levels for log filtering and display. */
    enum class Level {
        LVL_DEBUG,      ///< Fine-grained informational events that are most useful to debug an application.
        LVL_INFO,       ///< Informational messages that highlight the progress of the application.
        LVL_WARNING,    ///< Potentially harmful situations.
        LVL_ERROR       ///< Error events that might still allow the application to continue running.
    };

    /**
     * @brief Captures the source code location where the log was generated.
     */
    class Where
    {
    public:
        /** @brief Explicitly construct location from file name and line number. */
        Where(std::string file, int line)
            : file_(std::move(file)), line_(line) {
        }

        /**
         * @brief Automatically captures the current source location.
         * @param location Defaults to the caller's location via std::source_location::current().
         */
        Where(const std::source_location& location = std::source_location::current())
            : file_(location.file_name()), line_(location.line()) {
        }

        ~Where() = default;

    public:
        /** @brief Returns the source file path. */
        const std::string& file() const { return file_; }

        /** @brief Returns the source line number. */
        int line() const { return line_; }

    private:
        std::string file_;
        int line_;
    };


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    Log() = default;

    /**
     * @brief Constructs a log entry from a std::string message.
     * @param what The log message content.
     * @param level The severity of the log.
     * @param location Captured source location (defaults to construction site).
     */
    Log(
        std::string what,
        Level level = Level::LVL_DEBUG,
        const std::source_location& location = std::source_location::current()
    );
    Log(
        const char* what,
        Level level = Level::LVL_DEBUG,
        const std::source_location& location = std::source_location::current()
    );

    ~Log() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the log message. */
    const std::string& what() const;

    /** @brief Returns the identifier of the entity that created the log. */
    const std::string& who() const;

    /** @brief Returns the source code location where the log originated. */
    const Where& where() const;

    /** @brief Returns the system timestamp when the log was created. */
    const std::chrono::system_clock::time_point& when() const;

    /** @brief Returns the severity level of the log. */
    Level level() const;

private:
    std::string what_;

    std::string who_;

    Where where_;

    std::chrono::system_clock::time_point when_;

    Level level_;
};

/**
 * @brief Template specialization for EnumTraits to support string conversion of Log::Level.
 */
template <>
struct EnumTraits<Log::Level> {
    static constexpr std::array<std::pair<Log::Level, const char*>, 4> values{ {
    {Log::Level::LVL_DEBUG, "DEBUG"},
    {Log::Level::LVL_INFO, "INFO"},
    {Log::Level::LVL_WARNING, "WARNING"},
    {Log::Level::LVL_ERROR, "ERROR"},
    } };
};