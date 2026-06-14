#include "PatternParams.hpp"


PatternParams::PatternParams(
	BoardPattern type,
	const cv::Size& patternSize)
	: type_(type), 
	patternSize_(patternSize)
{
}

//////////////////////////////////

BoardPattern PatternParams::patternType() const
{
	return type_;
}

cv::Size PatternParams::patternSize() const
{
	return patternSize_;
}