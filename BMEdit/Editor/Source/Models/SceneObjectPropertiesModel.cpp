#include <Models/SceneObjectPropertiesModel.h>
#include <Types/QGlacierValue.h>

#include <GameLib/Type.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/Scene/SceneObject.h>


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
		if (!m_level || !m_geom) {
			return 0;
		}

		return static_cast<int>(m_geom->getProperties().getEntries().size());
	}

	int SceneObjectPropertiesModel::columnCount(const QModelIndex &parent) const
	{
		if (!m_level || !m_geom) {
			return 0;
		}

		return ColumnID::MAX_COLUMNS;
	}

	QVariant SceneObjectPropertiesModel::data(const QModelIndex &index, int role) const
	{
		if (!m_geom) return {};

		const auto &entList = m_geom->getProperties().getEntries();
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
			value.instructions = gamelib::Span(m_geom->getProperties().getInstructions()).slice(currentEnt.instructions).as<std::vector<gamelib::prp::PRPInstruction>>();
			value.views = currentEnt.views;

			if (role == Qt::EditRole)
			{
				return QVariant::fromValue<types::QGlacierValue>(value);
			}
			else if (role == Qt::DisplayRole)
			{
				//TODO: Create repr?
			}
			else return {};
		}

		return QVariant {};
	}

	bool SceneObjectPropertiesModel::setData(const QModelIndex &index, const QVariant &value, int role)
	{
		if (!m_geom) return false;

		if (index.column() == ColumnID::VALUE && role == Qt::EditRole)
		{
			if (!value.canConvert<types::QGlacierValue>())
				return false;

			const auto val = value.value<types::QGlacierValue>();

			// Here we need to check that we have same (by size) containers
			const auto& [off, sz] = m_geom->getProperties().getEntries()[index.row()].instructions;

			if (sz == val.instructions.size())
			{
				for (auto i = off; i < off + sz; ++i)
				{
					m_geom->getProperties().getInstructions()[i] = val.instructions[i - off];
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
		m_geom = nullptr;

		endResetModel();
	}

	void SceneObjectPropertiesModel::setGeom(gamelib::scene::SceneObject * geom)
	{
		beginResetModel();

		if (m_level)
		{
			for (const auto& sceneGeomSP : m_level->getSceneObjects())
			{
				if (sceneGeomSP.get() == geom)
				{
					m_geom = geom;
					break;
				}
			}
		}

		endResetModel();
	}

	void SceneObjectPropertiesModel::resetLevel()
	{
		beginResetModel();

		m_level = nullptr;
		m_geom = nullptr;

		endResetModel();
	}

	void SceneObjectPropertiesModel::resetGeom()
	{
		beginResetModel();

		m_geom = nullptr;

		endResetModel();
	}
}