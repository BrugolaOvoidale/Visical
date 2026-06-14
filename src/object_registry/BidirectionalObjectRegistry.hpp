#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <map>


/**
 * @brief A registry that maps objects to both an internal Id and an ExternalId.
 * 
 * This class facilitates two-way lookups:
 * 
 * 1. Find an object or ExternalId using a Id.
 * 
 * 2. Find an object or Id using an ExternalId.
 * 
 * @tparam T The type of object to be stored.
 * @tparam ExternalId The type used as an external unique identifier.
 */
template<typename T, typename ExternalId>
class BidirectionalObjectRegistry
{
public:
    //-----------------------------------------------------------------------------
    // Support classes
    //-----------------------------------------------------------------------------

    /** * @brief Represents a stable internal index/slot for an object within the registry. */
    class Id
    {
    public:
        explicit Id(std::uint32_t v) : value_(v) {}

        ~Id() = default;


        auto operator<=>(const Id& other) const = default;
        bool operator==(const Id& other) const { return value_ == other.value_; }
        auto operator<=>(std::uint32_t other) const { return value_ <=> other; }
        bool operator==(std::uint32_t other) const { return value_ == other; }

        /** @brief Returns the raw numeric value of the Id. */
        std::uint32_t get() const { return value_; }

    private:
        std::uint32_t value_;
    };

    /** @brief Represents a single entry in the registry. */
	using Entry = std::pair<Id, T>;


    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    BidirectionalObjectRegistry() = default;

    ~BidirectionalObjectRegistry() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Inserts an object with a corresponding external identifier.
     * @param obj The object to store.
     * @param extId The unique external Id to associate with this object.
     * @return The generated Id assigned to this entry.
     */
    Id insert(const T& obj, const ExternalId& extId);

    /**
     * @brief Removes an entry using its internal Id.
     * @param toRemoveId The slot to clear.
     * @return std::optional<Id>
     * - @b std::nullopt: If the Id was not found.
     * - @b 0: If the object was removed but the collection is now empty.
     * - @b >0: The Id of the next element, or the previous element if the removed one was the last.
     */
    std::optional<Id> remove(Id toRemoveId);

    /**
     * @brief Removes an entry using its ExternalId.
     * @param extId The external identifier to search for.
     * @return std::optional<Id>
     * - std::nullopt: If the Id was not found.
     * - 0: If the object was removed but the collection is now empty.
     * - >0: The Id of the next element, or the previous element if the removed one was the last.
     */
    std::optional<Id> remove(const ExternalId& extId);

    /**
     * @brief Updates both the object and its external identifier for a given slot.
     * @param toUpdateId The slot to modify.
     * @param obj The new object data.
     * @param newExtId The new external Id.
     * @return true if the slot existed and was updated.
     */
    bool replace(Id toUpdateId, const T& obj, const ExternalId& newExtId);

    /** @brief Returns a reference to the internal Id-to-Object map. */
    const std::map<Id, T>& getAll() const;

    /** @brief Checks if a specific Id exists. */
    bool contains(Id slot) const;

    /** @brief Checks if a specific ExternalId exists. */
    bool contains(const ExternalId& extId) const;

    /** @brief Accesses an object by its Id. @throw std::out_of_range if not found. */
    const T& at(Id Id) const;

    /** @brief Accesses an object by its ExternalId. @throw std::out_of_range if not found. */
    const T& at(const ExternalId& extId) const;

    /** @brief Returns current population size. */
    size_t size() const;

    /** @brief Returns true if registry is empty. */
    bool empty() const;

    /** @brief Accesses the first element (lowest Id value). */
    const T& front() const;

    /** @brief Accesses the last element (highest Id value). */
    const T& back() const;

    /** @brief Clears all objects and Id mappings. */
    void clear();

private:
    ///< Primary storage: Id -> Data
    std::map<Id, T> idToObject_;

    ///< Secondary index: ExternalId -> Id
    std::map<ExternalId, Id> extIdToId_;

    ///< Reverse index: Id -> ExternalId
    std::map<Id, ExternalId> idToExtId_;

    std::uint32_t idCounter_{ 0 };
};

#include "BidirectionalObjectRegistry.inl"