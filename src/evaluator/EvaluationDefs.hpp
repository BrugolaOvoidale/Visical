#pragma once


// This is a general purpose evaluation level
// used for plugin-level results and any intermediate diagnostic annotations
enum class EvaluationSeverity {
    OK,
    WARNING,
    CRITICAL,
    FAILED,
    INSUFFICIENT_DATA
};