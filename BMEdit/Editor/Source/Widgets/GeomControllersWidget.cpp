#include "ui_GeomControllersWidget.h"
#include <Widgets/GeomControllersWidget.h>
#include <Delegates/TypePropertyItemDelegate.h>
#include <Models/SceneObjectControllerModel.h>
#include <Models/GeomControllerListModel.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/TypeKind.h>
#include <QMenu>


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
		{
			// Filter allTypes
			QSignalBlocker blocker(m_ui->controllerSelector);

			// Load possible controllers
			m_geomControllersListModel->setGeom(sceneObject);

			//
			if (!sceneObject->getControllers().empty())
			{
				// Select controller
				m_ui->controllerSelector->setEnabled(true);
				m_ui->removeControllerButton->setEnabled(true);
			}
			else
			{
				m_ui->controllerSelector->setEnabled(false);
				m_ui->removeControllerButton->setEnabled(false);
			}
		}

		m_controllerPropertiesModel->setGeom(sceneObject);

		m_sceneObject = sceneObject;
		updateAvailableControllersList();

		emit geomChanged();
	}
}

void GeomControllersWidget::resetGeom()
{
	switchToDefaults();

	// Discard geom
	m_geomControllersListModel->resetGeom();
	m_controllerPropertiesModel->resetGeom();

	// Discard current geom
	m_sceneObject = nullptr;

	// Clear available controller type names
	m_availableToAddControllersModel->setStringList({});

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

	// new controllers menu
	m_availableToAddControllersModel.reset(new QStringListModel(this));

	m_availableToAddControllersProxyModel.reset(new QSortFilterProxyModel(this));
	m_availableToAddControllersProxyModel->setSourceModel(m_availableToAddControllersModel.get());

	m_ui->allPossibleControllersList->setModel(m_availableToAddControllersProxyModel.get());

	connect(m_ui->addSelectedControllerButton, &QPushButton::clicked, [=]() {
		const auto selectedRows = m_ui->allPossibleControllersList->selectionModel()->selectedRows();
		if (selectedRows.isEmpty())
			return;

		addControllerToGeom(m_ui->allPossibleControllersList->model()->data(selectedRows[0], Qt::DisplayRole).value<QString>());
	});

	connect(m_ui->searchEdit, &QLineEdit::textChanged, [this](const QString& newQuery) {
		m_availableToAddControllersProxyModel->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
		m_availableToAddControllersProxyModel->setFilterFixedString(newQuery);
	});

	connect(m_ui->controllerSelector, &QComboBox::currentIndexChanged, [this](int newIndex) {
		if (newIndex < 0)
		{
			switchToDefaults();
		}
		else
		{
			setController(newIndex);
		}
	});

	connect(m_ui->removeControllerButton, &QPushButton::clicked, [this]() {
		removeCurrentController();
	});
}

void GeomControllersWidget::updateAvailableControllersList()
{
	auto allTypes = getAllPossibleControllerNamesFromTypesDb();
	m_availableToAddControllersModel->setStringList(allTypes);
}

void GeomControllersWidget::addControllerToGeom(const QString& controllerName)
{
	if (auto pType = gamelib::TypeRegistry::getInstance().findTypeByShortName(controllerName.toStdString()))
	{
		auto serializeControllerName = [](const std::string& controllerName) -> std::string
		{
			if (controllerName.empty()) return controllerName;

			std::string newName { controllerName };
			// See TypeRegistry::findTypeByShortName for details
			if (newName[0] == 'Z' || newName[0] == 'C')
			{
				newName.erase(newName.begin(), newName.begin() + 1);
			}

			if (newName.starts_with("HM3"))
			{
				newName.erase(newName.begin(), newName.begin() + 3);
			}

			if (auto it = newName.find("Event"); it != std::string::npos)
			{
				newName.erase(it, 5);
			}

			return newName;
		};

		const bool bAddedFirstController = m_sceneObject->getControllers().empty();

		// Register a new controller
		gamelib::scene::SceneObject::Controller& newController = m_sceneObject->getControllers().emplace_back();
		newController.name = serializeControllerName(pType->getName());
		newController.properties = pType->makeDefaultPropertiesPack();

		// Update UI
		updateAvailableControllersList();

		// Make it selectable
		m_ui->controllerSelector->setEnabled(true);
		m_ui->removeControllerButton->setEnabled(true);

		// Load possible controllers
		m_geomControllersListModel->updateControllersList();

		if (bAddedFirstController)
		{
			switchToFirstController();
		}
	}
}

void GeomControllersWidget::removeCurrentController()
{
	const int indexToRemove = m_ui->controllerSelector->currentIndex();
	if (indexToRemove < 0)
		return;

	// Remove current controller
	m_sceneObject->getControllers().erase(m_sceneObject->getControllers().begin() + indexToRemove);

	// update UI
	m_ui->controllerSelector->setEnabled(!m_sceneObject->getControllers().empty());
	m_ui->removeControllerButton->setEnabled(!m_sceneObject->getControllers().empty());

	// Discard current controller
	m_controllerPropertiesModel->resetController();

	// Remove controller from controllers list of current geom
	m_geomControllersListModel->updateControllersList();
}

QStringList GeomControllersWidget::getAllPossibleControllerNamesFromTypesDb()
{
	QStringList result;
	gamelib::TypeRegistry::getInstance().forEachType([&result](const gamelib::Type* pType) {
		if (pType->getKind() == gamelib::TypeKind::COMPLEX)
		{
			const auto pAsComplex = static_cast<const gamelib::TypeComplex*>(pType);
			if (pAsComplex->isInheritedOf("ZEventBase"))
			{
				result.emplace_back(QString::fromStdString(pAsComplex->getName()));
			}
		}
	});

	return result;
}