#include <Widgets/BitSetViewWidget.h>
#include <QString>


namespace widgets
{
	void clearLayout(QLayout* pLayout) // NOLINT(*-no-recursion)
	{
		if (!pLayout)
			return;

		QLayoutItem* pItem = nullptr;

		while ((pItem = pLayout->takeAt(0)))
		{
			if (pItem->layout())
			{
				clearLayout(pItem->layout());
				delete pItem->layout();
			}

			if (pItem->widget())
			{
				delete pItem->widget();
			}

			delete pItem;
		}
	}

	BitSetViewWidget::BitSetViewWidget(QWidget *parent) : QWidget(parent)
	{
		m_pLayout.reset(new QVBoxLayout(this));
		setLayout(m_pLayout.get());
	}

	void BitSetViewWidget::setPossibleValues(const BitSetViewWidget::Bits &values)
	{
		m_allowedValues = values;
		m_intRepr = 0;
		m_bitNrToCheckBox.clear();

#ifdef QT_DEBUG
		// static check
		{
			QSet<QString> knowUrName;

			for (const auto& [name, _] : values)
			{
				if (knowUrName.contains(name))
				{
					Q_ASSERT_X(false, __FILE__, "KEY DUPLICATE!!!");
					return;
				}

				// store
				knowUrName.insert(name);
			}
		}
#endif

		// rebuild view
		if (!m_allowedValues.empty())
		{
			clearView();
		}

		buildView();
		// no need to call updateView here because no value - no view
	}

	const BitSetViewWidget::Bits& BitSetViewWidget::getPossibleValues() const
	{
		return m_allowedValues;
	}

	BitSetViewWidget::Bits BitSetViewWidget::getChecked() const
	{
		Bits r {};

		for (const auto& [name, idx] : m_allowedValues)
		{
			if ((m_intRepr & (1 << idx)) != 0)
			{
				r.emplace_back(name, idx);
			}
		}

		return r;
	}

	BitSetViewWidget::Bits BitSetViewWidget::getUnchecked() const
	{
		Bits r {};

		for (const auto& [name, idx] : m_allowedValues)
		{
			if ((m_intRepr & (1 << idx)) == 0)
			{
				r.emplace_back(name, idx);
			}
		}

		return r;
	}

	void BitSetViewWidget::setValue(uint32_t value)
	{
		if (value != m_intRepr)
		{
			m_intRepr = value;
			updateView();

			emit valueChanged(m_intRepr);
		}
	}

	void BitSetViewWidget::resetValue()
	{
		setValue(0u);
	}

	uint32_t BitSetViewWidget::getValue() const
	{
		return m_intRepr;
	}

	void BitSetViewWidget::reset()
	{
		m_allowedValues.clear();
		m_bitNrToCheckBox.clear();
		clearView();
	}

	void BitSetViewWidget::clearView()
	{
		clearLayout(m_pLayout.get());
	}

	void BitSetViewWidget::buildView()
	{
		for (const auto& [name, bitIdx] : m_allowedValues)
		{
			auto* pCheckBox = new QCheckBox(name, this);
			m_bitNrToCheckBox.emplace_back(name, pCheckBox);
			m_pLayout->addWidget(pCheckBox);

			connect(pCheckBox, &QCheckBox::toggled, [this, bitIdx](bool checked) {
				const auto oldValue = m_intRepr;
				if (checked)
				{
					m_intRepr |= (1 << bitIdx);
				}
				else
				{
					m_intRepr &= ~(1 << bitIdx);
				}

				if (oldValue != m_intRepr)
				{
					emit valueChanged(m_intRepr);
				}
			});
		}
	}

	void BitSetViewWidget::updateView()
	{
		int index = 0;
		for (const auto& [name, pCurrentCheckBox] : m_bitNrToCheckBox)
		{
			if (!pCurrentCheckBox)
			{
				Q_ASSERT(pCurrentCheckBox != nullptr);
				++index;
				continue;
			}

			{
				QSignalBlocker blocker { pCurrentCheckBox };
				const auto bitIdx = m_allowedValues[index].second;

				const bool bExpectedValue = static_cast<bool>((m_intRepr & (1 << bitIdx)));
				pCurrentCheckBox->setChecked(bExpectedValue);
			}

			++index;
		}
	}
}