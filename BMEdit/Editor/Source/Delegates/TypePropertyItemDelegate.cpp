#include <Delegates/TypePropertyItemDelegate.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>


namespace delegates
{
	///----------------
	/// CONFIGURATION
	///----------------
	static constexpr int kValueColumnIndex = 1;

	///---------------------------
	/// TypePropertyItemDelegate
	///---------------------------
	TypePropertyItemDelegate::TypePropertyItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
	{
	}
}