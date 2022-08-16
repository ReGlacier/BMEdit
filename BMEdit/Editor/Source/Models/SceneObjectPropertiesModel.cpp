#include <Models/SceneObjectPropertiesModel.h>

#include <Types/QGlacierValue.h>

#include <GameLib/Type.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/TypeRegistry.h>


namespace models
{
	enum ColumnID : int {
		NAME = 0,
		VALUE = 1,

		MAX_COLUMNS
	};

	SceneObjectPropertiesModel::SceneObjectPropertiesModel(QObject *parent) : QAbstractTableModel(parent)
	{
	}

	int SceneObjectPropertiesModel::rowCount(const QModelIndex &parent) const
	{
		if (!m_level || !m_geomIndex.has_value()) {
			return 0;
		}

		const auto &geom = m_level->getSceneObjects().at(m_geomIndex.value());
		return static_cast<int>(geom->getProperties().getEntries().size());
	}

	int SceneObjectPropertiesModel::columnCount(const QModelIndex &parent) const
	{
		if (!m_level || !m_geomIndex.has_value()) {
			return 0;
		}

		return ColumnID::MAX_COLUMNS;
	}

	QVariant SceneObjectPropertiesModel::data(const QModelIndex &index, int role) const
	{
		if (role != Qt::DisplayRole)
		{
			return QVariant {};
		}

		const auto &geom = m_level->getSceneObjects().at(m_geomIndex.value());
		const auto &entList = geom->getProperties().getEntries();

		if (index.column() == ColumnID::NAME)
		{
			return QVariant(QString::fromStdString(entList[index.row()].name));
		}
		else if (index.column() == ColumnID::VALUE)
		{
			return QVariant::fromValue<types::QGlacierValue>({ geom->getProperties() });
		}

		return QVariant {};
	}

	bool SceneObjectPropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role)
	{
		if (index.column() == ColumnID::VALUE && role == Qt::EditRole)
		{
			if (!value.canConvert<types::QGlacierValue>())
				return false;

			const auto val = value.value<types::QGlacierValue>();
			const auto &geom = m_level->getSceneObjects().at(m_geomIndex.value());
			geom->getProperties() = val.value; //re-assign props
			return true;
		}

		return QAbstractItemModel::setData(index, value, role);
	}

	QVariant SceneObjectPropertiesModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			if (section == ColumnID::NAME) return QVariant { "Name" };
			if (section == ColumnID::VALUE) return QVariant { "Value" };
		}

		return QAbstractTableModel::headerData(section, orientation, role);
	}

	void SceneObjectPropertiesModel::setLevel(const gamelib::Level *level)
	{
		beginResetModel();

		m_level = level;
		m_geomIndex = std::nullopt;

		endResetModel();
	}

	void SceneObjectPropertiesModel::setGeomIndex(std::size_t geomIndex)
	{
		if (!m_level || m_level->getSceneObjects().size() <= geomIndex)
		{
			return;
		}

		beginResetModel();

		m_geomIndex = geomIndex;

		endResetModel();
	}

	void SceneObjectPropertiesModel::resetLevel()
	{
		beginResetModel();

		m_level = nullptr;
		m_geomIndex = std::nullopt;

		endResetModel();
	}

	void SceneObjectPropertiesModel::resetGeomIndex()
	{
		beginResetModel();

		m_geomIndex = std::nullopt;

		endResetModel();
	}
}