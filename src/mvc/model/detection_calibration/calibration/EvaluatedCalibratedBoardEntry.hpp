#pragma once
#include "BoardMap.hpp"
#include "calibrated_board/EvaluatedCalibratedBoard.hpp"


/**
 * @brief A container associating a BoardMap Id with its corresponding evaluation results.
 *
 * This class acts as a link between the registry system (BoardMap) and the
 * physical calibration analysis results (EvaluatedCalibratedBoard).
 */
class EvaluatedCalibratedBoardEntry
{
public:
    //-----------------------------------------------------------------------------
    // Constructor/Destructor
    //-----------------------------------------------------------------------------

    /**
     * @brief Constructs an entry with a specific Id and its associated evaluation data.
     * @param mapId The unique identifier from the BoardMap registry.
     * @param evalCalibBoard The evaluation results for the calibrated board.
     */
    EvaluatedCalibratedBoardEntry(
        BoardMap::Id mapId,
        EvaluatedCalibratedBoard evalCalibBoard
    );

    ~EvaluatedCalibratedBoardEntry() = default;


    //-----------------------------------------------------------------------------
    // Class operators
    //-----------------------------------------------------------------------------

    /** @brief Returns the ID of the board in the map. */
    BoardMap::Id mapId() const;

    /** @brief Returns a constant reference to the evaluation results. */
    const EvaluatedCalibratedBoard& evaluatedCalibratedBoard() const;

private:
    // Unique identifier for the board.
    BoardMap::Id mapId_;

    // Associated calibration evaluation data.
    EvaluatedCalibratedBoard evalCalibBoard_;
};