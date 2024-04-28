#pragma once

#include <QColor>
#include <Models/ValueModelBase.h>
#include <GameLib/Scene/SceneObject.h>
#include <Types/QGlacierController.h>
#include <vector>


namespace models
{
	class SceneObjectControllerModel : public ValueModelBase
	{
		Q_OBJECT

	private:
		struct SpecialRow
		{
			int startRow { 0 };
			int endRow { 0 };
			QColor backgroundColor {};

			bool includes(int row) const
			{
				return row >= startRow && row <= endRow;
			}
		};

	public:
		SceneObjectControllerModel(QObject *parent = nullptr);

		void setGeom(gamelib::scene::SceneObject *geom);
		void resetGeom();
		void setControllerIndex(int controllerIndex);
		void resetController();

		QVariant data(const QModelIndex &index, int role) const override;

	private:
		void addSugarViews(const gamelib::Type* pControllerType, gamelib::Value& v, const std::string& scriptName);
		void removeSugarViews(const gamelib::Type* pControllerType, gamelib::Value& v);

	private slots:
		void onValueChanged();

	private:
		static constexpr int kUnset = -1;

		gamelib::scene::SceneObject *m_geom { nullptr };
		int m_currentControllerIndex = kUnset;
		std::vector<SpecialRow> m_specialRows {};
	};
}