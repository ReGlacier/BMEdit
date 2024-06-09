#include "ui_SelectLocalizationTool.h"
#include <SelectLocalizationTool.h>
#include <Models/ModelsLocator.h>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QStringList>
#include <QPushButton>
#include <QString>


Frontend_SelectLocalizationTool::Frontend_SelectLocalizationTool(QWidget *parent, widgets::TypePropertyWidget* pTarget)
    : widgets::TypePropertyWidget(parent)
      , m_pTarget(pTarget)
{
	if (m_pTarget)
	{
		connect(m_pTarget, &widgets::TypePropertyWidget::valueChanged, this, &Frontend_SelectLocalizationTool::onTargetEditFinished);
		connect(m_pTarget, &widgets::TypePropertyWidget::editFinished, this, &Frontend_SelectLocalizationTool::onTargetValueChanged);

		m_pTarget->setWindowModality(Qt::WindowModality::ApplicationModal);
		m_pTarget->show();
	}

	// And build layout
	auto* pLayout = new QHBoxLayout(this);
	m_pLabel = new QLabel(this);
	m_pLabel->setText("MAYBE");
	pLayout->addWidget(m_pLabel);
	setLayout(pLayout);
}

Frontend_SelectLocalizationTool::~Frontend_SelectLocalizationTool()
{
	m_pTarget = nullptr;
}

void Frontend_SelectLocalizationTool::setValue(const types::QGlacierValue &value)
{
	if (m_pLabel && !value.instructions.empty() && value.instructions[0].isString())
	{
		m_pLabel->setText(QString::fromStdString(value.instructions[0].getOperand().str));
	}

	if (m_pTarget)
	{
		m_pTarget->setValue(value);
	}

	commitValue(value);
}

const types::QGlacierValue &Frontend_SelectLocalizationTool::getValue() const
{
	static const types::QGlacierValue s_Invalid {};
	if (!m_pTarget) return s_Invalid;
	return m_pTarget->getValue();
}

bool Frontend_SelectLocalizationTool::canHookFocus() const
{
	// Yep it can in this case
	return true;
}

void Frontend_SelectLocalizationTool::commitValue(const types::QGlacierValue &value)
{
	widgets::TypePropertyWidget::setValue(value);
}

void Frontend_SelectLocalizationTool::onTargetValueChanged()
{
	if (m_pTarget)
	{
		commitValue(m_pTarget->getValue());
		emit editFinished();
	}
}

void Frontend_SelectLocalizationTool::onTargetEditFinished()
{
	if (m_pTarget)
	{
		commitValue(m_pTarget->getValue());
		emit valueChanged();
	}
}

SelectLocalizationTool::SelectLocalizationTool(QWidget* parent) : widgets::TypePropertyWidget(parent), m_ui(new Ui::SelectLocalizationTool)
{
	m_ui->setupUi(this);

	// Set model
	m_ui->localizedStrings->setModel(models::ModelsLocator::s_LocalizationTreeModel.get());
	m_ui->localizedStrings->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

	// Connect signals
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, [this]() {
		emit editFinished();
		close();
	});

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &SelectLocalizationTool::onAccepted);
}

SelectLocalizationTool::~SelectLocalizationTool()
{
	delete m_ui;
}

Frontend_SelectLocalizationTool *SelectLocalizationTool::Create(QWidget *parent)
{
	auto*  pEditor = new SelectLocalizationTool(nullptr);
	auto*  pFrontend = new Frontend_SelectLocalizationTool(parent, pEditor);
	return pFrontend;
}

void SelectLocalizationTool::setValue(const types::QGlacierValue &value)
{
	// call for base
	widgets::TypePropertyWidget::setValue(value);

	if (value.instructions.size() == 1 && value.instructions[0].isString())
	{
		// Need to parse path
		selectByPath(QString::fromStdString(value.instructions[0].getOperand().str));
	}
}

void SelectLocalizationTool::disableAcceptButton()
{
	if (QPushButton* pOkButton = m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok))
	{
		pOkButton->setEnabled(false);
	}
}

void SelectLocalizationTool::enableAcceptButton()
{
	if (QPushButton* pOkButton = m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok))
	{
		pOkButton->setEnabled(true);
	}
}

void SelectLocalizationTool::selectByPath(const QString &path)
{
	QStringList pathParts = path.split('/');
	if (pathParts.empty()) return;

	if (pathParts[0] == "ROOT")
	{
		// remove ROOT because it literally does not exists :)
		pathParts.removeFirst();
	}

	auto* model = qobject_cast<models::LocalizationTreeModel*>(m_ui->localizedStrings->model());
	if (!model)
	{
		return;
	}

	QModelIndex currentIndex = QModelIndex();

	foreach (const QString& part, pathParts)
	{
		bool found = false;
		int rows = model->rowCount(currentIndex);

		for (int i = 0; i < rows; ++i)
		{
			QModelIndex childIndex = model->index(i, 0, currentIndex);
			QString entryName = model->data(childIndex, Qt::DisplayRole).toString();

			if (entryName == part)
			{
				currentIndex = childIndex;
				found = true;
				m_ui->localizedStrings->expand(currentIndex);
				m_ui->localizedStrings->scrollTo(currentIndex);
				break;
			}
		}

		if (!found)
		{
			return;
		}
	}

	m_ui->localizedStrings->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void SelectLocalizationTool::onAccepted()
{
	// need to set value
	const QModelIndexList selectedIndexes = m_ui->localizedStrings->selectionModel()->selectedIndexes();
	if (selectedIndexes.isEmpty())
	{
		// just do nothing
		emit editFinished();
		return;
	}

	QModelIndex currentIndex = selectedIndexes.first();
	QStringList pathParts {};
	while (currentIndex.isValid())
	{
		pathParts.prepend(currentIndex.data().toString());
		currentIndex = currentIndex.parent();
	}

	const auto fullPath = pathParts.join('/').toStdString();
	auto newVal = getValue();
	if (!newVal.instructions.empty())
	{
		// weird but ok
		newVal.instructions[0] = gamelib::prp::PRPInstruction(newVal.instructions[0].getOpCode(), gamelib::prp::PRPOperandVal(fullPath));

		// use base to avoid of extra selector iteration
		widgets::TypePropertyWidget::setValue(newVal);
		emit editFinished();
	}

	// and close us
	close();
}

void SelectLocalizationTool::onRejected()
{
	emit editFinished();
	close();
}

void SelectLocalizationTool::onLocaleSelected(const QItemSelection &selected, const QItemSelection &deselected)
{
	if (selected.empty())
	{
		disableAcceptButton();
		return;
	}

	QModelIndex index = selected.first().indexes()[0];
	auto* node = reinterpret_cast<gamelib::loc::LOCTreeNode*>(index.internalPointer());

	if (!node)
	{
		disableAcceptButton();
	}
	else
	{
		types::QGlacierValue temp = getValue();
		if (temp.instructions.empty())
		{
			disableAcceptButton();
		}
		else
		{
			enableAcceptButton();
		}
	}
}

void SelectLocalizationTool::closeEvent(QCloseEvent *pEvent)
{
	emit editFinished();
	QWidget::closeEvent(pEvent);
}