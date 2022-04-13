#pragma once

#include <QStyledItemDelegate>



namespace delegates
{
	class TypePropertyItemDelegate : public QStyledItemDelegate
	{
		Q_OBJECT
	public:
		TypePropertyItemDelegate(QObject *parent = nullptr);
	};
}