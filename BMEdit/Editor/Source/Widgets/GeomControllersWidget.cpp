#include "ui_GeomControllersWidget.h"
#include <Widgets/GeomControllersWidget.h>
#include <Delegates/TypePropertyItemDelegate.h>
#include <Models/SceneObjectControllerModel.h>

using namespace widgets;


GeomControllersWidget::GeomControllersWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::GeomControllersWidget())
    , m_controllersListModel(new QStringListModel(this))
    , m_controllerPropertiesModel(new models::SceneObjectControllerModel(this))
	, m_controllerEditorDelegate(new delegates::TypePropertyItemDelegate(this))
{
	m_ui->setupUi(this);

	setup();
	switchToDefaults();
}

GeomControllersWidget::~GeomControllersWidget()
{
	delete m_controllerPropertiesModel;
	delete m_controllerEditorDelegate;
	delete m_controllersListModel;

	delete m_ui;
}

void GeomControllersWidget::setGeom(gamelib::scene::SceneObject *sceneObject)
{
	assert(sceneObject != nullptr);

	if (sceneObject && sceneObject != m_sceneObject)
	{
		if (!sceneObject->getControllers().empty())
		{
			QSignalBlocker blocker(m_ui->controllerSelector);

			// Load possible controllers
			QStringList controllers;
			controllers.reserve(static_cast<qsizetype>(sceneObject->getControllers().size()));

			for (const auto& [controllerName, _controllerDecl]: sceneObject->getControllers())
			{
				controllers.push_back(QString::fromStdString(controllerName));
			}
			m_controllersListModel->setStringList(controllers);

			// Select controller
			m_ui->controllerSelector->setEnabled(true);
		}
		else
		{
			m_controllersListModel->setStringList({});
			m_ui->controllerSelector->setEnabled(false);
		}

		m_controllerPropertiesModel->setGeom(sceneObject);

		m_sceneObject = sceneObject;
		m_currentController = {};
		emit geomChanged();
	}
}

void GeomControllersWidget::resetGeom()
{
	switchToDefaults();
}

const gamelib::scene::SceneObject* GeomControllersWidget::getGeom() const
{
	return m_sceneObject;
}

void GeomControllersWidget::setController(const QString &controllerName)
{
	if (controllerName.isEmpty())
	{
		return;
	}

	m_currentController = controllerName;

	{
		QSignalBlocker selectorBlocker(m_ui->controllerSelector);

		m_ui->controllerSelector->setCurrentText(controllerName);
		m_controllerPropertiesModel->setControllerName(controllerName.toStdString());
	}

	emit controllerSelectionChanged(m_currentController);
}

const QString &GeomControllersWidget::getController() const
{
	return m_currentController;
}

void GeomControllersWidget::switchToDefaults()
{
	if (!m_currentController.isEmpty())
	{
		m_controllerPropertiesModel->resetValue();
	}
	m_currentController = {};
	m_sceneObject = nullptr;

	m_ui->editControllerListButton->setEnabled(false);
	m_ui->controllerSelector->setEnabled(false);
	m_ui->controllerSelector->clear();
}

void GeomControllersWidget::switchToFirstController()
{
	if (!m_sceneObject || m_sceneObject->getControllers().empty())
	{
		return;
	}

	setController(QString::fromStdString(m_sceneObject->getControllers().begin()->first));
	m_ui->controllerSelector->setCurrentText(m_currentController);
}

void GeomControllersWidget::setup()
{
	m_ui->controllerSelector->setModel(m_controllersListModel);

	m_ui->controllerProperties->setModel(m_controllerPropertiesModel);
	m_ui->controllerProperties->setItemDelegateForColumn(1, m_controllerEditorDelegate);
	m_ui->controllerProperties->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_ui->controllerProperties->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	connect(m_ui->controllerSelector, &QComboBox::currentTextChanged, [=](const QString &nextControllerName) {
		setController(nextControllerName);
	});
}