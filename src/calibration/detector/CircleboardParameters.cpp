#include "CircleboardParameters.hpp"


CircleboardParameters::CircleboardParameters(
	CircleboardParameters::Geometry g,
	bool isAsymmetric)
	: PatternParameters(isAsymmetric ? BoardPattern::ASYMMETRIC_CIRCLES : BoardPattern::SYMMETRIC_CIRCLES),
	geometry(std::move(g))
{
}

////////////////////////////////////////////////////////////////

const CircleboardParameters::Geometry& CircleboardParameters::getGeometry() const
{
	return geometry;
}


////////////////////////////////////////////////////////////////


CircleboardParameters::Geometry::Geometry(
	int patternWidth,
	int patternHeight,
	float markDiam,
	float centerDist)
	: PatternParameters::Geometry(cv::Size(patternWidth, patternHeight)),
	markDiameter(markDiam),
	centerDistance(centerDist)
{
}

CircleboardParameters::Geometry::Geometry(
	const cv::Size& patternSize,
	float markDiam,
	float centerDist)
	: PatternParameters::Geometry(patternSize),
	markDiameter(markDiam),
	centerDistance(centerDist)
{
}