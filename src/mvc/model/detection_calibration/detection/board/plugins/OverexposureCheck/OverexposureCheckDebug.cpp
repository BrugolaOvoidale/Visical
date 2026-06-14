#include "OverexposureCheckDebug.hpp"


OverexposureCheckDebug::OverexposureCheckDebug(
	const CvRegion& boardRegion,
	const CvRegion& overexpRegion,
	double boardArea,
	double areaOverexp)
	: overexpRegion_(overexpRegion),
	boardRegion_(boardRegion),
	areaPlate_(boardArea),
	areaOverexp_(areaOverexp)
{
}

////////////////////////////////////

const CvRegion& OverexposureCheckDebug::boardRegion() const
{
	return boardRegion_;
}

const CvRegion& OverexposureCheckDebug::overexposedRegion() const
{
	return overexpRegion_;
}

double OverexposureCheckDebug::boardArea() const
{
	return areaPlate_;
}

double OverexposureCheckDebug::overexposedArea() const
{
	return areaOverexp_;
}