#pragma once

#include <QObject>


namespace items
{
	class ObjectPropertyItem : public QObject
	{
		Q_OBJECT
	public:
		ObjectPropertyItem(QObject *parent = nullptr); // NOLINT(google-explicit-constructor)

	private:

	};
}