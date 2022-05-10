#include <Models/SceneObjectPropertiesModel.h>
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

	struct TypePropertyView
	{
		const gamelib::TypeComplex *owner { nullptr };
		gamelib::ValueView view {};
	};

	void buildPropertyViewsOfComplexType(const gamelib::TypeComplex *asComplex, QList<TypePropertyView> &viewList) // NOLINT(misc-no-recursion)
	{
		if (!asComplex || asComplex->areUnexposedInstructionsAllowed())
		{
			//Unable to recognize props list (?)
			return;
		}

		const gamelib::TypeComplex *parentType = asComplex;

		if (auto parent = parentType->getParent(); parent && parent->getKind() == gamelib::TypeKind::COMPLEX)
		{
			buildPropertyViewsOfComplexType(reinterpret_cast<const gamelib::TypeComplex *>(parent), viewList);
		}

		for (const auto &propertyView: asComplex->getInstructionViews())
		{
			auto &view = viewList.emplace_back();
			view.view = propertyView;
			view.owner = asComplex;
		}
	}

	int getCountOfPropertiesInComplexType(const gamelib::TypeComplex *complexType) // NOLINT(misc-no-recursion)
	{
		if (!complexType || complexType->getKind() != gamelib::TypeKind::COMPLEX)
		{
			return 0;
		}

		return getCountOfPropertiesInComplexType(reinterpret_cast<const gamelib::TypeComplex *>(complexType->getParent())) + static_cast<int>(complexType->getInstructionViews().size());
	}

	SceneObjectPropertiesModel::SceneObjectPropertiesModel(QObject *parent) : QAbstractTableModel(parent)
	{
	}

	int SceneObjectPropertiesModel::rowCount(const QModelIndex &parent) const
	{
		if (!m_level || !m_geomIndex.has_value()) {
			return 0;
		}

		const auto &geom = m_level->getSceneProperties()->header.getEntries().getGeomEntities().at(m_geomIndex.value());

		auto type = gamelib::TypeRegistry::getInstance().findTypeByHash(geom.getTypeId());
		if (!type || type->getKind() != gamelib::TypeKind::COMPLEX)
		{
			return 0;
		}

		auto asComplex = reinterpret_cast<const gamelib::TypeComplex *>(type);
		return getCountOfPropertiesInComplexType(asComplex);
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

		const auto& geom = m_level->getSceneProperties()->header.getEntries().getGeomEntities()[m_geomIndex.value()];
		auto type = gamelib::TypeRegistry::getInstance().findTypeByHash(geom.getTypeId());

		if (!type)
		{
			return QVariant {};
		}

		QList<TypePropertyView> viewList;
		buildPropertyViewsOfComplexType(reinterpret_cast<const gamelib::TypeComplex *>(type), viewList);

		if (!viewList.isEmpty())
		{
			if (index.column() == ColumnID::NAME)
			{
				auto &view = viewList.at(index.row());

				return QVariant { QString("%1::%2").arg(QString::fromStdString(view.owner->getName()), QString::fromStdString(view.view.getName())) };
			}
			else if (index.column() == ColumnID::VALUE)
			{
				//Here we need to put our data into QVariant and our 'TypePropertyItemDelegate' will draw it for us
				//To make it possible we need to combine scene object index (?)
			}
		}

		return QVariant {};
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
		if (!m_level)
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