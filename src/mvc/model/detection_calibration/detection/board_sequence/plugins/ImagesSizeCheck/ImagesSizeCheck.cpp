#include "ImagesSizeCheck.hpp"


ImagesSizeCheck::ImagesSizeCheck(double threshold)
	: Plugin(
		std::string(ID),
		std::string(NAME),
		std::string(DESCRIPTION),
		threshold)
{
}

/////////////////////////////////////////////////////////////

std::shared_ptr<ImagesSizeCheck> ImagesSizeCheck::create(double threshold)
{
	struct Enabler : public ImagesSizeCheck
	{
		Enabler(double thr) : ImagesSizeCheck(thr) {}
	};

	return std::make_shared<Enabler>(threshold);
}

/////////////////////////////////////////////////////////////

std::shared_ptr<PluginResult> ImagesSizeCheck::executeImpl(const std::vector<std::shared_ptr<Board>>& boards) const
{
	if (boards.empty())
	{
		return executionNotApplicable("No boards provided");
	}


	const cv::Size refSize = boards.back()->image().size();
	bool success = true;

	for (const auto& board : boards)
	{
		if (board->image().size() != refSize)
		{
			success = false;
			break;
		}
	}

	if (success)
		return executionResult(100.0);
	else
		return executionFailed("Not all image sizes are identical");
}

void ImagesSizeCheck::validateParameters() const
{
}

std::string ImagesSizeCheck::getMessageForSeverity(EvaluationSeverity severity) const
{
	switch (severity)
	{
	case EvaluationSeverity::FAILED:
		return "Images size check failed";
	case EvaluationSeverity::CRITICAL:
		return "Not all image sizes are identical";
	case EvaluationSeverity::WARNING:
		return "Not all image sizes are identical";
	case EvaluationSeverity::OK:
	default:
		return "";
	}
}