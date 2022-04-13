#pragma once

#include <QAbstractTableModel>
#include <QMetaType>
#include <QObject>
#include <QString>


namespace models
{
	class TypePropertiesDataModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		explicit TypePropertiesDataModel(QObject *parent = nullptr);

		int rowCount(const QModelIndex &parent) const override;
		int columnCount(const QModelIndex &parent) const override;
		QVariant data(const QModelIndex &index, int role) const override;
		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const override;

		void setType(const QString& typeName);
		void resetType();

	private:
		QString m_currentTypeName;
	};
}