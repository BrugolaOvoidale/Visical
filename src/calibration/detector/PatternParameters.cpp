#include "PatternParameters.hpp"
#include <stdexcept>


PatternParameters::PatternParameters(BoardPattern type)
	: type_(type)
{
}

//////////////////////////////////

BoardPattern PatternParameters::patternType() const
{
	return type_;
}


//////////////////////////////////


PatternParameters::Geometry::Geometry(const cv::Size& size)
	: patternSize_(size)
{
}

//////////////////////////////////

cv::Size PatternParameters::Geometry::patternSize() const
{
	return patternSize_;
}