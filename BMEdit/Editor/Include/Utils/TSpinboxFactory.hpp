#pragma once

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLayout>
#include <QWidget>

#include <type_traits>
#include <functional>

namespace utils
{
	template <typename T>
	struct TSpinboxFactory
	{
		template <bool Condition, typename TTrue, typename TFalse> struct TIfCondition;

		template <typename TTrue, typename TFalse> struct TIfCondition<true, TTrue, TFalse> {
			using Result = TTrue;
		};

		template <typename TTrue, typename TFalse> struct TIfCondition<false, TTrue, TFalse> {
			using Result = TFalse;
		};

		using TSpinWidget = typename TIfCondition<std::is_same_v<T, float> || std::is_same_v<T, double>, QDoubleSpinBox, QSpinBox>::Result;
		using TValueType = typename TIfCondition<std::is_same_v<T, float> || std::is_same_v<T, double>, double, int>::Result;

		static void createAndSetup(const QString& accessibleName, const types::QGlacierValue &value, int instructionIdx, QWidget* parent, QLayout* layout, const std::function<void(int, T)>& onDataChanged)
		{
			auto widget = create(accessibleName, value, instructionIdx, parent, onDataChanged);
			if (widget)
			{
				layout->addWidget(widget);
			}
		}

		static QWidget* create(const QString& accessibleName, const types::QGlacierValue &value, int instructionIdx, QWidget* parent, const std::function<void(int, T)>& onDataChanged)
		{
			auto widget = new TSpinWidget(parent);
			widget->setMinimum(std::numeric_limits<T>::lowest());
			widget->setMaximum(std::numeric_limits<T>::max());
			widget->setAccessibleName(accessibleName);
			widget->setValue(value.instructions[instructionIdx].getOperand().get<T>());

			parent->connect(widget, &TSpinWidget::valueChanged, [instructionIdx, onDataChanged](TValueType newValue) {
				onDataChanged(instructionIdx, static_cast<T>(newValue));
			});

			return widget;
		}

		static void updateValue(const QString& widgetName, const types::QGlacierValue &value, int valueIdx, QWidget *parent)
		{
			if (auto widget = parent->template findChild<TSpinWidget*>(widgetName))
			{
				QSignalBlocker blocker(widget);

				widget->setValue(value.instructions[valueIdx].getOperand().get<T>());
			}
		}
	};
}