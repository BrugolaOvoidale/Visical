#include "ChessboardParameters.hpp"


ChessboardParameters::ChessboardParameters(ChessboardParameters::Geometry g)
	: PatternParameters(BoardPattern::CHESSBOARD),
	geometry(std::move(g))
{
}

////////////////////////////////////////////////////////////////

const ChessboardParameters::Geometry& ChessboardParameters::getGeometry() const
{
	return geometry;
}

////////////////////////////////////////////////////////////////


ChessboardParameters::Geometry::Geometry(
	int patternWidth,
	int patternHeight,
	float sqSize)
	: PatternParameters::Geometry(cv::Size(patternWidth, patternHeight)),
	squareSize(sqSize)
{
}

ChessboardParameters::Geometry::Geometry(
	const cv::Size& patternSize,
	float sqSize)
	: PatternParameters::Geometry(patternSize),
	squareSize(sqSize)
{
}