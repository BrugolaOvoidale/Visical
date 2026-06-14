#pragma once
#include <stdexcept>
#include "BidirectionalObjectRegistry.hpp"


template<typename T, typename ExternalId>
void BidirectionalObjectRegistry<T, ExternalId>::clear()
{
	idToObject_.clear();
	extIdToId_.clear();
	idToExtId_.clear();
}

template<typename T, typename ExternalId>
BidirectionalObjectRegistry<T, ExternalId>::Id BidirectionalObjectRegistry<T, ExternalId>::insert(
	const T& obj,
	const ExternalId& extId)
{
	BidirectionalObjectRegistry<T, ExternalId>::Id assignedId(++idCounter_);

	idToObject_.emplace(assignedId, obj);

	extIdToId_.emplace(extId, assignedId);

	idToExtId_.emplace(assignedId, extId);

	return assignedId;
}

template<typename T, typename ExternalId>
std::optional<typename BidirectionalObjectRegistry<T, ExternalId>::Id> BidirectionalObjectRegistry<T, ExternalId>::remove(BidirectionalObjectRegistry<T, ExternalId>::Id toRemoveId)
{
	auto it = idToObject_.find(toRemoveId);
	if (it == idToObject_.end())
		return std::nullopt;

	std::optional<BidirectionalObjectRegistry<T, ExternalId>::Id> nextId = std::nullopt;

	// Erase from the auxiliary containers first
	ExternalId extId = idToExtId_.at(toRemoveId);

	extIdToId_.erase(extId);

	idToExtId_.erase(toRemoveId);

	// Erase from the main map
	auto nextIt = idToObject_.erase(it);

	// Determine what to return
	if (nextIt != idToObject_.end())
	{
		// There is a next element
		nextId = nextIt->first;
	}
	else if (!idToObject_.empty())
	{
		// No next element, but not empty -> return previous element
		nextId = std::prev(idToObject_.end())->first;
	}
	else
	{
		// Collection is now empty
		nextId = BidirectionalObjectRegistry<T, ExternalId>::Id(0);
	}

	return nextId;
}

template<typename T, typename ExternalId>
std::optional<typename BidirectionalObjectRegistry<T, ExternalId>::Id> BidirectionalObjectRegistry<T, ExternalId>::remove(const ExternalId& toRemoveId)
{
	auto it = extIdToId_.find(toRemoveId);
	if (it == extIdToId_.end())
		return std::nullopt;

	return remove(it->second);
}

template<typename T, typename ExternalId>
bool BidirectionalObjectRegistry<T, ExternalId>::replace(
	BidirectionalObjectRegistry<T, ExternalId>::Id toUpdateId,
	const T& obj,
	const ExternalId& newExtId)
{
	auto it = idToObject_.find(toUpdateId);
	if (it == idToObject_.end())
		return false;

	// Get old ExternalId
	const ExternalId& oldExtId = idToExtId_.at(toUpdateId);

	// Optional: prevent collisions
	auto existing = extIdToId_.find(newExtId);
	if (existing != extIdToId_.end() && existing->second != toUpdateId)
		return false;

	// Remove old mapping
	extIdToId_.erase(oldExtId);

	// Replace object
	it->second = obj;

	// Insert new mapping
	extIdToId_.emplace(newExtId, toUpdateId);
	idToExtId_.emplace(toUpdateId, newExtId);

	return true;
}

template<typename T, typename ExternalId>
const std::map<typename BidirectionalObjectRegistry<T, ExternalId>::Id, T>& BidirectionalObjectRegistry<T, ExternalId>::getAll() const
{
	return idToObject_;
}

template<typename T, typename ExternalId>
bool BidirectionalObjectRegistry<T, ExternalId>::contains(Id slot) const
{
	auto it = idToObject_.find(slot);
	if (it == idToObject_.end())
		return false;

	return true;
}

template<typename T, typename ExternalId>
bool BidirectionalObjectRegistry<T, ExternalId>::contains(const ExternalId& extId) const
{
	auto it = extIdToId_.find(extId);
	if (it == extIdToId_.end())
		return false;

	return true;
}

template<typename T, typename ExternalId>
const T& BidirectionalObjectRegistry<T, ExternalId>::at(BidirectionalObjectRegistry<T, ExternalId>::Id slot) const
{
	auto it = idToObject_.find(slot);
	if (it == idToObject_.end())
		throw std::out_of_range("BidirectionalObjectRegistry::at: slot not found");

	return it->second;
}

template<typename T, typename ExternalId>
const T& BidirectionalObjectRegistry<T, ExternalId>::at(const ExternalId& extId) const
{
	auto it = extIdToId_.find(extId);
	if (it == extIdToId_.end())
		throw std::out_of_range("BidirectionalObjectRegistry::at: slot not found");

	return at(it->second);
}

template<typename T, typename ExternalId>
size_t BidirectionalObjectRegistry<T, ExternalId>::size() const
{
	return idToObject_.size();
}

template<typename T, typename ExternalId>
bool BidirectionalObjectRegistry<T, ExternalId>::empty() const
{
	return idToObject_.empty();
}

template<typename T, typename ExternalId>
const T& BidirectionalObjectRegistry<T, ExternalId>::front() const
{
	if (idToObject_.empty())
		throw std::out_of_range("BidirectionalObjectRegistry::front: slot not found");

	return idToObject_.begin()->second;
}

template<typename T, typename ExternalId>
const T& BidirectionalObjectRegistry<T, ExternalId>::back() const
{
	if (idToObject_.empty())
		throw std::out_of_range("BidirectionalObjectRegistry::back: slot not found");

	return idToObject_.end()->second;
}