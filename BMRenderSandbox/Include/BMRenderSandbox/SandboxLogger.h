#pragma once

#include <BMRender/ILogInterface.h>


class SandboxLogger : public bmr::ILogInterface
{
public:
	void logMessage(bmr::Level level, std::string &&message) override;
};