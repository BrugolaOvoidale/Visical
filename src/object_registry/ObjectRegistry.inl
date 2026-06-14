#pragma once
#include "ObjectRegistry.hpp"
#include <stdexcept>


template<typename T>
void ObjectRegistry<T>::clear()
{
	idToObject_.clear();
}

template<typename T>
ObjectRegistry<T>::Id ObjectRegistry<T>::insert(const T& obj)
{
	ObjectRegistry<T>::Id assignedId(++idCounter_);
	idToObject_.emplace(assignedId, obj);

	return assignedId;
}

template<typename T>
std::optional<typename ObjectRegistry<T>::Id> ObjectRegistry<T>::remove(ObjectRegistry<T>::Id toRemoveId)
{
	std::optional<ObjectRegistry<T>::Id> nextId = std::nullopt;

	auto it = idToObject_.find(toRemoveId);
	if (it == idToObject_.end())
		return std::nullopt;

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
		nextId = ObjectRegistry<T>::Id(0);
	}

	return nextId;
}

template<typename T>
bool ObjectRegistry<T>::update(
	ObjectRegistry<T>::Id toUpdateId,
	const T& obj)
{
	auto it = idToObject_.find(toUpdateId);
	if (it == idToObject_.end()) return false;
	
	idToObject_.emplace(toUpdateId, obj);

	return true;
}

template<typename T>
const std::map<typename ObjectRegistry<T>::Id, T>& ObjectRegistry<T>::getAll() const
{
	return idToObject_;
}

template<typename T>
bool ObjectRegistry<T>::contains(Id id) const
{
	auto it = idToObject_.find(id);
	if (it == idToObject_.end())
		return false;

	return true;
}

template<typename T>
const T& ObjectRegistry<T>::at(ObjectRegistry<T>::Id id) const
{
	auto it = idToObject_.find(id);
	if (it == idToObject_.end())
		throw std::out_of_range("ObjectRegistry::at: id not found");

	return it->second;
}

template<typename T>
size_t ObjectRegistry<T>::size() const
{
	return idToObject_.size();
}

template<typename T>
bool ObjectRegistry<T>::empty() const
{
	return idToObject_.empty();
}

template<typename T>
const T& ObjectRegistry<T>::front() const
{
	if (idToObject_.empty())
		return std::nullopt;

	return idToObject_.begin()->second;
}

template<typename T>
const T& ObjectRegistry<T>::back() const
{
	if (idToObject_.empty())
		return std::nullopt;

	return idToObject_.end()->second;
}