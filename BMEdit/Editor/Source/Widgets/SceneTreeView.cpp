#include <Widgets/SceneTreeView.h>
#include <QMouseEvent>

using namespace widgets;

void SceneTreeView::mousePressEvent(QMouseEvent *event)
{
	// It was made to avoid selection by the right click
	if (event->button() != Qt::RightButton)
	{
		QTreeView::mousePressEvent(event);
	}
}