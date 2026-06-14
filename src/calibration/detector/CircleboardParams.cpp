#include "CircleboardParams.hpp"


CircleboardParams::CircleboardParams(
	int patternWidth,
	int patternHeight,
	float markDiam,
	float centerDist,
	bool isAsymmetric)
	: PatternParams(
		isAsymmetric ? BoardPattern::ASYMMETRIC_CIRCLES : BoardPattern::SYMMETRIC_CIRCLES,
		cv::Size(patternWidth, patternHeight)),
	markDiameter_(markDiam),
	centerDistance_(centerDist)
{
}

CircleboardParams::CircleboardParams(
	const cv::Size& patternSize,
	float markDiameter,
	float centerDistance,
	bool isAsymmetric)
	: PatternParams(
		isAsymmetric ? BoardPattern::ASYMMETRIC_CIRCLES : BoardPattern::SYMMETRIC_CIRCLES,
		patternSize),
	markDiameter_(markDiameter),
	centerDistance_(centerDistance)
{
}

//////////////////////////////////////////////////////////

float CircleboardParams::centerDistance() const
{
	return centerDistance_;
}

float CircleboardParams::markDiameter() const
{
	return markDiameter_;
}