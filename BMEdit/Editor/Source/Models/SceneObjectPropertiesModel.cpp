#include <Models/SceneObjectPropertiesModel.h>
#include <Types/QGlacierValue.h>

#include <GameLib/Type.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/Scene/SceneObject.h>


namespace models
{
	SceneObjectPropertiesModel::SceneObjectPropertiesModel(QObject *parent)
		: ValueModelBase(parent)
	{
		connect(this, &ValueModelBase::valueChanged, [=]() { onValueChanged(); });
	}

	void SceneObjectPropertiesModel::setLevel(const gamelib::Level *level)
	{
		beginResetModel();
		m_level = level;
		m_geom = nullptr;
		endResetModel();

		resetValue();
	}

	void SceneObjectPropertiesModel::setGeom(gamelib::scene::SceneObject * geom)
	{
		const bool isNewGeom = m_geom != geom;
		beginResetModel();

		if (m_level)
		{
			for (const auto& sceneGeomSP : m_level->getSceneObjects())
			{
				if (sceneGeomSP.get() == geom)
				{
					m_geom = geom;
					break;
				}
			}
		}

		endResetModel();

		if (m_geom && isNewGeom)
		{
			setValue(m_geom->getProperties());
		}
	}

	void SceneObjectPropertiesModel::resetLevel()
	{
		beginResetModel();
		m_level = nullptr;
		m_geom = nullptr;
		endResetModel();

		resetValue();
	}

	void SceneObjectPropertiesModel::resetGeom()
	{
		beginResetModel();
		m_geom = nullptr;
		endResetModel();

		resetValue();
	}

	void SceneObjectPropertiesModel::onValueChanged()
	{
		const auto& value = getValue();
		if (!value.has_value()) return;

		if (value.value() != m_geom->getProperties())
		{
			m_geom->getProperties() = value.value();

			emit objectPropertiesChanged(m_geom);
		}
	}
}