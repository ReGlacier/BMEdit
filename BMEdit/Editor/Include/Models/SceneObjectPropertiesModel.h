#pragma once

#include <GameLib/Level.h>
#include <GameLib/GMS/GMSGeomEntity.h>
#include <Models/ValueModelBase.h>


namespace gamelib::scene
{
	class SceneObject;
}

namespace models
{
	class SceneObjectPropertiesModel : public ValueModelBase
	{
		Q_OBJECT
	public:
		SceneObjectPropertiesModel(QObject *parent = nullptr);

		void setLevel(const gamelib::Level *level);
		void setGeom(gamelib::scene::SceneObject * geom);
		void resetLevel();
		void resetGeom();

	signals:
		void objectPropertiesChanged(const QString& propertyId, const gamelib::scene::SceneObject* pObject);

	private slots:
		void onValueChanged(const QString& propertyName);

	private:
		std::optional<std::size_t> m_geomIndex {};
		gamelib::scene::SceneObject* m_geom { nullptr };
		const gamelib::Level *m_level { nullptr };
	};
}