#pragma once
#include <optional>
#include <vector>
#include <map>


/**
 * @brief A generic container that manages objects of type T in a unique incrementing Id.
 * 
 * This class provides a centralized way to store, retrieve, and track objects using a
 * type-safe Id system. It maintains internal ordering via a std::map.
 * 
 * @tparam T The type of object to be stored in the registry.
 */
template<typename T>
class ObjectRegistry
{
public:
    //-----------------------------------------------------------------------------
    // Support classes
    //-----------------------------------------------------------------------------

    /** * @brief A type-safe wrapper around a 32-bit unsigned integer. */
    class Id
    {
    public:
        explicit Id(std::uint32_t v) : value_(v) {}

        ~Id() = default;


        auto operator<=>(const Id& other) const = default;
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

    ObjectRegistry() = default;

    ~ObjectRegistry() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /**
     * @brief Inserts a new object into the registry.
     * @param obj The object to store.
     * @return The unique Id assigned to the object.
     */
    Id insert(const T& obj);

    /**
     * @brief Removes an object from the registry and determines the next logical selection.
     * * @param toRemoveId The Id of the object to erase.
     * @return std::optional<Id>
     * - std::nullopt: If the Id was not found.
     * - 0: If the object was removed but the collection is now empty.
     * - >0: The Id of the next element, or the previous element if the removed one was the last.
     */
    std::optional<Id> remove(Id toRemoveId);

    /**
     * @brief Replaces the object associated with a specific Id.
     * @param toUpdateId The Id to search for.
     * @param obj The new object data.
     * @return true if the update was successful, false if the Id does not exist.
     */
    bool update(Id toUpdateId, const T& obj);

    /** @brief Returns a reference to the internal map of all objects. */
    const std::map<Id, T>& getAll() const;

    /** @brief Checks if a Id exists in the registry. */
    bool contains(Id id) const;

    /** * @brief Accesses an object by Id.
     * @throw std::out_of_range if the Id is not found.
     */
    const T& at(Id Id) const;

    /** @brief Returns the number of objects currently in the registry. */
    size_t size() const;

    /** @brief Returns true if the registry has no objects. */
    bool empty() const;

    /** @brief Returns the object with the lowest Id. */
    const T& front() const;

    /** @brief Returns the object with the highest Id. */
    const T& back() const;

    /** @brief Removes all objects and resets the registry (Note: does not reset counter). */
    void clear();

private:
    ///< Internal storage mapping IDs to objects.
    std::map<Id, T> idToObject_;

    ///< Counter used to generate the next unique Id.
    std::uint32_t idCounter_{ 0 };
};

#include "ObjectRegistry.inl"