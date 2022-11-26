#pragma once

#include <GameLib/Level.h>
#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject> // VAO
#include <QOpenGLBuffer> // Generic buffer
#include <cstdint>


namespace widgets
{
	class PrimitivePreviewWidget : public QOpenGLWidget
	{
		Q_OBJECT

		using Base = QOpenGLWidget;

	public:
		using QOpenGLWidget::QOpenGLWidget;

		void setLevel(const gamelib::Level* level);
		void resetLevel();

		void setPrimitiveIndex(std::uint32_t primitiveIndex);
		void resetPrimitiveIndex();

	protected:
		void initializeGL() override;
		void paintGL() override;
		void resizeGL(int w, int h) override;

	private:
		void doPreloadNewPrimitive();
		void doDrawCurrentPrimitive();

	private:
		const gamelib::Level *m_level { nullptr };
		std::uint32_t m_primitiveIndex { 0 };
		bool m_doPreloadNewPrimitive { false };
	};
}