#pragma once
#include <object_registry/BidirectionalObjectRegistry.hpp>
#include "DetectionResult.hpp"


/**
 * @brief Specialization of BidirectionalObjectRegistry for DetectionResult objects.
 *
 * Maps DetectionResult objects using:
 * 
 * - Id: Internal registry index.
 * 
 * - ExternalId: The unique Id defined within DetectionResult::Id.
 */
using DetectionResultMap = BidirectionalObjectRegistry<std::shared_ptr<DetectionResult>, DetectionResult::Id>;