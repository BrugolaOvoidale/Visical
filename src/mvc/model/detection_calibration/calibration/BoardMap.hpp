#pragma once
#include <memory>
#include <object_registry/ObjectRegistry.hpp>


// Forward declarations
class Board;


/**
 * @brief Specialization of ObjectRegistry for Board objects.
 *
 * Maps Board objects using its internal registry index.
 */
using BoardMap = ObjectRegistry<std::shared_ptr<Board>>;