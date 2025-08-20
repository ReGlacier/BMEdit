#include <Models/SceneTexturesModel.h>
#include <Types/QTextureREF.h>
#include <GameLib/Level.h>

using namespace models;


SceneTexturesModel::SceneTexturesModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int SceneTexturesModel::rowCount(const QModelIndex &parent) const
{
	if (!isReady()) return 0;
	return static_cast<int>(m_level->getSceneTextures()->entries.size());
}

int SceneTexturesModel::columnCount(const QModelIndex &parent) const
{
	if (!isReady()) return 0;
	return ColumnID::MAX_COLUMNS;
}

QVariant SceneTexturesModel::data(const QModelIndex &index, int role) const
{
	if (!isReady() || index.row() >= m_level->getSceneTextures()->entries.size())
	{
		return {};
	}

	if (index.column() == ColumnID::INDEX)
	{
		if (role == Qt::DisplayRole)
		{
			// May be customized
			return m_level->getSceneTextures()->entries.at(index.row()).m_index;
		}

		if (role == Qt::UserRole)
		{
			// DO NOT CUSTOMIZE THIS
			return m_level->getSceneTextures()->entries.at(index.row()).m_index;
		}
	}

	if (index.column() == ColumnID::OFFSET && role == Qt::DisplayRole)
	{
		return m_level->getSceneTextures()->entries.at(index.row()).m_offset;
	}

	if (index.column() == ColumnID::NAME && role == Qt::DisplayRole)
	{
		if (auto& name = m_level->getSceneTextures()->entries.at(index.row()).m_fileName; name.has_value())
		{
			return QString::fromStdString(name.value());
		}

		return QString();
	}
#ifndef NDEBUG
	if (index.column() == ColumnID::CUBEMAP && role == Qt::DisplayRole)
	{
		return QString("%1").arg(m_level->getSceneTextures()->entries.at(index.row()).m_flags, 8, 16);
	}
#endif

	if (index.column() == ColumnID::RESOLUTION && role == Qt::DisplayRole)
	{
		const auto& entry = m_level->getSceneTextures()->entries.at(index.row());
		return QString("%1x%2").arg(entry.m_width).arg(entry.m_height);
	}

	if (index.column() == ColumnID::FORMAT && role == Qt::DisplayRole)
	{
		auto formatToQString = [](gamelib::tex::TEXEntryType type) -> QString {
			switch (type)
			{
				case gamelib::tex::TEXEntryType::ET_BITMAP_I8:       return "I8";
				case gamelib::tex::TEXEntryType::ET_BITMAP_EMBM:     return "EMBM";
				case gamelib::tex::TEXEntryType::ET_BITMAP_DOT3:     return "DOT3";
				case gamelib::tex::TEXEntryType::ET_BITMAP_CUBE:     return "CUBE";
				case gamelib::tex::TEXEntryType::ET_BITMAP_DMAP:     return "DMAP";
				case gamelib::tex::TEXEntryType::ET_BITMAP_PAL:      return "PAL (Neg)";
				case gamelib::tex::TEXEntryType::ET_BITMAP_PAL_OPAC: return "PAL (Opac)";
				case gamelib::tex::TEXEntryType::ET_BITMAP_32:       return "RGBA";
				case gamelib::tex::TEXEntryType::ET_BITMAP_U8V8:     return "U8V8";
				case gamelib::tex::TEXEntryType::ET_BITMAP_DXT1:     return "DXT1";
				case gamelib::tex::TEXEntryType::ET_BITMAP_DXT3:     return "DXT3";
			    default:
				    assert(false && "Unknown entry");
				    return "Unknown";
			}
		};

		return formatToQString(m_level->getSceneTextures()->entries.at(index.row()).m_type1);
	}

	if (role == SceneTexturesModel::Roles::R_TEXTURE_REF)
	{
		types::QTextureREF ref;
		ref.ownerModel = this;
		ref.textureIndex = m_level->getSceneTextures()->entries.at(index.row()).m_index;
		return QVariant::fromValue<types::QTextureREF>(ref);
	}

	return {};
}

bool SceneTexturesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	//TODO: Fixme later
	return false;
}

QVariant SceneTexturesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == ColumnID::NAME) return QString("Name");
		if (section == ColumnID::OFFSET) return QString("Offset");
		if (section == ColumnID::INDEX) return QString("Index");
		if (section == ColumnID::FORMAT) return QString("Format");
		if (section == ColumnID::RESOLUTION) return QString("Resolution");
#ifndef NDEBUG
		if (section == ColumnID::CUBEMAP) return QString("CubeMap");
#endif
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags SceneTexturesModel::flags(const QModelIndex &index) const
{
	return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled; //TODO: Fixme later
}

void SceneTexturesModel::setLevel(const gamelib::Level *level)
{
	if (m_level == level) return;

	beginResetModel();
	m_level = level;
	endResetModel();
}

void SceneTexturesModel::resetLevel()
{
	if (!isReady()) return;

	beginResetModel();
	m_level = nullptr;
	endResetModel();
}

const gamelib::Level *SceneTexturesModel::getLevel() const
{
	return m_level;
}

gamelib::Level *SceneTexturesModel::getLevel()
{
	return const_cast<gamelib::Level*>(m_level); // Bruh. TODO: Refactor later
}

bool SceneTexturesModel::isReady() const
{
	return m_level != nullptr;
}