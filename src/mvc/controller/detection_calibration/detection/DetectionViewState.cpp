#include "DetectionViewState.hpp"


void DetectionViewState::UnsetCurrentRenderedBoard()
{
	if (m_renderedBoardId.has_value() && m_liveFrameId.has_value() &&
		m_renderedBoardId.value().getValue() == m_liveFrameId.value().getValue())
	{
		m_liveFrameId.reset();
	}

	m_renderedBoardId.reset();
}

void DetectionViewState::SetCurrentRenderedBoard(DetectionResult::Id resultId)
{
	m_renderedBoardId = resultId;
}

std::optional<DetectionResult::Id> DetectionViewState::GetCurrentRenderedBoard() const
{
	return m_renderedBoardId;
}

void DetectionViewState::UnsetCurrentLiveFrame()
{
	if (m_renderedBoardId.has_value() && m_liveFrameId.has_value() &&
		m_renderedBoardId.value().getValue() == m_liveFrameId.value().getValue())
	{
		m_renderedBoardId.reset();
	}

	m_liveFrameId.reset();
}

void DetectionViewState::SetCurrentLiveFrame(DetectionResult::Id frameId)
{
	m_liveFrameId = frameId;
}

std::optional<DetectionResult::Id> DetectionViewState::GetCurrentLiveFrame() const
{
	return m_liveFrameId;
}