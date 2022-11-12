#include "ui_GeomControllersWidget.h"
#include <Widgets/GeomControllersWidget.h>
#include <Delegates/TypePropertyItemDelegate.h>
#include <Models/SceneObjectControllerModel.h>
#include <Models/GeomControllerListModel.h>

using namespace widgets;


GeomControllersWidget::GeomControllersWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::GeomControllersWidget())
    , m_geomControllersListModel(new models::GeomControllerListModel(this))
    , m_controllerPropertiesModel(new models::SceneObjectControllerModel(this))
	, m_controllerEditorDelegate(new delegates::TypePropertyItemDelegate(this))
{
	m_ui->setupUi(this);

	setup();
	switchToDefaults();
}

GeomControllersWidget::~GeomControllersWidget()
{
	delete m_geomControllersListModel;
	delete m_controllerPropertiesModel;
	delete m_controllerEditorDelegate;

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
			m_geomControllersListModel->setGeom(sceneObject);

			// Select controller
			m_ui->controllerSelector->setEnabled(true);
		}
		else
		{
			m_geomControllersListModel->resetGeom();
			m_ui->controllerSelector->setEnabled(false);
		}

		m_controllerPropertiesModel->setGeom(sceneObject);

		m_sceneObject = sceneObject;
		emit geomChanged();
	}
}

void GeomControllersWidget::resetGeom()
{
	switchToDefaults();

	emit geomReset();
}

const gamelib::scene::SceneObject* GeomControllersWidget::getGeom() const
{
	return m_sceneObject;
}

void GeomControllersWidget::setController(int controllerIndex)
{
	if (!m_sceneObject || m_sceneObject->getControllers().size() <= controllerIndex)
	{
		return;
	}

	{
		QSignalBlocker blocker { m_ui->controllerSelector };
		m_ui->controllerSelector->setCurrentIndex(controllerIndex);
	}

	m_controllerPropertiesModel->setControllerIndex(controllerIndex);
}

void GeomControllersWidget::switchToDefaults()
{
	m_controllerPropertiesModel->resetValue();
	m_geomControllersListModel->resetGeom();
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

	setController(0);
}

void GeomControllersWidget::setup()
{
	m_ui->controllerSelector->setModel(m_geomControllersListModel);

	m_ui->controllerProperties->setModel(m_controllerPropertiesModel);
	m_ui->controllerProperties->setItemDelegateForColumn(1, m_controllerEditorDelegate);
	m_ui->controllerProperties->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	m_ui->controllerProperties->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);

	connect(m_ui->controllerSelector, &QComboBox::currentIndexChanged, [=](int newIndex) {
		if (newIndex < 0)
		{
			switchToDefaults();
		}
		else
		{
			setController(newIndex);
		}
	});
}