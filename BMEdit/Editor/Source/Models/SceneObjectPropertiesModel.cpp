#include <Models/SceneObjectPropertiesModel.h>
#include <Types/QGlacierValue.h>
#include <Editor/PropertyAsStringRepr.h>

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
		const auto &geom = m_level->getSceneObjects().at(m_geomIndex.value());
		const auto &entList = geom->getProperties().getEntries();
		const auto& currentEnt = entList[index.row()];

		if (index.column() == ColumnID::NAME)
		{
			if (role == Qt::DisplayRole)
			{
				return QVariant(QString::fromStdString(currentEnt.name));
			}
			else if (role == Qt::ToolTipRole && !currentEnt.views.empty())
			{
				const gamelib::Type* ownerType = currentEnt.views.back().getOwnerType();
				const QString declaredAtClass = QString::fromStdString(ownerType ? ownerType->getName() : "(Undefined)");
				const QString propertyName = QString::fromStdString(currentEnt.name);
				return QString("%1::%2").arg(declaredAtClass, propertyName);
			}
			else return {};
		}
		else if (index.column() == ColumnID::VALUE)
		{
			types::QGlacierValue value;
			value.instructions = gamelib::Span(geom->getProperties().getInstructions()).slice(currentEnt.instructions).as<std::vector<gamelib::prp::PRPInstruction>>();
			value.views = currentEnt.views;

			if (role == Qt::EditRole)
			{
				return QVariant::fromValue<types::QGlacierValue>(value);
			}
			else if (role == Qt::DisplayRole)
			{
				/**
				 * FIXME: Weird bug: when we getting into editor mode we have two widgets: caption & editor. Need to understand how to avoid this behaviour :thinking:
				 */
				return editor::PropertyAsStringRepr::getStringRepresentationOfValue(value);
			}
			else return {};
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

			// Here we need to check that we have same (by size) containers
			const auto& [off, sz] = geom->getProperties().getEntries()[index.row()].instructions;

			if (sz == val.instructions.size())
			{
				for (auto i = off; i < off + sz; ++i)
				{
					geom->getProperties().getInstructions()[i] = val.instructions[i - off];
				}

				return true;
			}

			return false;
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

	Qt::ItemFlags SceneObjectPropertiesModel::flags(const QModelIndex &index) const
	{
		if (index.column() == ColumnID::NAME)
		{
			return Qt::ItemFlag::ItemIsSelectable;
		}
		else if (index.column() == ColumnID::VALUE)
		{
			return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
		}

		return Qt::NoItemFlags;
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