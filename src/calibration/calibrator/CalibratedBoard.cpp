#include "CalibratedBoard.hpp"
#include <stdexcept>
#include "../Board.hpp"


CalibratedBoard::CalibratedBoard(
	const std::shared_ptr<Board>& board,
	const cv::Mat& rvec,
	const cv::Mat& tvec,
	double RMSerror,
	double maxError)
{
	if (!board->isDetected())
		throw std::invalid_argument("Cannot create a calibrated board with a non-detected board");

	board_ = board;
	rvec_ = rvec;
	tvec_ = tvec;
	RMSerror_ = RMSerror;
	maxError_ = maxError;
}

////////////////////////////////////////////////

const std::shared_ptr<Board>& CalibratedBoard::board() const
{
	return board_;
}

double CalibratedBoard::RMSerror() const
{
	return RMSerror_;
}

double CalibratedBoard::maxError() const
{
	return maxError_;
}

const cv::Mat& CalibratedBoard::rvec() const
{
	return rvec_;
}

const cv::Mat& CalibratedBoard::tvec() const
{
	return tvec_;
}
