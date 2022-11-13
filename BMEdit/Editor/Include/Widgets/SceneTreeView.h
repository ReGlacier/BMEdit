#pragma once

#include <QTreeView>


namespace widgets
{
	class SceneTreeView : public QTreeView
	{
		Q_OBJECT
	public:
		using QTreeView::QTreeView;

	protected:
		void mousePressEvent(QMouseEvent *event) override;
	};
}