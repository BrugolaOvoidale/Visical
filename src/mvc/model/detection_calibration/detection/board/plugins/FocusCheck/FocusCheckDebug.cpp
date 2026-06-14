#include "FocusCheckDebug.hpp"


FocusCheckDebug::FocusCheckDebug(
    CvImage edgeImage,
    double contrast,
    std::vector<Mark> marks)
    : edgeImage_(std::move(edgeImage)),
    contrast_(contrast),
    marks_(std::move(marks))
{
}

///////////////////////////////////////

const CvImage& FocusCheckDebug::edgeImage() const
{
    return edgeImage_;
}

double FocusCheckDebug::contrast() const
{
    return contrast_;
}

const std::vector<FocusCheckDebug::Mark>& FocusCheckDebug::marks() const
{
    return marks_;
}