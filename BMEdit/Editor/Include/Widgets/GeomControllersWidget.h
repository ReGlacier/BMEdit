#pragma once

#include <QWidget>
#include <QStringListModel>

#include <GameLib/Scene/SceneObject.h>


namespace models
{
	class ValueModelBase;
}

namespace delegates
{
	class TypePropertyItemDelegate;
}

namespace widgets
{
	namespace Ui
	{
		class GeomControllersWidget;
	}

	class GeomControllersWidget : public QWidget
	{
		Q_OBJECT

	public:
		GeomControllersWidget(QWidget *parent = nullptr);
		~GeomControllersWidget();

		void setGeom(gamelib::scene::SceneObject* sceneObject);
		void resetGeom();
		void setController(const QString& controllerName);

		[[nodiscard]] const gamelib::scene::SceneObject* getGeom() const;
		[[nodiscard]] const QString &getController() const;

		void switchToDefaults();
		void switchToFirstController();

	signals:
		void geomChanged();
		void geomReset();
		void editControllers();
		void controllerSelectionChanged(const QString& controllerName);

	private:
		void setup();

	private:
		Ui::GeomControllersWidget *m_ui { nullptr };

		gamelib::scene::SceneObject* m_sceneObject;
		QString m_currentController;
		QStringListModel *m_controllersListModel{ nullptr };
		models::ValueModelBase *m_controllerPropertiesModel{ nullptr };
		delegates::TypePropertyItemDelegate *m_controllerEditorDelegate{ nullptr };
	};
}