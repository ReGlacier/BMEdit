#include "ui_SelectSceneObjectTool.h"
#include <SelectSceneObjectTool.h>
#include <Models/ModelsLocator.h>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QStringList>
#include <QString>


Frontend_SelectSceneObjectTool::Frontend_SelectSceneObjectTool(QWidget *parent, widgets::TypePropertyWidget* pTarget)
    : widgets::TypePropertyWidget(parent)
	, m_pTarget(pTarget)
{
	if (m_pTarget)
	{
		connect(m_pTarget, &widgets::TypePropertyWidget::valueChanged, [this]() {
			commitValue(m_pTarget->getValue());
			emit valueChanged();
		});

		connect(m_pTarget, &widgets::TypePropertyWidget::editFinished, [this]() {
			auto v = m_pTarget->getValue();
			commitValue(v);
			emit editFinished();
		});

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

void Frontend_SelectSceneObjectTool::setValue(const types::QGlacierValue &value)
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

const types::QGlacierValue &Frontend_SelectSceneObjectTool::getValue() const
{
	static const types::QGlacierValue s_Invalid {};
	if (!m_pTarget) return s_Invalid;
	return m_pTarget->getValue();
}

bool Frontend_SelectSceneObjectTool::canHookFocus() const
{
	// Yep it can in this case
	return true;
}

void Frontend_SelectSceneObjectTool::commitValue(const types::QGlacierValue &value)
{
	widgets::TypePropertyWidget::setValue(value);
}


SelectSceneObjectTool::SelectSceneObjectTool(QWidget* parent) : widgets::TypePropertyWidget(parent), m_ui(new Ui::SelectSceneObjectTool)
{
	m_ui->setupUi(this);

	// Set model
	m_ui->objectsTree->setModel(models::ModelsLocator::s_SceneTreeModel.get());
	m_ui->objectsTree->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

	// Connect signals
	connect(m_ui->buttonBox, &QDialogButtonBox::rejected, [this]() {
		emit editFinished();
		close();
	});

	connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &SelectSceneObjectTool::onAccepted);
}

SelectSceneObjectTool::~SelectSceneObjectTool()
{
	delete m_ui;
}

Frontend_SelectSceneObjectTool *SelectSceneObjectTool::Create(QWidget *parent)
{
	auto*  pEditor = new SelectSceneObjectTool(nullptr);
	auto*  pFrontend = new Frontend_SelectSceneObjectTool(parent, pEditor);
	return pFrontend;
}

void SelectSceneObjectTool::setValue(const types::QGlacierValue &value)
{
	// call for base
	widgets::TypePropertyWidget::setValue(value);

	if (value.instructions.size() == 1 && value.instructions[0].isString())
	{
		// Need to parse path
		selectByPath(QString::fromStdString(value.instructions[0].getOperand().str));
	}
}

void SelectSceneObjectTool::selectByPath(const QString &path)
{
	QStringList pathParts = path.split('\\');
	if (pathParts.empty()) return;

	if (pathParts[0] == "ROOT")
	{
		// remove ROOT because it literally does not exists :)
		pathParts.removeFirst();
	}

	auto* model = qobject_cast<models::SceneObjectsTreeModel*>(m_ui->objectsTree->model());
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
				m_ui->objectsTree->expand(currentIndex);
				m_ui->objectsTree->scrollTo(currentIndex);
				break;
			}
		}

		if (!found)
		{
			return;
		}
	}

	m_ui->objectsTree->selectionModel()->select(currentIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void SelectSceneObjectTool::onAccepted()
{
	// need to set value
	const QModelIndexList selectedIndexes = m_ui->objectsTree->selectionModel()->selectedIndexes();
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

	// Always include ROOT subject here
	pathParts.prepend("ROOT");

	const auto fullPath = pathParts.join('\\').toStdString();
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

void SelectSceneObjectTool::closeEvent(QCloseEvent *pEvent)
{
	emit editFinished();
	QWidget::closeEvent(pEvent);
}