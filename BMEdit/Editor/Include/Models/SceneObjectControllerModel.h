#pragma once

#include <Models/ValueModelBase.h>
#include <GameLib/Scene/SceneObject.h>
#include <Types/QGlacierController.h>


namespace models
{
	class SceneObjectControllerModel : public ValueModelBase
	{
		Q_OBJECT
	public:
		SceneObjectControllerModel(QObject *parent = nullptr);

		void setGeom(gamelib::scene::SceneObject *geom);
		void resetGeom();
		void setControllerIndex(int controllerIndex);
		void resetController();

	private slots:
		void onValueChanged();

	private:
		gamelib::scene::SceneObject *m_geom { nullptr };
		int m_currentControllerIndex = -1;
	};
}