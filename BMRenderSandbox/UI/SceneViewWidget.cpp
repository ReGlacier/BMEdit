#include <glad/glad.h>
#include <SceneViewWidget.h>

#include <AssetProvider/ZIPLevelAssetProvider.h>
#include <GTIL/GlacierTypeInfoLoader.h>

#include <GameLib/GMS/GMSStructureError.h>
#include <GameLib/PRP/PRPStructureError.h>
#include <GameLib/TypeNotFoundException.h>
#include <GameLib/Scene/SceneObjectVisitorException.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/Level.h>

#include <stdexcept>


struct LoadLevelResultVisitor
{
	SceneViewWidget *owner { nullptr };

	void operator()(std::unique_ptr<gamelib::Level> &&level) const
	{
		owner->m_level = std::move(level);

		owner->m_renderer = std::make_unique<bmr::SimplePrimitiveRender>(owner->m_level.get()); // Give render a 'weak' pointer to level

		bmr::RenderOptions options = owner->getRenderOptions();
		owner->m_renderer->setup(std::move(options));

		emit owner->levelLoaded();
	}

	void operator()(const QString &errorMessage) const
	{
		emit owner->levelLoadFailed(errorMessage);
	}
};

SceneViewWidget::SceneViewWidget(QWidget *parent) : QOpenGLWidget(parent)
{
	connect(&m_levelFutureWatcher, &QFutureWatcher<impl::LoadResult>::finished, this, &SceneViewWidget::onLevelLoadFutureFinished);
}

SceneViewWidget::~SceneViewWidget()
{
	makeCurrent(); // See Qt documentation for details

	m_renderer = nullptr;
	m_level = nullptr;

	doneCurrent();
}

impl::LoadResult loadLevelImpl(const QString &levelZip)
{
	// Load level
	auto levelAssetProvider = std::make_unique<ap::ZIPLevelAssetProvider>(levelZip.toStdString());
	auto level = std::make_unique<gamelib::Level>(std::move(levelAssetProvider));

	try
	{
		gamelib::LoadLevelOptions loadOptions = gamelib::LoadLevelOption::LLO_NONE;

		// Disable properties & scene tree loader to make it faster
		loadOptions |= gamelib::LoadLevelOption::LLO_SKIP_PROPERTIES;
		loadOptions |= gamelib::LoadLevelOption::LLO_SKIP_SCENE_TREE;

		if (!level->loadSceneData(loadOptions))
		{
			return { QString("Failed to load scene data") };
		}
		else
		{
			return { std::move(level) };
		}
	}
	catch (const gamelib::gms::GMSStructureError &gmsStructureError)
	{
		return QString("Error in GMS structure: %1").arg(gmsStructureError.what());
	}
	catch (const gamelib::prp::PRPStructureError &prpStructureError)
	{
		return QString("Error in PRP structure: %1").arg(prpStructureError.what());
	}
	catch (const gamelib::TypeNotFoundException &typeNotFoundException)
	{
		return QString("Unable to locate requried type %1").arg(typeNotFoundException.what());
	}
	catch (const gamelib::scene::SceneObjectVisitorException &sceneObjectException)
	{
		return QString("Unable to visit geom on scene: %1").arg(sceneObjectException.what());
	}
	catch (const std::runtime_error &runtimeFailure)
	{
		return QString("RUNTIME ERROR: %1").arg(runtimeFailure.what());
	}
	catch (const std::exception &ex)
	{
		return QString("COMMON EXCEPTION: %1").arg(ex.what());
	}
	catch (...)
	{
		assert(false && "unprocessed routine");

		return QString("Unknown exception");
	}

	return QString("Unknown result");
}

void SceneViewWidget::requestLoadLevel(const QString &pathToLevelZip)
{
	if (m_levelFutureWatcher.isRunning())
	{
		assert(false);
		return;
	}

	m_loadLevelFuture = QtConcurrent::run(loadLevelImpl, pathToLevelZip);

	m_levelFutureWatcher.setFuture(m_loadLevelFuture);
	m_levelName = pathToLevelZip;
}

void SceneViewWidget::setActivePrimitiveIndex(std::uint32_t primitiveIndex)
{
	if (m_renderer && primitiveIndex > 0)
	{
		m_renderer->setPrimitiveIndex(primitiveIndex);
		update();
	}
}

std::uint32_t SceneViewWidget::getActivePrimitiveIndex() const
{
	return m_renderer ? m_renderer->getPrimitiveIndex() : 0u;
}

const QString &SceneViewWidget::getLevelName() const
{
	return m_levelName;
}

void SceneViewWidget::initializeGL()
{
	if (!context()) {
		qCritical() << "Can't get OGL context";
		close();
		return;
	}

	// Setup glad bindings
	if (gladLoadGL() != 1)
	{
		qCritical() << "Failed to initialize glad loader!";
		close();
		return;
	}

	qInfo() << "Detected OpenGL version" << reinterpret_cast<const char *>(glGetString(GL_VERSION));

}

void SceneViewWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}

void SceneViewWidget::paintGL()
{
	glClearColor(.0f, 1.0f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (m_loadLevelFuture.isRunning() || !m_level || !m_renderer)
		return; // Nothing to draw

	m_renderer->drawFrame();
}

void SceneViewWidget::onLevelLoadFutureFinished()
{
	auto result = m_loadLevelFuture.takeResult();

	LoadLevelResultVisitor visitor { this };
	std::visit(visitor, std::move(result));
}

bmr::RenderOptions SceneViewWidget::getRenderOptions() const
{
	return { width(), height(), m_renderer ? m_renderer->getCamera().getFOV() : kDefaultFOV };
}

void SceneViewWidget::setFOV(float fov)
{
	if (m_renderer && fov > .1f)
	{
		m_renderer->getCamera().setFOV(fov);
		update();
	}
}

void SceneViewWidget::setCameraPosition(const glm::vec3 &position)
{
	if (m_renderer)
	{
		m_renderer->getCamera().setCameraPosition(position);
		update();
	}
}

void SceneViewWidget::setCameraLookAt(const glm::vec3 &lookAt)
{
	if (m_renderer)
	{
		m_renderer->getCamera().setCameraLookAt(lookAt);
		update();
	}
}

void SceneViewWidget::setCameraHeadPosition(const glm::vec3 &headPos)
{
	if (m_renderer)
	{
		m_renderer->getCamera().setCameraHeadPosition(headPos);
		update();
	}
}

void SceneViewWidget::setCameraYaw(float yaw)
{
	if (m_renderer)
	{
		m_renderer->getCamera().setCameraYaw(yaw);
		update();
	}
}

void SceneViewWidget::setCameraPitch(float pitch)
{
	if (m_renderer)
	{
		m_renderer->getCamera().setCameraPitch(pitch);
		update();
	}
}

void SceneViewWidget::setCameraRoll(float roll)
{
	if (m_renderer)
	{
		m_renderer->getCamera().setCameraRoll(roll);
		update();
	}
}

void SceneViewWidget::moveCameraPositionOutOfPrimitive()
{
	if (m_renderer && m_renderer->getPrimitiveIndex() != 0u)
	{
		auto& chk = m_level->getLevelGeometry()->chunks.at(m_renderer->getPrimitiveIndex());
		if (chk.getKind() != gamelib::prm::PRMChunkRecognizedKind::CRK_DESCRIPTION_BUFFER)
		{
			return;
		}

		// TODO: Do it better
		const auto &pMax = chk.getDescriptionBufferHeader()->boundingBox.max;
		m_renderer->getCamera().setCameraPosition(glm::vec3 { pMax.x, pMax.y, pMax.z });
	}
}

float SceneViewWidget::getFOV() const
{
	return m_renderer ? m_renderer->getCamera().getFOV() : .0f;
}

const glm::vec3 &SceneViewWidget::getCameraPosition() const
{
	static glm::vec3 kZero { .0f };
	return m_renderer ? m_renderer->getCamera().getCameraPosition() : kZero;
}

const glm::vec3 &SceneViewWidget::getCameraLookAt() const
{
	static glm::vec3 kZero { .0f };
	return m_renderer ? m_renderer->getCamera().getCameraLookAt() : kZero;
}

const glm::vec3 &SceneViewWidget::getCameraHeadPos() const
{
	static glm::vec3 kZero { .0f };
	return m_renderer ? m_renderer->getCamera().getCameraHeadPos() : kZero;
}

float SceneViewWidget::getCameraYaw() const
{
	return m_renderer ? m_renderer->getCamera().getCameraYaw() : .0f;
}

float SceneViewWidget::getCameraPitch() const
{
	return m_renderer ? m_renderer->getCamera().getCameraPitch() : .0f;
}

float SceneViewWidget::getCameraRoll() const
{
	return m_renderer ? m_renderer->getCamera().getCameraRoll() : .0f;
}

gamelib::Level *SceneViewWidget::getGameLevel()
{
	return m_level.get();
}