#pragma once

// Qt3D
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QEntityPtr>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QGeometry>

#include <Qt3DExtras/Qt3DWindow>

#include <Qt3DRender/QTextureImageDataGenerator>
#include <Qt3DRender/QAbstractTextureImage>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QTexture>

// modmesh
#include <Scene3D/RCameraController.hpp>

// GameLib
#include <GameLib/Scene/SceneObject.h>
#include <GameLib/TEX/TEXEntry.h>
#include <GameLib/Level.h>

// Qt etc
#include <QMouseEvent>
#include <QKeyEvent>
#include <QString>
#include <QWidget>
#include <QMap>

// STL
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <vector>


namespace scene3d
{
	namespace impl
	{
		class QImageTextureImage final : public Qt3DRender::QAbstractTextureImage
		{
			Q_OBJECT

		public:
			QImageTextureImage(const QImage& image, Qt3DCore::QNode* parent = nullptr);

			Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override;

		private:
			QImage m_image;
		};

		class QImageTextureDataGenerator : public Qt3DRender::QTextureImageDataGenerator
		{
		public:
			explicit QImageTextureDataGenerator(const QImage& image);

			Qt3DRender::QTextureImageDataPtr operator()() override;

			bool operator==(const Qt3DRender::QTextureImageDataGenerator &other) const override;
			qintptr id() const override;

		private:
			QImage m_image;
		};

		class TEXEntryTextureImage final : public Qt3DRender::QAbstractTextureImage
		{
			Q_OBJECT

		public:
			explicit TEXEntryTextureImage(uint32_t texIndex, const gamelib::Level* pLevel, Qt3DCore::QNode* parent = nullptr);

		protected:
			Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override;

		private:
			uint32_t m_texIndex;
			const gamelib::Level* m_pLevel;
		};

		class TEXEntryDataGenerator final : public Qt3DRender::QTextureImageDataGenerator
		{
		public:
			explicit TEXEntryDataGenerator(uint32_t texIndex, const gamelib::Level* pLevel);
			~TEXEntryDataGenerator() override = default;

			Qt3DRender::QTextureImageDataPtr operator()() override;
			bool operator==(const Qt3DRender::QTextureImageDataGenerator &other) const override;
			qintptr id() const override;

		private:
			uint32_t m_texIndex;
			const gamelib::Level* m_pLevel;
		};
	}

	namespace game_scene = gamelib::scene;

	class GameScene : public Qt3DCore::QEntity
	{
		modmesh::RCameraController* m_pCameraController { nullptr };

	public:
		explicit GameScene(Qt3DCore::QNode *parent = nullptr);

		modmesh::RCameraController *getCameraController() const;
		void setCameraController(modmesh::RCameraController *controller);
	};

	class GameSceneView : public QWidget
	{
		Q_OBJECT

	private:
		// Main
		std::unique_ptr<Qt3DExtras::Qt3DWindow> m_pWindow { nullptr };
		QWidget* m_pContainer { nullptr };
		GameScene* m_pScene { nullptr };
		Qt3DCore::QEntity* m_pRoot { nullptr };

		// Owned by Qt or somebody else
		Qt3DRender::QCamera* m_pCamera { nullptr };
		gamelib::Level* m_pLevel { nullptr };

		// Scene to Qt3D scene
		QMap<QString, Qt3DCore::QEntity*> m_mObjectToEntity {};

		// Geometry
		struct MeshInfo
		{
			Qt3DCore::QGeometry* pGeometry { nullptr };
			Qt3DRender::QTexture2D* pTexture { nullptr }; // primary texture
			bool bAlphaBlendEnabled{false};
		};

		struct PrimitiveInfo
		{
			std::vector<MeshInfo> vMeshes {};
		};

		std::unordered_map<uint32_t, PrimitiveInfo> m_mPrimIdToGeometry {};

		// Textures
		std::unordered_map<uint32_t, Qt3DRender::QTexture2D*> m_mTexIdToTexture {};
		std::unordered_map<std::string, uint32_t> m_mTexNameToTexId {};
		Qt3DRender::QTexture2D* m_pMissingTexture {};
		Qt3DRender::QTexture2D* m_pUnsupportedMaterialTexture {};

		// Camera options
		float m_fFOV { 80.f };

	public:
		explicit GameSceneView(QWidget* parent);
		virtual ~GameSceneView() override;

		void setLevel(gamelib::Level* pLevel);
		void resetLevel();

		void setSelectedObject(gamelib::scene::SceneObject* pObject);
		void resetSelectedObject();

		void moveCameraTo(const QVector3D& vPosition);

	protected:
		void resizeEvent(QResizeEvent *event) override;

	public slots:
		void gameObjectPropertyChanged(gamelib::scene::SceneObject* pObject, const QString& propertyId);

	private:
		void buildSceneHierarchy();
		void dropSceneHierarchy();
		void buildTextureCache();
		void buildGeometryCache();
		void setupEntity(Qt3DCore::QEntity* pEntity, const game_scene::SceneObject::Ptr& pObject);
	};
}