#pragma once

#include <BMRender/SimplePrimitiveRender.h>
#include <QtConcurrent/QtConcurrent>
#include <GameLib/Level.h>
#include <QOpenGLWidget>
#include <glm/vec3.hpp>
#include <cstdint>
#include <memory>
#include <variant>


namespace impl
{
	using LoadResult = std::variant<QString, std::unique_ptr<gamelib::Level>>;
}

class SceneViewWidget : public QOpenGLWidget 
{
	Q_OBJECT

	friend struct LoadLevelResultVisitor;
	static constexpr float kDefaultFOV = 60.f;

public:
	explicit SceneViewWidget(QWidget *parent = nullptr);
	~SceneViewWidget();

	// Level stuff
	void requestLoadLevel(const QString &pathToLevelZip);
	[[nodiscard]] const QString &getLevelName() const;
	void setActivePrimitiveIndex(std::uint32_t primitiveIndex);
	std::uint32_t getActivePrimitiveIndex() const;

	// Camera stuff
	void setFOV(float fov);
	void setCameraPosition(const glm::vec3 &position);
	void setCameraLookAt(const glm::vec3 &lookAt);
	void setCameraHeadPosition(const glm::vec3 &headPos);
	void setCameraYaw(float yaw);
	void setCameraPitch(float pitch);
	void setCameraRoll(float roll);
	void moveCameraPositionOutOfPrimitive();

	[[nodiscard]] float getFOV() const;
	[[nodiscard]] const glm::vec3 &getCameraPosition() const;
	[[nodiscard]] const glm::vec3 &getCameraLookAt() const;
	[[nodiscard]] const glm::vec3 &getCameraHeadPos() const;
	[[nodiscard]] float getCameraYaw() const;
	[[nodiscard]] float getCameraPitch() const;
	[[nodiscard]] float getCameraRoll() const;

	[[nodiscard]] gamelib::Level *getGameLevel();

signals:
	void levelLoaded();
	void levelLoadFailed(const QString &errorMessage);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

private slots:
	void onLevelLoadFutureFinished();

private:
	bmr::RenderOptions getRenderOptions() const;

private:
	QFuture<impl::LoadResult> m_loadLevelFuture {};
	QFutureWatcher<impl::LoadResult> m_levelFutureWatcher;
	QString m_levelName;
	QString m_loadLevelError;

	std::unique_ptr<bmr::SimplePrimitiveRender> m_renderer{ nullptr };
	std::unique_ptr<gamelib::Level> m_level{nullptr};
};