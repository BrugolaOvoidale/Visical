#include "Board.hpp"


Board::Board(
    const CvImage& image,
    BoardPattern patternType,
    cv::Size patternSize,
    CvContour boardContour,
    std::vector<CvContour> marksContours,
    std::vector<cv::Point2f> imagePoints,
    std::vector<cv::Point3f> objectPoints,
    const cv::Mat& rvec,
    const cv::Mat& tvec,
    std::vector<cv::Point2f> axes2D)
    : image_(image),
    patternType_(patternType),
    patternSize_(patternSize),
    boardContour_(std::move(boardContour)),
    marksContours_(std::move(marksContours)),
    imagePoints_(std::move(imagePoints)),
    objectPoints_(std::move(objectPoints)),
    pose_({ rvec, tvec }),
    axes2D_(std::move(axes2D)),
    errorCode_(ErrorCode::NONE)
{
}

/////////////////////////////////////////////////////

Board Board::notDetected(
    const CvImage& image,
    BoardPattern patternType,
    cv::Size patternSize)
{
    Board b;

	b.image_ = image;
	b.patternType_ = patternType;
	b.patternSize_ = patternSize;
	b.errorCode_ = ErrorCode::NOT_DETECTED;

    return b;
}

Board Board::poseNotSolved(
    const CvImage& image,
    BoardPattern patternType,
    cv::Size patternSize,
    CvContour boardContour,
    std::vector<cv::Point2f> imagePoints,
    std::vector<cv::Point3f> objectPoints)
{
    Board b;

    b.image_ = image;
    b.patternType_ = patternType;
    b.patternSize_ = patternSize;
    b.boardContour_ = std::move(boardContour);
    b.imagePoints_ = std::move(imagePoints);
    b.objectPoints_ = std::move(objectPoints);
    b.errorCode_ = ErrorCode::POSE_NOT_SOLVED;

    return b;
}

/////////////////////////////////////////////////////

const CvImage& Board::image() const
{
    return image_;
}

BoardPattern Board::patternType() const
{
    return patternType_;
}

cv::Size Board::patternSize() const
{
    return patternSize_;
}

const CvContour& Board::boardContour() const
{
    return boardContour_;
}

const std::vector<CvContour>& Board::marksContours() const
{
    return marksContours_;
}

const std::vector<cv::Point2f>& Board::imagePoints() const
{
    return imagePoints_;
}

const std::vector<cv::Point3f>& Board::objectPoints() const
{
    return objectPoints_;
}

const Board::Pose& Board::pose() const
{
    return pose_;
}

const std::vector<cv::Point2f>& Board::axes() const
{
    return axes2D_;
}

Board::ErrorCode Board::errorCode() const
{
    return errorCode_;
}

bool Board::isDetected() const
{
    return errorCode_ != ErrorCode::NOT_DETECTED;
}

bool Board::hasPose() const
{
    return errorCode_ == ErrorCode::NONE;
}
