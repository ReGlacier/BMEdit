#include <Widgets/PrimitivePreviewWidget.h>

#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVersionFunctions>


using namespace widgets;

void PrimitivePreviewWidget::setLevel(const gamelib::Level* level)
{
	if (level)
	{
		m_level = level;
	}
}

void PrimitivePreviewWidget::resetLevel()
{
	m_level = nullptr;
	m_primitiveIndex = 0u;
}

void PrimitivePreviewWidget::setPrimitiveIndex(std::uint32_t primitiveIndex)
{
	if (m_primitiveIndex != primitiveIndex)
	{
		m_primitiveIndex = primitiveIndex;
		m_doPreloadNewPrimitive = true;
	}
}

void PrimitivePreviewWidget::resetPrimitiveIndex()
{
	m_primitiveIndex = 0u;
}

void PrimitivePreviewWidget::initializeGL()
{
	Base::initializeGL();

	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 3); // Use OpenGL 3.3 Core profile
	format.setProfile(QSurfaceFormat::CoreProfile);
	setFormat(format);
}

void PrimitivePreviewWidget::paintGL()
{
	Base::paintGL();

	if (!m_level || !m_primitiveIndex) return;

    auto glFunctions = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
	if (!glFunctions)
	{
		throw std::runtime_error { "Current GPU does not support OpenGL 3.3 Core Profile" };
	}

	if (m_doPreloadNewPrimitive)
	{
		doPreloadNewPrimitive();
		m_doPreloadNewPrimitive = false;
	}

	doDrawCurrentPrimitive();
}

void PrimitivePreviewWidget::resizeGL(int w, int h)
{
	Base::resizeGL(w, h);
}

void PrimitivePreviewWidget::doPreloadNewPrimitive()
{
	auto& chk = m_level->getLevelGeometry()->chunks.at(m_primitiveIndex);
	if (chk.getKind() != gamelib::prm::PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER)
	{
		// Invalid case: we've unable to draw model by non-descriptor index
		m_primitiveIndex = 0u;
		return;
	}

	const auto descriptionHeader = chk.getDescriptionBufferHeader();
	//TODO: Extract index buffer, extract vertex buffer, validate index buffer, validate vertex buffer.
}

void PrimitivePreviewWidget::doDrawCurrentPrimitive()
{
}