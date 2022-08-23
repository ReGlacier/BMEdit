#pragma once

#include <Models/ValueModelBase.h>
#include <GameLib/Scene/SceneObject.h>


namespace models
{
	class SceneObjectControllerModel : public ValueModelBase
	{
		Q_OBJECT
	public:
		SceneObjectControllerModel(QObject *parent = nullptr);

		void setGeom(gamelib::scene::SceneObject *geom);
		void resetGeom();
		void setControllerName(const std::string &controllerName);
		void resetControllerName();

	private slots:
		void onValueChanged();

	private:
		gamelib::scene::SceneObject *m_geom { nullptr };
		std::string m_controllerName {};
	};
}