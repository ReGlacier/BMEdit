#include <Models/ScenePrimitivesModel.h>
#include <Types/QCustomRoles.h>
#include <Types/QPrimType.h>


using namespace models;

ScenePrimitivesModel::ScenePrimitivesModel(QObject *parent) : QAbstractTableModel(parent)
{
}

int ScenePrimitivesModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);

	if (!m_level)
		return 0;

	return static_cast<int>(m_level->getLevelGeometry()->chunks.size());
}

int ScenePrimitivesModel::columnCount(const QModelIndex &parent) const
{
	return ColumnID::CID_MAX_COLUMNS;
}

QVariant ScenePrimitivesModel::data(const QModelIndex &index, int role) const
{
	auto getKindAsQString = [](gamelib::prm::PRMChunkRecognizedKind kind) -> QString
	{
		switch (kind)
		{
			case gamelib::prm::PRMChunkRecognizedKind::CRK_UNKNOWN_BUFFER: return "UNKNOWN BUFFER";
			case gamelib::prm::PRMChunkRecognizedKind::CRK_VERTEX_BUFFER: return "VERTEX BUFFER";
			case gamelib::prm::PRMChunkRecognizedKind::CRK_INDEX_BUFFER: return "INDEX BUFFER";
			case gamelib::prm::PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER: return "DESCRIPTION BUFFER";
			case gamelib::prm::PRMChunkRecognizedKind::CRK_ZERO_CHUNK: return "<ZERO CHUNK>";
		}

		return "???";
	};

	auto getVertexFormatAsQString = [](gamelib::prm::PRMVertexBufferFormat bufferFormat) -> QString
	{
		switch (bufferFormat)
		{
			case gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_10: return "Vertex Format 10";
			case gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_24: return "Vertex Format 24";
			case gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_28: return "Vertex Format 28";
			case gamelib::prm::PRMVertexBufferFormat::VBF_VERTEX_34: return "Vertex Format 34";
		    case gamelib::prm::PRMVertexBufferFormat::VBF_UNKNOWN_VERTEX: return "UNKNOWN FORMAT";
	    }

		return "???";
	};

	if (!m_level)
	{
		return {};
	}

	auto& chk = m_level->getLevelGeometry()->chunks.at(index.row());

	if (role == Qt::DisplayRole)
	{
		switch (static_cast<ColumnID>(index.column()))
		{
			case ColumnID::CID_INDEX: return chk.getIndex();
			case ColumnID::CID_KIND:  return getKindAsQString(chk.getKind());
		    case ColumnID::CID_SIZE:  return chk.getBuffer().size();
		    case ColumnID::CID_VERTICES:
		    {
			    if (chk.getKind() == gamelib::prm::PRMChunkRecognizedKind::CRK_VERTEX_BUFFER)
			    {
				    return getVertexFormatAsQString(chk.getVertexBufferHeader()->vertexFormat);
			    }
			    break;
		    }
		    case ColumnID::CID_INDICES:
		    {
			    if (chk.getKind() == gamelib::prm::PRMChunkRecognizedKind::CRK_INDEX_BUFFER)
			    {
				    return chk.getIndexBufferHeader()->indicesCount;
			    }
			    break;
		    }
		    case ColumnID::CID_PTR_OBJECTS:
		    {
			    if (chk.getKind() == gamelib::prm::PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER)
			    {
				    return QString("%1 (0x%2)").arg(chk.getDescriptionBufferHeader()->ptrObjects).arg(chk.getDescriptionBufferHeader()->ptrObjects, 8, 16, QChar('0'));
			    }
			    break;
		    }
		    case ColumnID::CID_PTR_PARTS:
		    {
			    if (chk.getKind() == gamelib::prm::PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER)
			    {
				    return QString("%1 (0x%2)").arg(chk.getDescriptionBufferHeader()->ptrParts).arg(chk.getDescriptionBufferHeader()->ptrParts, 8, 16, QChar('0'));
			    }
			    break;
		    }
		    default: return {};
		}
	}
	else if (role == types::kChunkKindRole)
	{
		return QVariant::fromValue<gamelib::prm::PRMChunkRecognizedKind>(chk.getKind());
	}
	else if (role == types::kChunkIndexRole)
	{
		return chk.getIndex();
	}
	else if (role == types::kChunkVertexFormatRole)
	{
		if (auto chkVertexBufferHeader = chk.getVertexBufferHeader(); chkVertexBufferHeader)
		{
			return QVariant::fromValue<gamelib::prm::PRMVertexBufferFormat>(chkVertexBufferHeader->vertexFormat);
		}
	}

	return {};
}

bool ScenePrimitivesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);
	// This model is read only
	return false;
}

QVariant ScenePrimitivesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
	{
		constexpr std::array<std::string_view, ColumnID::CID_MAX_COLUMNS> g_ColNames = {
		    "Index", "Kind (chunk)", "Size (chunk)", "Indices", "Vertices", "Objects REF", "Parts REF"
		};

		return QString::fromStdString(g_ColNames.at(section).data());
	}

	return {};
}

Qt::ItemFlags ScenePrimitivesModel::flags(const QModelIndex &index) const
{
	return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}

void ScenePrimitivesModel::setLevel(gamelib::Level* level)
{
	if (level)
	{
		beginResetModel();
		m_level = level;
		endResetModel();
	}
}

void ScenePrimitivesModel::resetLevel()
{
	beginResetModel();
	m_level = nullptr;
	endResetModel();
}