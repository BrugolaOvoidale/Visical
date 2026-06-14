#include "ChessboardParams.hpp"


ChessboardParams::ChessboardParams(
	int patternWidth,
	int patternHeight,
	float squareSize) :
	PatternParams(BoardPattern::CHESSBOARD, cv::Size(patternWidth, patternHeight)), squareSize_(squareSize)
{
}

ChessboardParams::ChessboardParams(
	const cv::Size& patternSize,
	float squareSize) :
	PatternParams(BoardPattern::CHESSBOARD, patternSize), squareSize_(squareSize)
{
}

////////////////////////////////////////////////////////////////////////////////

float ChessboardParams::squareSize() const
{
	return squareSize_;
}
