#include "CvToWxImageConverter.hpp"
#include <cv/CvImage.hpp>


wxImage CvToWxImageConverter::toWxImage(const CvImage& img)
{
    cv::Mat bgrMat = img.toBGR().mat();
    cv::Mat rgbMat;
    cv::cvtColor(bgrMat, rgbMat, cv::COLOR_BGR2RGB);

    wxImage wxImg(rgbMat.cols, rgbMat.rows);

    std::memcpy(wxImg.GetData(), rgbMat.data, rgbMat.cols * rgbMat.rows * 3);

    return wxImg;
}

CvImage CvToWxImageConverter::toCvMat(const wxImage& img)
{
    cv::Mat mat(img.GetHeight(), img.GetWidth(), CV_8UC3);

    std::memcpy(mat.data, img.GetData(), img.GetWidth() * img.GetHeight() * 3);

    cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);

    return CvImage(mat);
}