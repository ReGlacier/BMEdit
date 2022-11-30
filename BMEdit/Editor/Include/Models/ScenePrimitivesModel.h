#pragma once

#include <QAbstractTableModel>
#include <GameLib/Level.h>


namespace models
{
	class ScenePrimitivesModel : public QAbstractTableModel
	{
		Q_OBJECT

	private:
		enum ColumnID : int {
			CID_INDEX = 0,      // Index of chunk
			CID_KIND,           // Kind of chunk
			CID_SIZE,           // Size of chunk
			CID_INDICES,        // Only for index buffer
			CID_VERTICES,       // Only for vertex buffer
			CID_PTR_OBJECTS,    // Only for description
			CID_PTR_PARTS,      // Only for description

			//NOTE: Don't forgot to add string view at g_ColNames array (see .cpp for details)

			// --- END ---
			CID_MAX_COLUMNS
		};

	public:
		ScenePrimitivesModel(QObject *parent = nullptr);

		int rowCount(const QModelIndex &parent) const override;
		int columnCount(const QModelIndex &parent) const override;
		QVariant data(const QModelIndex &index, int role) const override;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;

		void setLevel(gamelib::Level* level);
		void resetLevel();

	private:
		gamelib::Level* m_level { nullptr };
	};
}