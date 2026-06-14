#pragma once
#include <wx/image.h>
#include <wx/colour.h>
#include <opencv2/core/mat.hpp>


// Forward declarations
class CvImage;


class CvToWxImageConverter
{
public:
    static wxImage toWxImage(const CvImage& img);

    static CvImage toCvMat(const wxImage& img);
};