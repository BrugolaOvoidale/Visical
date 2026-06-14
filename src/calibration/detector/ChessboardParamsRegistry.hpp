#pragma once
#include <parameter/ParameterRegistry.hpp>


/**
 * @brief Registry for managing detection parameters for chessboard pattern.
 */
class ChessboardParamsRegistry : public ParameterRegistry
{
public:
	static const std::string CATEGORY;

	ChessboardParamsRegistry();

	~ChessboardParamsRegistry() = default;

private:
	void registerAllParameters();
};