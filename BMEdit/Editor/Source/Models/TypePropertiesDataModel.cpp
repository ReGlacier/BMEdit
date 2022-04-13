#include <Models/TypePropertiesDataModel.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeComplex.h>
#include <GameLib/Type.h>
#include <deque>


namespace models
{
	using namespace gamelib;

	enum ColumnID : int
	{
		NAME,
		VALUE,
		CLASS_NAME,

		TOTAL_COLUMNS
	};

	int calculateFieldsCountRecursive(const Type *type)
	{
		int count = 0;
		if (!type)
		{
			return 0;
		}

		if (type->getKind() == TypeKind::COMPLEX)
		{
			auto complexType = reinterpret_cast<const TypeComplex *>(type);
			count += complexType->getInstructionViews().size();
			count += calculateFieldsCountRecursive(complexType->getParent());
		}
		else if (type->getKind() == TypeKind::ENUM)
		{
			return 1;
		}

		return count;
	}

	const ValueView *locatePropertyInfoByIndexInHierarchy(int index, const TypeComplex *type)
	{
		// Here we need save list of parent classes to locate mapping between them
		std::deque<const TypeComplex *> parentClassList { type };

		const Type *parent = type->getParent();
		while (parent)
		{
			if (parent->getKind() == TypeKind::COMPLEX)
			{
				auto parentComplex = reinterpret_cast<const TypeComplex *>(parent);
				if (!parentComplex->getInstructionViews().empty())
				{
					parentClassList.push_front(parentComplex);
				}

				parent = parentComplex->getParent();
			}
			else
			{
				break;
			}
		}

		int propertyIndex = 0;
		for (const auto& parentClass: parentClassList)
		{
			if (index >= propertyIndex && index < propertyIndex + parentClass->getInstructionViews().size())
			{
				// It's here
				int localIndex = index - propertyIndex;
				return &parentClass->getInstructionViews()[localIndex];
			}

			propertyIndex += parentClass->getInstructionViews().size();
		}

		return nullptr;
	}


	TypePropertiesDataModel::TypePropertiesDataModel(QObject *parent) : QAbstractTableModel(parent)
	{
	}

	int TypePropertiesDataModel::rowCount(const QModelIndex &parent) const
	{
		using namespace gamelib;

		Q_UNUSED(parent);

		if (m_currentTypeName.isEmpty())
		{
			return 0;
		}

		// So, here we need to recognize how much fields here
		auto type = TypeRegistry::getInstance().findTypeByName(m_currentTypeName.toStdString());
		if (!type)
		{
			return 0;
		}

		return calculateFieldsCountRecursive(type);
	}

	int TypePropertiesDataModel::columnCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);
		if (m_currentTypeName.isEmpty())
		{
			return 0;
		}

		auto type = TypeRegistry::getInstance().findTypeByName(m_currentTypeName.toStdString());
		if (type)
		{
			if (type->getKind() == TypeKind::COMPLEX)
			{
				return ColumnID::TOTAL_COLUMNS;
			}

			return 2; // Only name + value
		}

		return 0;
	}

	QVariant TypePropertiesDataModel::data(const QModelIndex &index, int role) const
	{
		if (role != Qt::DisplayRole)
			return QVariant();

		const int row = index.row();
		auto type = TypeRegistry::getInstance().findTypeByName(m_currentTypeName.toStdString());
		if (!type)
		{
			return QVariant();
		}

		const auto typeKind = type->getKind();

		if (typeKind == TypeKind::COMPLEX)
		{
			// Good, process complex property here
			auto info = locatePropertyInfoByIndexInHierarchy(row, reinterpret_cast<const TypeComplex *>(type));
			if (!info)
			{
				return QVariant();
			}

			if (index.column() == ColumnID::NAME)
			{
				return QVariant(QString::fromStdString(info->getName()));
			}

			if (index.column() == ColumnID::VALUE && info->getOwnerType())
			{
				// Return property type owner + property name to locate it in item delegate
				return QVariant(
					QList<QString>
					    {
							QString::fromStdString(info->getOwnerType()->getName()),
							QString::fromStdString(info->getName())
					    });
			}

			if (index.column() == ColumnID::CLASS_NAME && info->getOwnerType())
			{
				return QVariant(QString::fromStdString(info->getOwnerType()->getName()));
			}
		}
		else if (typeKind == TypeKind::ENUM && index.column() == ColumnID::NAME)
		{
			return QVariant("Value");
		}

		return QVariant();
	}

	QVariant TypePropertiesDataModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		{
			if (section == ColumnID::NAME) return QVariant("Name");
			if (section == ColumnID::VALUE) return QVariant("Value");
			if (section == ColumnID::CLASS_NAME) return QVariant("Class");
		}

		return QAbstractTableModel::headerData(section, orientation, role);
	}

	void TypePropertiesDataModel::setType(const QString &typeName)
	{
		beginResetModel();
		m_currentTypeName = typeName;
		endResetModel();
	}

	void TypePropertiesDataModel::resetType()
	{
		setType(QString());
	}
}