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
	m_currentControllerIndex = -1;
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
	m_currentControllerIndex = -1;
	endResetModel();

	resetValue();
}

void SceneObjectControllerModel::setControllerIndex(int controllerIndex)
{
	if (!m_geom || controllerIndex < 0 || controllerIndex >= m_geom->getControllers().size() || m_currentControllerIndex == controllerIndex)
	{
		return;
	}

	beginResetModel();
	m_currentControllerIndex = controllerIndex;
	endResetModel();

	setValue(m_geom->getControllers().at(controllerIndex).properties);
}

void SceneObjectControllerModel::resetController()
{
	beginResetModel();
	m_currentControllerIndex = -1;
	endResetModel();

	resetValue();
}

void SceneObjectControllerModel::onValueChanged()
{
	if (m_geom && m_currentControllerIndex != -1 && m_currentControllerIndex >= 0 && m_currentControllerIndex < m_geom->getControllers().size() && getValue().has_value())
	{
		m_geom->getControllers().at(m_currentControllerIndex).properties = getValue().value();
	}
}