#include <Models/ValueModelBase.h>
#include <Types/QGlacierValue.h>
#include <GameLib/Type.h>

using namespace models;


int ValueModelBase::rowCount(const QModelIndex &parent) const
{
	if (!isReady()) return 0;

	return static_cast<int>(m_value.value().getEntries().size());
}

int ValueModelBase::columnCount(const QModelIndex &parent) const
{
	if (!isReady()) return 0;

	return ColumnID::MAX_COLUMNS;
}

QVariant ValueModelBase::data(const QModelIndex &index, int role) const
{
	if (!isReady()) return {};

	const auto &entList = m_value.value().getEntries();;
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
		value.instructions = gamelib::Span(m_value->getInstructions()).slice(currentEnt.instructions).as<std::vector<gamelib::prp::PRPInstruction>>();
		value.views = currentEnt.views;

		if (role == Qt::EditRole)
		{
			return QVariant::fromValue<types::QGlacierValue>(value);
		}
		else return {};
	}

	return QVariant {};
}

bool ValueModelBase::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!isReady()) return false;

	if (index.column() == ColumnID::VALUE && role == Qt::EditRole)
	{
		if (!value.canConvert<types::QGlacierValue>())
			return false;

		const auto val = value.value<types::QGlacierValue>();

		// Here we need to check that we have same (by size) containers
		const auto& [off, sz] = m_value.value().getEntries()[index.row()].instructions;

		if (sz == val.instructions.size())
		{
			for (auto i = off; i < off + sz; ++i)
			{
				m_value.value().getInstructions()[i] = val.instructions[i - off];
			}

			emit valueChanged();

			return true;
		}

		return false;
	}

	return QAbstractItemModel::setData(index, value, role);
}

QVariant ValueModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		if (section == ColumnID::NAME) return QVariant { "Name" };
		if (section == ColumnID::VALUE) return QVariant { "Value" };
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ValueModelBase::flags(const QModelIndex &index) const
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

void ValueModelBase::setValue(const gamelib::Value &value)
{
	beginResetModel();
	m_value = value;
	endResetModel();

	emit valueChanged();
}

void ValueModelBase::resetValue()
{
	beginResetModel();
	m_value = std::nullopt;
	endResetModel();

	emit valueChanged();
}

const std::optional<gamelib::Value> &ValueModelBase::getValue() const
{
	return m_value;
}

bool ValueModelBase::isReady() const
{
	return m_value.has_value();
}