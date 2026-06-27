#include "CharucoParameters.hpp"


CharucoParameters::CharucoParameters(CharucoParameters::Geometry g)
	: ArucoParameters(BoardPattern::CHARUCO),
	geometry(std::move(g))
{
}

////////////////////////////////////////////////////////////////

const CharucoParameters::Geometry& CharucoParameters::getGeometry() const
{
	return geometry;
}

////////////////////////////////////////////////////////////////


CharucoParameters::Geometry::Geometry(cv::aruco::CharucoBoard board)
	: PatternParameters::Geometry(board.getChessboardSize()),
	charuco(std::move(board))
{
}
