#pragma once

#include <Widgets/TypePropertyWidget.h>
#include <QWidget>


namespace widgets
{
	struct EditorToolFactory
	{
		static TypePropertyWidget* createToolById(QWidget* parent, const std::string &hintId);
	};
}