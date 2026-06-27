#include "ConstrastCheckDebug.hpp"


ConstrastCheckDebug::ConstrastCheckDebug(
	std::vector<std::vector<CvContour>> dilatedMarks,
	double contrast,
	std::vector<Mark> marks)
	: dilatedMarks_(std::move(dilatedMarks)),
	contrast_(contrast),
	marks_(std::move(marks))
{
}

////////////////////////////////////////////////////

const std::vector<std::vector<CvContour>>& ConstrastCheckDebug::dilatedMarks() const
{
	return dilatedMarks_;
}

double ConstrastCheckDebug::contrast() const
{
	return contrast_;
}

const std::vector<ConstrastCheckDebug::Mark>& ConstrastCheckDebug::marks() const
{
	return marks_;
}