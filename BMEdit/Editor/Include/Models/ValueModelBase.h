#pragma once

#include <QAbstractTableModel>
#include <GameLib/Value.h>
#include <optional>


namespace models
{
	class ValueModelBase : public QAbstractTableModel
	{
		Q_OBJECT

	protected:
		enum ColumnID : int {
			NAME = 0,
			VALUE = 1,

			MAX_COLUMNS
		};

	public:
		using QAbstractTableModel::QAbstractTableModel;

		int rowCount(const QModelIndex &parent) const override;
		int columnCount(const QModelIndex &parent) const override;
		QVariant data(const QModelIndex &index, int role) const override;
		bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const override;
		Qt::ItemFlags flags(const QModelIndex &index) const override;

		void setValue(const gamelib::Value &value);
		void resetValue();
		[[nodiscard]] const std::optional<gamelib::Value> &getValue() const;

	signals:
		void valueChanged();

	protected:
		[[nodiscard]] bool isReady() const;

	private:
		std::optional<gamelib::Value> m_value;
	};
}