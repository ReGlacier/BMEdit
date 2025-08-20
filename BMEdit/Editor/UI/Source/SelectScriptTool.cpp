#include <Models/GameScriptsTreeModel.h>
#include <Models/ModelsLocator.h>
#include <SelectScriptTool.h>
#include <QPushButton>
#include "ui_SelectScriptTool.h"


Frontend_SelectScriptTool::Frontend_SelectScriptTool(QWidget *parent, widgets::TypePropertyWidget *pTarget)
	: widgets::TypePropertyWidget(parent)
	, m_pTarget(pTarget)
{
	if (m_pTarget)
	{
		connect(m_pTarget, &widgets::TypePropertyWidget::valueChanged, this, &Frontend_SelectScriptTool::onTargetValueChanged);
		connect(m_pTarget, &widgets::TypePropertyWidget::editFinished, this, &Frontend_SelectScriptTool::onTargetEditFinished);

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

Frontend_SelectScriptTool::~Frontend_SelectScriptTool()
{
	m_pTarget = nullptr;
}

void Frontend_SelectScriptTool::onTargetValueChanged()
{
	if (m_pTarget)
	{
		commitValue(m_pTarget->getValue());
		emit valueChanged();
	}
}

void Frontend_SelectScriptTool::onTargetEditFinished()
{
	if (m_pTarget)
	{
		commitValue(m_pTarget->getValue());
		emit editFinished();
	}
}

void Frontend_SelectScriptTool::setValue(const types::QGlacierValue &value)
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

const types::QGlacierValue &Frontend_SelectScriptTool::getValue() const
{
	static const types::QGlacierValue s_Invalid {};
	if (!m_pTarget) return s_Invalid;
	return m_pTarget->getValue();
}

bool Frontend_SelectScriptTool::canHookFocus() const
{
	// Yep it can in this case
	return true;
}

void Frontend_SelectScriptTool::commitValue(const types::QGlacierValue &value)
{
	widgets::TypePropertyWidget::setValue(value);
}


SelectScriptTool::SelectScriptTool(QWidget *parent)
    : widgets::TypePropertyWidget(parent)
    , m_ui(new Ui::SelectScriptTool)
{
	m_ui->setupUi(this);

	disableAcceptButton();

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &SelectScriptTool::onAccepted);
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &SelectScriptTool::onRejected);

	// assign model and connect signals
	m_ui->gameScripts->setModel(models::ModelsLocator::s_GameScriptsTreeModel.get());
	m_ui->gameScripts->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	connect(m_ui->gameScripts->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SelectScriptTool::onScriptSelected);
}

SelectScriptTool::~SelectScriptTool()
{
	delete m_ui;
}

void SelectScriptTool::closeEvent(QCloseEvent *pEvent)
{
	emit editFinished();
	QWidget::closeEvent(pEvent);
}

void SelectScriptTool::disableAcceptButton()
{
	if (QPushButton* pOkButton = m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok))
	{
		pOkButton->setEnabled(false);
	}
}

void SelectScriptTool::enableAcceptButton()
{
	if (QPushButton* pOkButton = m_ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok))
	{
		pOkButton->setEnabled(true);
	}
}

void SelectScriptTool::selectByPath(const QString &path)
{
	QSignalBlocker blocker { m_ui->gameScripts->selectionModel() };

	QStringList pathParts = path.split('\\');
	if (pathParts.empty()) return;

	auto* model = qobject_cast<models::GameScriptsTreeModel*>(m_ui->gameScripts->model());
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
				m_ui->gameScripts->expand(currentIndex);
				m_ui->gameScripts->scrollTo(currentIndex);
				break;
			}
		}

		if (!found)
		{
			return;
		}
	}

	m_ui->gameScripts->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void SelectScriptTool::onAccepted()
{
	// need to set value
	const QModelIndexList selectedIndexes = m_ui->gameScripts->selectionModel()->selectedIndexes();
	if (selectedIndexes.isEmpty())
	{
		// just do nothing
		return;
	}

	QModelIndex currentIndex = selectedIndexes.first();
	auto* pScript = reinterpret_cast<models::GameScriptsTreeModel::ScripTreeNode*>(currentIndex.internalPointer());
	if (!pScript || pScript->type != models::GameScriptsTreeModel::ScripTreeNode::NodeType::STN_SCRIPT || m_value.instructions.empty())
	{
		// Just do nothing
		return;
	}

	auto newVal = getValue();
	newVal.instructions[0] = gamelib::prp::PRPInstruction(newVal.instructions[0].getOpCode(), gamelib::prp::PRPOperandVal(pScript->fullPath.toStdString()));
	widgets::TypePropertyWidget::setValue(newVal);
	emit editFinished();

	close();
}

void SelectScriptTool::onRejected()
{
	emit editFinished();
	close();
}

void SelectScriptTool::onScriptSelected(const QItemSelection &selected, const QItemSelection &deselected)
{
	if (selected.empty())
	{
		disableAcceptButton();
		return;
	}

	QModelIndex index = selected.first().indexes()[0];
	auto* pScript = reinterpret_cast<models::GameScriptsTreeModel::ScripTreeNode*>(index.internalPointer());

	if (!pScript || pScript->type != models::GameScriptsTreeModel::ScripTreeNode::NodeType::STN_SCRIPT)
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

void SelectScriptTool::setValue(const types::QGlacierValue &value)
{
	// call for base
	widgets::TypePropertyWidget::setValue(value);

	if (value.instructions.size() == 1 && value.instructions[0].isString())
	{
		// Need to parse path
		selectByPath(QString::fromStdString(value.instructions[0].getOperand().str));
	}
}

Frontend_SelectScriptTool *SelectScriptTool::Create(QWidget *parent)
{
	auto*  pEditor = new SelectScriptTool(nullptr);
	auto*  pFrontend = new Frontend_SelectScriptTool(parent, pEditor);
	return pFrontend;
}