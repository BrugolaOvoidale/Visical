#include "TiltCheckDebug.hpp"


TiltCheckDebug::TiltCheckDebug(
	std::vector<std::optional<BoardPose>> entries,
	std::vector<double> sortedSlant,
	std::vector<double> sortedPan,
	int used,
	double slantSc,
	double panSc)
	: boardEntries_(std::move(entries)),
	sortedSlantScores_(std::move(sortedSlant)),
	sortedPanScores_(std::move(sortedPan)),
	numUsed_(used),
	slantScore_(slantSc),
	panScore_(panSc)
{
}

////////////////////////////////

const std::vector<std::optional<TiltCheckDebug::BoardPose>> &TiltCheckDebug::poses() const
{
	return boardEntries_;
}

const std::vector<double>& TiltCheckDebug::sortedSlantScores() const
{
	return sortedSlantScores_;
}

const std::vector<double>& TiltCheckDebug::sortedPanScores() const
{
	return sortedPanScores_;
}

int TiltCheckDebug::numUsed() const
{
	return numUsed_;
}

double TiltCheckDebug::slantScore() const
{
	return slantScore_;
}

double TiltCheckDebug::panScore() const
{
	return panScore_;
}