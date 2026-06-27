#include "DetectionUtility.hpp"
#include <execution>
#include <thread/Thread.hpp>
#include <cv/CvPainter.hpp>
#include <CvToWxImageConverter.hpp>
#include <evaluator/EvaluationResult.hpp>
#include <mvc/model/detection_calibration/detection/board/EvaluatedBoard.hpp>
#include <mvc/model/detection_calibration/detection/board/plugins/BoardSizeCheck/BoardSizeCheck.hpp>
#include <mvc/model/detection_calibration/detection/board/plugins/ContrastCheck/ContrastCheck.hpp>
#include <mvc/model/detection_calibration/detection/board/plugins/FocusCheck/FocusCheck.hpp>
#include <mvc/model/detection_calibration/detection/board/plugins/IlluminationCheck/IlluminationCheck.hpp>
#include <mvc/model/detection_calibration/detection/board/plugins/OverexposureCheck/OverexposureCheck.hpp>
#include <mvc/model/detection_calibration/detection/board_sequence/plugins/FOVCoverageCheck/FOVCoverageCheck.hpp>
#include "DetectionResultView.hpp"


DetectionUtility::DetectionUtility()
{
	worker_ = std::make_unique<Thread>();

    worker_->start();
}

////////////////////////////////////////////////////////////////////////////////////////////

TaskResult DetectionUtility::saveImages(
    const std::string& path,
    const std::map<DetectionResultMap::Id, std::shared_ptr<DetectionResult>>& results)
{
    std::atomic<size_t> counter{ 0 };

    std::for_each(std::execution::par, results.begin(), results.end(),
        [&](const DetectionResultMap::Entry& kv)
        {
            size_t i = counter++;
            const std::string fileName = std::format("image_{:02d}", i) + ".png";

            kv.second
                ->evaluatedBoard()
                ->object()
                ->image()
                .write(path + "/" + fileName);
        }
    );

    return true;
}

void DetectionUtility::cook(const DetectionResultMap::Entry& boardRes)
{
    if (!boardRes.second)
        return;

    addCookTask();
    worker_->enqueueTask([this, boardRes]() {

        try
        {
            storeLatestBoardResult(boardRes);
        }
        catch (...)
        {
            // TODO: attach logger
        }

        completeCookTask();

        });
}

void DetectionUtility::cook(const std::shared_ptr<EvaluatedBoardSequence>& seqRes)
{
    if (!seqRes)
        return;

    addCookTask();
    worker_->enqueueTask([this, seqRes]() {

        try
        {
            storeLatestSequenceResults(seqRes);
        }
        catch (...)
        {
            // TODO: attach logger
        }

        completeCookTask();

        });
}

std::optional<PluginDebugResultView> DetectionUtility::buildPluginDebugResult(
    const std::string& pluginId,
    const std::shared_ptr<EvaluatedBoard>& evalBoard) const
{
    if (pluginId.empty() || !evalBoard)
    {
        return std::nullopt;
    }

    const std::vector<std::shared_ptr<PluginResult>>& pluginsResults = evalBoard->assessments();

    if (pluginsResults.empty())
    {
        return std::nullopt;
    }

    for (const auto& a : pluginsResults)
    {
        if (a->plugin()->id() != pluginId)
            continue;

        std::shared_ptr<PluginDebugResult> deb = a->debugResult();

        if (!deb)
            return std::nullopt;

        if (pluginId == ContrastCheck::ID)
        {
            return buildContrastCheckResult(
                evalBoard,
                *std::static_pointer_cast<ConstrastCheckDebug>(deb)
            );
        }
        else if (pluginId == FocusCheck::ID)
        {
            return buildFocusCheckResult(
                evalBoard,
                *std::static_pointer_cast<FocusCheckDebug>(deb)
            );
        }
        else if (pluginId == IlluminationCheck::ID)
        {
            return buildIlluminationCheckResult(
                evalBoard,
                *std::static_pointer_cast<IlluminationCheckDebug>(deb)
            );
        }
        else if (pluginId == OverexposureCheck::ID)
        {
            return buildOverexposureCheckResult(
                evalBoard,
                *std::static_pointer_cast<OverexposureCheckDebug>(deb)
            );
        }
        else
        {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

std::optional<PluginDebugResultView> DetectionUtility::buildPluginDebugResult(
    const std::string& pluginId,
    const std::shared_ptr<EvaluatedBoardSequence>& evalSequence) const
{
    if (pluginId.empty() || !evalSequence)
    {
        return std::nullopt;
    }

    const std::vector<std::shared_ptr<PluginResult>>& pluginsResults = evalSequence->assessments();

    if (pluginsResults.empty())
    {
        return std::nullopt;
    }

    for (const auto& a : pluginsResults)
    {
        if (a->plugin()->id() != pluginId)
            continue;

        std::shared_ptr<PluginDebugResult> deb = a->debugResult();

        if (!deb)
            return std::nullopt;

        if (pluginId == FOVCoverageCheck::ID)
        {
            return buildFOVCoverageCheckResult(
                evalSequence,
                *std::static_pointer_cast<FOVCoverageCheckDebug>(deb)
            );
        }
        else
        {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

std::optional<wxString> DetectionUtility::getPluginPixelTooltip(
    const std::string& pluginId,
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const wxPoint& pixel) const
{
    if (pluginId.empty() || !evalBoard)
    {
        return std::nullopt;
    }

    const std::vector<std::shared_ptr<PluginResult>> pluginsResults = evalBoard->assessments();

    if (pluginsResults.empty())
    {
        return std::nullopt;
    }

    for (const auto& a : pluginsResults)
    {
        if (a->plugin()->id() != pluginId)
            continue;

        const std::shared_ptr<PluginDebugResult>& deb = a->debugResult();

        if (!deb)
            return std::nullopt;

        if (pluginId == ContrastCheck::ID)
        {
            return getContrastCheckPixelTooltip(
                evalBoard,
                *std::static_pointer_cast<ConstrastCheckDebug>(deb),
                pixel
            );
        }
        else if (pluginId == FocusCheck::ID)
        {
            return getFocusCheckPixelTooltip(
                evalBoard,
                *std::static_pointer_cast<FocusCheckDebug>(deb),
                pixel
            );
        }
        else if (pluginId == IlluminationCheck::ID)
        {
            return getIlluminationCheckPixelTooltip(
                evalBoard,
                *std::static_pointer_cast<IlluminationCheckDebug>(deb),
                pixel
            );
        }
        else if (pluginId == OverexposureCheck::ID)
        {
            return getOverexposureCheckPixelTooltip(
                *std::static_pointer_cast<OverexposureCheckDebug>(deb),
                pixel
            );
        }
        else
        {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

std::optional<wxString> DetectionUtility::getPluginPixelTooltip(
    const std::string& pluginId,
    const std::shared_ptr<EvaluatedBoardSequence>& evalSequence,
    const wxPoint& pixel) const
{
    return std::nullopt;
}

std::optional<DetectionUtility::Mail> DetectionUtility::takeLatest()
{
    std::lock_guard<std::mutex> lock(latestMailMutex_);

    std::optional<Mail> tmp = std::move(latestMail_);

    latestMail_.reset();

    return tmp;
}

std::optional<DetectionUtility::Mail> DetectionUtility::takeHighPriority()
{
    std::lock_guard<std::mutex> lock(highPriorityMailMutex_);

    if (highPriorityMail_.empty())
    {
        return std::nullopt;
    }

    std::optional<DetectionUtility::Mail> tmp = std::move(highPriorityMail_.front());

    highPriorityMail_.pop();

    return tmp;
}

std::uint32_t DetectionUtility::getRunningCookTasks() const
{
    return cookTasks_.load();
}

bool DetectionUtility::highPriorityEmpty()
{
    std::lock_guard<std::mutex> lock(highPriorityMailMutex_);
    return highPriorityMail_.empty();
}

////////////////////////////////////////////////////////////////////////////////////////////

void DetectionUtility::addCookTask()
{
    ++cookTasks_;
}

void DetectionUtility::completeCookTask()
{
    if (cookTasks_.load() > 0)
        --cookTasks_;
}

void DetectionUtility::storeLatestBoardResult(const DetectionResultMap::Entry& payload)
{
    try
    {
        std::shared_ptr<DetectionResultView> boardResView = std::make_shared<DetectionResultView>(
            payload,
            convertBoardImageToWx(payload.second->evaluatedBoard()->object())
        );

        const bool isHighPriority = payload.second->source() != DetectionResult::Source::CAMERA_LIVE;

        if (isHighPriority)
        {
            addHighPriorityMail({ boardResView, nullptr });
        }
        else
        {
            setLatestMail({ boardResView, nullptr });
        }
    }
    catch (const std::exception& ex)
    {
        auto err = ex.what();
        bool b = false;
    }
    //catch (...)
    //{
    //    // TODO: attach logger
    //}
}

void DetectionUtility::storeLatestSequenceResults(const std::shared_ptr<EvaluatedBoardSequence>& payload)
{
    addHighPriorityMail({ nullptr, payload });
}

void DetectionUtility::setLatestMail(Mail data)
{
    std::lock_guard<std::mutex> lock(latestMailMutex_);
    latestMail_ = std::move(data);
}

void DetectionUtility::addHighPriorityMail(Mail data)
{
    std::lock_guard<std::mutex> lock(highPriorityMailMutex_);

    highPriorityMail_.push(data);
}

PluginDebugResultView DetectionUtility::buildContrastCheckResult(
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const ConstrastCheckDebug& pluginDebRes) const
{
    const std::vector<std::vector<CvContour>>& dilatedMarks = pluginDebRes.dilatedMarks();
    const std::vector<ConstrastCheckDebug::Mark>& marks = pluginDebRes.marks();
    const size_t n = dilatedMarks.size();

    CvImage debugImg(
        CvRegion::fromContours(dilatedMarks, true).toMask(evalBoard->object()->image().size())
    );
    debugImg = debugImg.toBGR();

    std::vector<size_t> compIdx(n);
    std::iota(compIdx.begin(), compIdx.end(), 0);

    std::for_each(std::execution::par, compIdx.begin(), compIdx.end(),
        [&](size_t i)
        {
            const ConstrastCheckDebug::Mark& mark = marks[i];

            std::optional<CvPainter::Colour> markColour;
            switch (mark.severity())
            {
                case EvaluationSeverity::CRITICAL:
                    markColour = { 0, 0, 255 };
                    break;

                case EvaluationSeverity::WARNING:
                    markColour = { 0, 165, 255 };
                    break;

                default:
                    markColour = std::nullopt;
                    break;
            }

            if (markColour.has_value())
            {
                // disjoint regions
                CvPainter(markColour.value())
                    .drawPoints(
                        debugImg,
                        CvRegion::fromContours(dilatedMarks[i], true).points()
                );
            }
        }
    );

    return PluginDebugResultView(
        std::string(ContrastCheck::ID),
        CvToWxImageConverter::toWxImage(debugImg)
    );
}

PluginDebugResultView DetectionUtility::buildFocusCheckResult(
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const FocusCheckDebug& pluginDebRes) const
{
    const std::vector<CvContour>& marksRegion = evalBoard->object()->marksContours();
    const std::vector<FocusCheckDebug::Mark>& marks = pluginDebRes.marks();
    const size_t n = marksRegion.size();

   CvImage debugImg = pluginDebRes.edgeImage().scaleImageMax().toBGR();

    std::vector<size_t> compIdx(n);
    std::iota(compIdx.begin(), compIdx.end(), 0);

    std::for_each(std::execution::par, compIdx.begin(), compIdx.end(),
        [&](size_t i)
        {
            const FocusCheckDebug::Mark& mark = marks[i];
            const std::vector<cv::Point2f>& markPts = marksRegion[i].points();

            std::optional<CvPainter::Colour> markColour;
            switch (mark.severity())
            {
            case EvaluationSeverity::CRITICAL:
                markColour = { 0, 0, 255 };
                break;

            case EvaluationSeverity::WARNING:
                markColour = { 0, 165, 255 };
                break;

            default:
                markColour = std::nullopt;
                break;
            }

            if (markColour.has_value())
            {
                // disjoint regions
                CvPainter(markColour.value())
                    .drawPoints(
                        debugImg,
                        CvRegion::fromContours({ marksRegion[i] }, false).points()
                );
            }
        }
    );

    return PluginDebugResultView(
        wxString(FocusCheck::ID.data()),
        CvToWxImageConverter::toWxImage(debugImg)
    );
}

PluginDebugResultView DetectionUtility::buildIlluminationCheckResult(
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const IlluminationCheckDebug& pluginDebRes) const
{
    const std::vector<std::vector<CvContour>>& dilatedMarks = pluginDebRes.dilatedMarks();
    const std::vector<IlluminationCheckDebug::Mark>& marks = pluginDebRes.marks();
    const size_t n = dilatedMarks.size();

    CvImage debugImg(
        CvRegion::fromContours(dilatedMarks, true).toMask(evalBoard->object()->image().size())
    );
    debugImg = debugImg.toBGR();

    std::vector<size_t> compIdx(n);
    std::iota(compIdx.begin(), compIdx.end(), 0);

    std::for_each(std::execution::par, compIdx.begin(), compIdx.end(),
        [&](size_t i)
        {
            const IlluminationCheckDebug::Mark& mark = marks[i];

            std::optional<CvPainter::Colour> markColour;
            switch (mark.severity())
            {
            case EvaluationSeverity::CRITICAL:
                markColour = { 0, 0, 255 };
                break;

            case EvaluationSeverity::WARNING:
                markColour = { 0, 165, 255 };
                break;

            default:
                markColour = std::nullopt;
                break;
            }

            if (markColour.has_value())
            {
                // disjoint regions
                CvPainter(markColour.value())
                    .drawPoints(
                        debugImg,
                        CvRegion::fromContours(dilatedMarks[i], true).points()
                );
            }
        }
    );

    return PluginDebugResultView(
        wxString(IlluminationCheck::ID.data()),
        CvToWxImageConverter::toWxImage(debugImg)
    );
}

PluginDebugResultView DetectionUtility::buildOverexposureCheckResult(
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const OverexposureCheckDebug& pluginDebRes) const
{
    const CvRegion& boardRegion = pluginDebRes.boardRegion();
    const CvRegion& overexpRegion = pluginDebRes.overexposedRegion();

    const std::vector<cv::Point>& overexpPts = overexpRegion.points();

    CvImage debugImg = evalBoard->object()->image().toBGR().clone();
    
    painter_->setColour({ 0, 255, 0})
        .drawContour(
            debugImg,
            evalBoard->object()->boardContour()
    );

    painter_->setColour({ 0, 0, 255 })
        .drawPoints(
            debugImg,
            overexpPts
    );

    return PluginDebugResultView(
        wxString(OverexposureCheck::ID.data()),
        CvToWxImageConverter::toWxImage(debugImg)
    );
}

PluginDebugResultView DetectionUtility::buildFOVCoverageCheckResult(
    const std::shared_ptr<EvaluatedBoardSequence>& evalSequence,
    const FOVCoverageCheckDebug& pluginDebRes) const
{
    CvImage LUTheatmap = pluginDebRes.heatmap();
    painter_->applyLUT(LUTheatmap);

    return PluginDebugResultView(
        wxString(FOVCoverageCheck::ID.data()),
        CvToWxImageConverter::toWxImage(LUTheatmap)
    );
}

std::optional<wxString> DetectionUtility::getContrastCheckPixelTooltip(
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const ConstrastCheckDebug& pluginDebRes,
    const wxPoint& pixel) const
{
    const std::vector<std::vector<CvContour>>& dilatedMarks = pluginDebRes.dilatedMarks();
    const std::vector<ConstrastCheckDebug::Mark>& marks = pluginDebRes.marks();
    const size_t n = dilatedMarks.size();

    std::vector<CvRegion> connectedComponents;
    connectedComponents.reserve(n);

    for (const auto& vec : dilatedMarks)
    {
        connectedComponents.push_back(CvRegion::fromContours(vec, true));
    }

    std::atomic<bool> ptFound{ false };
    std::optional<wxString> tip;

    std::vector<size_t> compIdx(n);
    std::iota(compIdx.begin(), compIdx.end(), 0);

    std::for_each(std::execution::par, compIdx.begin(), compIdx.end(),
        [&](size_t i)
        {
            const ConstrastCheckDebug::Mark& mark = marks[i];
            const std::vector<cv::Point>& markPts = connectedComponents[i].points();

            for (const auto& p : markPts)
            {
                if (ptFound.load())
                    break;

                if (p.x != pixel.x || p.y != pixel.y)
                    continue;

                ptFound.store(true);

                tip = wxString::Format(
                    "(X ; Y) %d, %d\n"
                    "--- Mark #%zu ---\n"
                    "Score: %.2f%%\n"
                    "Range: %.2f\n",
                    p.x, p.y,
                    i,
                    mark.score(), mark.range()
                );

                break;
            }
        }
    );

    return tip;
}

std::optional<wxString> DetectionUtility::getFocusCheckPixelTooltip(
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const FocusCheckDebug& pluginDebRes,
    const wxPoint& pixel) const
{
    const double contrast = pluginDebRes.contrast();
    const std::vector<CvContour>& marksContours = evalBoard->object()->marksContours();
    const std::vector<FocusCheckDebug::Mark>& marks = pluginDebRes.marks();
    const size_t n = marksContours.size();

    std::vector<CvContour> connectedContours;
    connectedContours.reserve(n);

    for (const auto& m : marksContours)
    {
        connectedContours.push_back(m);
    }

    std::vector<CvRegion> connectedComponents;
    connectedComponents.reserve(n);

    for (const auto& m : connectedContours)
    {
        connectedComponents.push_back(CvRegion::fromContours({ m }, false));
    }

    std::atomic<bool> ptFound{ false };
    std::optional<wxString> tip;

    std::vector<size_t> compIdx(n);
    std::iota(compIdx.begin(), compIdx.end(), 0);

    std::for_each(std::execution::par, compIdx.begin(), compIdx.end(),
        [&](size_t i)
        {
            const FocusCheckDebug::Mark& mark = marks[i];
            const std::vector<cv::Point>& markPts = connectedComponents[i].points();

            for (const auto& p : markPts)
            {
                if (ptFound.load())
                    break;

                if (p.x != pixel.x || p.y != pixel.y)
                    continue;

                ptFound.store(true);

                tip = wxString::Format(
                    "(X ; Y) %d, %d\n"
                    "--- Mark #%zu ---\n"
                    "Score: %.2f%%\n"
                    "Mean Gradient: %.2f\n"
                    "Normalized Gradient: %.2f\n"
                    "--- Global ---\n"
                    "Contrast: %.2f",
                    p.x, p.y,
                    i,
                    mark.score(),
                    mark.meanGradient(),
                    mark.normalizedGradient(),
                    contrast
                );

                break;
            }
        }
    );

    return tip;
}

std::optional<wxString> DetectionUtility::getIlluminationCheckPixelTooltip(
    const std::shared_ptr<EvaluatedBoard>& evalBoard,
    const IlluminationCheckDebug& pluginDebRes,
    const wxPoint& pixel) const
{
    const double mean = pluginDebRes.mean();
    const double stdDev = pluginDebRes.stdDev();
    const std::vector<std::vector<CvContour>>& dilatedMarks = pluginDebRes.dilatedMarks();
    const std::vector<IlluminationCheckDebug::Mark>& marks = pluginDebRes.marks();
    const size_t n = dilatedMarks.size();

    std::vector<CvRegion> connectedComponents;
    connectedComponents.reserve(n);

    for (const auto& vec : dilatedMarks)
    {
        connectedComponents.push_back(CvRegion::fromContours(vec, true));
    }

    std::atomic<bool> ptFound{ false };
    std::optional<wxString> tip;

    std::vector<size_t> compIdx(n);
    std::iota(compIdx.begin(), compIdx.end(), 0);

    std::for_each(std::execution::par, compIdx.begin(), compIdx.end(),
        [&](size_t i)
        {
            const IlluminationCheckDebug::Mark& mark = marks[i];
            const std::vector<cv::Point>& markPts = connectedComponents[i].points();

            for (const auto& p : markPts)
            {
                if (ptFound.load())
                    break;

                if (p.x != pixel.x || p.y != pixel.y)
                    continue;

                ptFound.store(true);

                tip = wxString::Format(
                    "(X ; Y) %d, %d\n"
                    "--- Mark #%zu ---\n"
                    "Score: %.2f%%\n"
                    "Max Gray Value: %.2f\n"
                    "Deviation: %.2f\n"
                    "--- Global ---\n"
                    "Mean: %.2f\n"
                    "StdDev: %.2f",
                    p.x, p.y,
                    i,
                    mark.score(),
                    mark.maxGrayValue(),
                    mark.deviation(),
                    mean, stdDev
                );

                break;
            }
        }
    );

    return tip;
}

std::optional<wxString> DetectionUtility::getOverexposureCheckPixelTooltip(
    const OverexposureCheckDebug& pluginDebRes,
    const wxPoint& pixel) const
{
    const CvRegion& boardRegion = pluginDebRes.boardRegion();
    const CvRegion& overexpRegion = pluginDebRes.overexposedRegion();
    const double boardArea = pluginDebRes.boardArea();
    const double areaOverexp = pluginDebRes.overexposedArea();
    const double currRatio = areaOverexp > 0.0 ? boardArea / areaOverexp : 0.0;

    std::atomic<bool> ptFound{ false };
    std::optional<wxString> tip;

    const auto& overexpPts = overexpRegion.points();

    std::for_each(std::execution::par, overexpPts.begin(), overexpPts.end(),
        [&](const cv::Point& p)
        {
            if (ptFound.load())
                return;

            if (p.x != pixel.x || p.y != pixel.y)
                return;

            ptFound.store(true);

            tip = wxString::Format(
                "(X ; Y) %d, %d\n"
                "*** OVEREXPOSED ***\n"
                "Plate area: %.1f\n"
                "Overexp area: %.1f\n"
                "Ratio: %.2f",
                p.x, p.y,
                boardArea,
                areaOverexp,
                currRatio
            );

            return;
        }
    );

    if (ptFound.load())
        return tip;

    const auto& boardPts = boardRegion.points();

    std::for_each(std::execution::par, boardPts.begin(), boardPts.end(),
        [&](const cv::Point& p)
        {
            if (ptFound.load())
                return;

            if (p.x != pixel.x || p.y != pixel.y)
                return;

            ptFound.store(true);

            tip = wxString::Format(
                "(X ; Y) %d, %d\n"
                "Plate area: %.1f\n"
                "Overexp area: %.1f\n"
                "Ratio: %.2f",
                p.x, p.y,
                boardArea,
                areaOverexp,
                currRatio
            );

            return;
        }
    );

    return tip;
}
