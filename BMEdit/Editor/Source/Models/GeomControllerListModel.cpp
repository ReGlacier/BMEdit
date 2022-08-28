#include <Models/GeomControllerListModel.h>
#include <Types/QGlacierController.h>
#include <GameLib/Scene/SceneObject.h>

namespace models
{

	GeomControllerListModel::GeomControllerListModel(QObject *parent) : QAbstractListModel(parent)
	{
	}

	Qt::ItemFlags GeomControllerListModel::flags(const QModelIndex &index) const
	{
		return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
	}

	QModelIndex GeomControllerListModel::index(int row, int column, const QModelIndex &parent) const
	{
		if (!hasIndex(row, column, parent) || !isReady()) {
			return {};
		}

		if (row >= 0 && row < m_sceneObject->getControllers().size()) {
			return createIndex(row, column);
		}

		return {};
	}

	QVariant GeomControllerListModel::data(const QModelIndex &index, int role) const
	{
		if (!isReady()) {
			return {};
		}

		if (role == Qt::DisplayRole) {
			return QString::fromStdString(m_sceneObject->getControllers().at(index.row()).name);
		}

		return {};
	}

	bool GeomControllerListModel::setData(const QModelIndex &index, const QVariant &value, int role)
	{
		if (!isReady() || (role != Qt::EditRole) || (index.row() >= m_sceneObject->getControllers().size()) || !value.canConvert<types::QGlacierController>()) {
			return false;
		}

		auto val = value.value<types::QGlacierController>();
		auto &toEdit = m_sceneObject->getControllers().at(index.row());
		toEdit.name = val.name.toStdString();
		toEdit.properties = val.data;

		return true;
	}

	int GeomControllerListModel::rowCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);

		if (!isReady()) {
			return 0;
		}

		return static_cast<int>(m_sceneObject->getControllers().size());
	}

	int GeomControllerListModel::columnCount(const QModelIndex &parent) const
	{
		Q_UNUSED(parent);

		if (!isReady()) {
			return 0;
		}

		return 1;
	}

	void GeomControllerListModel::setGeom(gamelib::scene::SceneObject *sceneObject)
	{
		if (m_sceneObject == sceneObject) {
			return;
		}

		beginResetModel();
		m_sceneObject = sceneObject;
		endResetModel();
	}

	void GeomControllerListModel::resetGeom()
	{
		if (!m_sceneObject) {
			return;
		}

		beginResetModel();
		m_sceneObject = nullptr;
		endResetModel();
	}

	bool GeomControllerListModel::isReady() const
	{
		return m_sceneObject != nullptr;
	}
}