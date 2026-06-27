#include "AprilTagParameters.hpp"


AprilTagParameters::AprilTagParameters(
	AprilTagParameters::Geometry g)
	: ArucoParameters(BoardPattern::APRIL_TAG),
	geometry(std::move(g))
{
}

////////////////////////////////////////////////////////////////

const AprilTagParameters::Geometry& AprilTagParameters::getGeometry() const
{
	return geometry;
}


////////////////////////////////////////////////////////////////


AprilTagParameters::Geometry::Geometry(
	cv::aruco::GridBoard board,
	cv::aruco::PredefinedDictionaryType dict)
	: PatternParameters::Geometry(board.getGridSize()),
	aprilTag(std::move(board)),
	dict_(dict)
{
}

////////////////////////////////////////////////////////////////

cv::aruco::PredefinedDictionaryType AprilTagParameters::Geometry::dictionary() const
{
	return dict_;
}