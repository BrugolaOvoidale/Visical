#include "ConstrastCheckDebug.hpp"


ConstrastCheckDebug::ConstrastCheckDebug(
	std::vector<std::vector<CvContour>> dilatedMarks,
	std::vector<Mark> marks)
	: dilatedMarks_(std::move(dilatedMarks)),
	marks_(std::move(marks))
{
}

////////////////////////////////////////////////////

const std::vector<std::vector<CvContour>>& ConstrastCheckDebug::dilatedMarks() const
{
	return dilatedMarks_;
}

const std::vector<ConstrastCheckDebug::Mark>& ConstrastCheckDebug::marks() const
{
	return marks_;
}