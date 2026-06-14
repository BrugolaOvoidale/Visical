#pragma once


/**
 * @brief Identifies the origin of a message.
 */
class SourceId
{
public:
    /** @brief Constructs a SourceId with the 'none' state. */
    SourceId() : id_(none().get()) {}

    /** @brief Constructs a SourceId from a specific integer value. */
    explicit SourceId(int id) : id_(id) {}

    ~SourceId() = default;

    /** @brief Returns a sentinel SourceId representing an invalid or null source. */
    static SourceId none() { return SourceId(-1); }

    /** @brief Returns the underlying integer Id. */
    int get() const { return id_; }

private:
    int id_;
};