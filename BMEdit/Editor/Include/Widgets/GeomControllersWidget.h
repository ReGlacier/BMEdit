#pragma once

#include <QWidget>
#include <QScopedPointer>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <GameLib/Scene/SceneObject.h>


namespace models
{
	class SceneObjectControllerModel;
	class GeomControllerListModel;
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
		void setController(int controllerIndex);

		[[nodiscard]] const gamelib::scene::SceneObject* getGeom() const;

		void switchToDefaults();
		void switchToFirstController();

	signals:
		void geomChanged();
		void geomReset();
		void editControllers();

	private:
		static QStringList getAllPossibleControllerNamesFromTypesDb();

	private:
		void setup();
		void updateAvailableControllersList();
		void addControllerToGeom(const QString& controllerName);
		void removeCurrentController();

	private:
		Ui::GeomControllersWidget *m_ui { nullptr };

		gamelib::scene::SceneObject* m_sceneObject;
		models::GeomControllerListModel *m_geomControllersListModel { nullptr };
		models::SceneObjectControllerModel *m_controllerPropertiesModel{ nullptr };
		delegates::TypePropertyItemDelegate *m_controllerEditorDelegate{ nullptr };

		QScopedPointer<QStringListModel> m_availableToAddControllersModel { nullptr }; // All possible controller classes
		QScopedPointer<QSortFilterProxyModel> m_availableToAddControllersProxyModel { nullptr }; // Filtered by user input controller classes
	};
}