#include "IlluminationCheckDebug.hpp"


IlluminationCheckDebug::IlluminationCheckDebug(
	std::vector<std::vector<CvContour>> dilatedMarks,
	double mean,
	double stdDev,
	std::vector<Mark> marks)
	: dilatedMarks_(std::move(dilatedMarks)),
	mean_(mean),
	stdDev_(stdDev),
	marks_(std::move(marks))
{

}

////////////////////////////////////////////////////

const std::vector<std::vector<CvContour>>& IlluminationCheckDebug::dilatedMarks() const
{
	return dilatedMarks_;
}

double IlluminationCheckDebug::mean() const
{
	return mean_;
}

double IlluminationCheckDebug::stdDev() const
{
	return stdDev_;
}

const std::vector<IlluminationCheckDebug::Mark>& IlluminationCheckDebug::marks() const
{
	return marks_;
}