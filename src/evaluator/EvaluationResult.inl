#pragma once
#include "EvaluationResult.hpp"


template<typename T>
EvaluationResult<T>::EvaluationResult(
    const T& object,
    std::vector<std::shared_ptr<PluginResult>> assessments)
	: object_(object),
    assessments_(std::move(assessments))
{
    init();
}

///////////////////////////////////////////////////////////////////////

template<typename T>
const T& EvaluationResult<T>::object() const
{
    return object_;
}

template<typename T>
const std::vector<std::shared_ptr<PluginResult>>& EvaluationResult<T>::assessments() const
{
    return assessments_;
}

template<typename T>
EvaluationResult<T>::Status EvaluationResult<T>::status() const
{
    return status_;
}

///////////////////////////////////////////////////////////////////////

template<typename T>
void EvaluationResult<T>::init()
{
    if (assessments_.empty())
    {
        status_ = Status::NOT_EVALUATED;
        return;
	}

    for (const auto& a : assessments_)
    {
        switch (a->severity())
        {
		    case EvaluationSeverity::OK:
                continue;

		    case EvaluationSeverity::WARNING:
		    case EvaluationSeverity::CRITICAL:
            case EvaluationSeverity::FAILED:
                status_ = Status::HAS_ISSUES;
			    return;

            default:
                continue;
        }
    }

	status_ = Status::GOOD;
}