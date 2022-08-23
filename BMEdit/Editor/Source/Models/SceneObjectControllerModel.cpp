#include <Models/SceneObjectControllerModel.h>

using namespace models;


SceneObjectControllerModel::SceneObjectControllerModel(QObject *parent)
	: ValueModelBase(parent)
{
	connect(this, &ValueModelBase::valueChanged, [=]() { onValueChanged(); });
}

void SceneObjectControllerModel::setGeom(gamelib::scene::SceneObject *geom)
{
	const bool isNewGeom = m_geom != geom;
	beginResetModel();
	m_geom = geom;
	m_controllerName.clear();
	endResetModel();

	if (isNewGeom)
	{
		resetValue();
	}
}

void SceneObjectControllerModel::resetGeom()
{
	beginResetModel();
	m_geom = nullptr;
	m_controllerName.clear();
	endResetModel();

	resetValue();
}

void SceneObjectControllerModel::setControllerName(const std::string &controllerName)
{
	if (!m_geom || !m_geom->getControllers().count(controllerName)) return;
	const bool isNewController = controllerName != m_controllerName;

	beginResetModel();
	m_controllerName = controllerName;
	endResetModel();

	if (isNewController)
	{
		setValue(m_geom->getControllers().at(controllerName));
	}
}

void SceneObjectControllerModel::resetControllerName()
{
	beginResetModel();
	m_controllerName.clear();
	endResetModel();

	resetValue();
}

void SceneObjectControllerModel::onValueChanged()
{
	if (m_geom && !m_controllerName.empty() && getValue().has_value())
	{
		m_geom->getControllers().at(m_controllerName) = getValue().value();
	}
}