#pragma once

#include <QAbstractItemModel>
#include <optional>


namespace gamelib
{
	class Level;
}

namespace models
{
	class SceneTexturesModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		enum ColumnID : int {
			INDEX = 0,
			NAME,
			RESOLUTION,
			FORMAT,
			OFFSET,
#ifndef NDEBUG
			CUBEMAP,
#endif

			MAX_COLUMNS,
		};

		/**
		 * @enum Roles
		 * @brief Contains all custom roles
		 */
		enum Roles : int {
			R_TEXTURE_REF = Qt::UserRole + 1 ///< Get texture ref by QModelIndex
		};

	public:
		SceneTexturesModel(QObject *parent = nullptr);

		int rowCount(const QModelIndex& parent) const override;
		int columnCount(const QModelIndex& parent) const override;
		QVariant data(const QModelIndex& index, int role) const override;
		bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags(const QModelIndex& index) const override;

		void setLevel(const gamelib::Level *level);
		void resetLevel();
		const gamelib::Level *getLevel() const;
		gamelib::Level *getLevel();

	private:
		bool isReady() const;

	private:
		const gamelib::Level *m_level { nullptr };
	};
}