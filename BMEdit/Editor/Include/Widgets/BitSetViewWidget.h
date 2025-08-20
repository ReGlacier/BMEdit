#pragma once

#include <QScopedPointer>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QWidget>
#include <vector>


namespace widgets
{
	class BitSetViewWidget : public QWidget
	{
		Q_OBJECT
	public:
		using Bits = std::vector<std::pair<QString, uint8_t>>; // Bit name to bit index

		BitSetViewWidget(QWidget* parent = nullptr);

		void setPossibleValues(const Bits& values);

		[[nodiscard]] const Bits& getPossibleValues() const;
		[[nodiscard]] Bits getChecked() const;
		[[nodiscard]] Bits getUnchecked() const;

		void setValue(uint32_t value);
		void resetValue();
		[[nodiscard]] uint32_t getValue() const;

		void reset();

	signals:
		void valueChanged(uint32_t value);

	private:
		void clearView();
		void buildView();
		void updateView();

	private:
		uint32_t m_intRepr { 0u }; // it's 32, for 64 write new widget please
		Bits m_allowedValues {};
		QScopedPointer<QVBoxLayout> m_pLayout { nullptr };
		std::vector<std::pair<QString, QCheckBox*>> m_bitNrToCheckBox {};
	};
}