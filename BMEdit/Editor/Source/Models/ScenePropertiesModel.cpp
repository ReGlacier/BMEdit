#include <Models/ScenePropertiesModel.h>
#include <Types/QSceneProperty.h>
#include <GameLib/Level.h>

using namespace models;


ScenePropertiesModel::ScenePropertiesModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int ScenePropertiesModel::rowCount(const QModelIndex &parent) const
{
	if (!isReady()) return 0;

	return static_cast<int>(m_level->getLevelProperties()->ZDefines.getDefinitions().size());
}

int ScenePropertiesModel::columnCount(const QModelIndex &parent) const
{
	if (!isReady()) return 0;
	return ColumnID::MAX_COLUMNS;
}

QVariant ScenePropertiesModel::data(const QModelIndex &index, int role) const
{
	if (!isReady() || index.row() >= m_level->getLevelProperties()->ZDefines.getDefinitions().size()) return {};

	if (index.column() == ColumnID::NAME && role == Qt::DisplayRole)
	{
		return QString::fromStdString(m_level->getLevelProperties()->ZDefines.getDefinitions().at(index.row()).getName());
	}
	else if (index.column() == ColumnID::VALUE && role == Qt::EditRole)
	{
		types::QSceneProperty prop { m_level->getLevelProperties()->ZDefines.getDefinitions().at(index.row()) };
		return QVariant::fromValue<types::QSceneProperty>(prop);
	}

	return {};
}

bool ScenePropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (role != Qt::EditRole || index.column() != ColumnID::VALUE || !value.canConvert<types::QSceneProperty>())
	{
		return false;
	}

	auto newVal = value.value<types::QSceneProperty>();

	if (!isReady() || (m_level->getLevelProperties()->ZDefines.getDefinitions().at(index.row()) != newVal.def))
	{
		m_level->getLevelProperties()->ZDefines.getDefinitions().at(index.row()) = newVal.def;
		emit valueChanged();
		return true;
	}

	return false;
}

QVariant ScenePropertiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == ColumnID::NAME) return QString("Name");
		if (section == ColumnID::VALUE) return QString("Value");
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ScenePropertiesModel::flags(const QModelIndex &index) const
{
	if (index.column() == ColumnID::NAME)
	{
		return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
	}
	else if (index.column() == ColumnID::VALUE)
	{
		return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable;
	}

	return Qt::NoItemFlags;
}

void ScenePropertiesModel::setLevel(gamelib::Level *level)
{
	if (m_level == level) return;

	beginResetModel();
	m_level = level;
	endResetModel();
}

void ScenePropertiesModel::resetLevel()
{
	if (!isReady()) return;

	beginResetModel();
	m_level = nullptr;
	endResetModel();
}

bool ScenePropertiesModel::isReady() const
{
	return m_level != nullptr;
}