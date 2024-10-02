#include <Qt3DRender/QFrustumCulling>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QTextureImage>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QFrameGraphNode>

#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DExtras/QPhongMaterial>

#include <Qt3DCore/QTransform>
#include <Qt3DCore/QAttribute>

#include <Editor/TextureProcessor.h>
#include <Scene3D/Scene3D.h>

#include <QtLogging>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace scene3d::impl
{
	QImageTextureImage::QImageTextureImage(const QImage &image, Qt3DCore::QNode *parent)
	    : Qt3DRender::QAbstractTextureImage(parent), m_image(image)
	{
	}

	Qt3DRender::QTextureImageDataGeneratorPtr QImageTextureImage::dataGenerator() const
	{
		return Qt3DRender::QTextureImageDataGeneratorPtr(new QImageTextureDataGenerator(m_image));
	}

	QImageTextureDataGenerator::QImageTextureDataGenerator(const QImage &image)
	    : m_image(image)
	{
	}

	Qt3DRender::QTextureImageDataPtr QImageTextureDataGenerator::operator()()
	{
		auto data = Qt3DRender::QTextureImageDataPtr::create();
		data->setImage(m_image);
		return data;
	}

	bool QImageTextureDataGenerator::operator==(const Qt3DRender::QTextureImageDataGenerator &other) const
	{
		const auto *otherFunctor = functor_cast<QImageTextureDataGenerator>(&other);
		return otherFunctor && m_image == otherFunctor->m_image;
	}

	qintptr QImageTextureDataGenerator::id() const
	{
		return reinterpret_cast<qintptr>(this);
	}

	TEXEntryTextureImage::TEXEntryTextureImage(uint32_t texIndex, const gamelib::Level* pLevel, Qt3DCore::QNode* parent)
	    : Qt3DRender::QAbstractTextureImage(parent), m_texIndex(texIndex), m_pLevel(pLevel)
	{
	}

	Qt3DRender::QTextureImageDataGeneratorPtr TEXEntryTextureImage::dataGenerator() const
	{
		if (!m_pLevel || !m_texIndex) return nullptr;

		return Qt3DRender::QTextureImageDataGeneratorPtr(
		    new TEXEntryDataGenerator(m_texIndex, m_pLevel)
		);
	}

	TEXEntryDataGenerator::TEXEntryDataGenerator(uint32_t texIndex, const gamelib::Level* pLevel)
	    : m_texIndex(texIndex), m_pLevel(pLevel)
	{
	}

	Qt3DRender::QTextureImageDataPtr TEXEntryDataGenerator::operator()()
	{
		uint16_t w { 0 }, h { 0 };

		for (const auto& sTexEntry : m_pLevel->getSceneTextures()->entries)
		{
			if (sTexEntry.m_index == m_texIndex)
			{
				std::unique_ptr<std::uint8_t[]> decompressedMemBlk = editor::TextureProcessor::decompressRGBA(sTexEntry, w, h, 0 /*mipLevel*/);
				if (!decompressedMemBlk)
				{
					return {};
				}

				auto data = Qt3DRender::QTextureImageDataPtr::create();
				data->setWidth(w);
				data->setHeight(h);
				data->setFormat(QOpenGLTexture::TextureFormat::RGBA8_UNorm);
				data->setPixelFormat(QOpenGLTexture::RGBA);
				data->setPixelType(QOpenGLTexture::UInt8);
				
				QByteArray imageData(reinterpret_cast<const char*>(decompressedMemBlk.get()), w * h * 4);
				data->setData(imageData, 0);

				return data;
			}
		}

		return nullptr;
	}

	bool TEXEntryDataGenerator::operator==(const Qt3DRender::QTextureImageDataGenerator &other) const
	{
		const auto *otherFunctor = functor_cast<TEXEntryDataGenerator>(&other);
		return otherFunctor && m_texIndex == otherFunctor->m_texIndex && m_pLevel == otherFunctor->m_pLevel;
	}

	qintptr TEXEntryDataGenerator::id() const
	{
		return m_texIndex;
	}
}

namespace scene3d::debug
{
	void printEntityTree(Qt3DCore::QEntity* entity, int depth = 0)
	{
		if (!entity) return;

		QString indent = QString(depth * 2, ' ');
		qDebug() << indent << "Entity at depth" << depth << ":" << entity;

		const auto& components = entity->components();
		for (const auto* component : components) {
			qDebug() << indent << "  Component:" << component;
		}

		const auto& children = entity->children();
		for (const auto* child : children)
		{
			if (const auto* childEntity = dynamic_cast<const Qt3DCore::QEntity*>(child))
			{
				printEntityTree(const_cast<Qt3DCore::QEntity*>(childEntity), depth + 1);
			}
		}
	}
}

namespace scene3d
{
#pragma pack(push, 1)
	struct GenericVertex
	{
		glm::vec3 vPos { 0.f };
		glm::vec2 vUV { 0.f };

		static constexpr int iPosOffset    = 0 * sizeof(float);
		static constexpr int iUVOffset     = decltype(vPos)::length() * sizeof(float);
		static constexpr int iByteStride   = sizeof(float) * (decltype(vPos)::length() + decltype(vUV)::length());
	};
#pragma pack(pop)

	Qt3DRender::QTexture2D* makeTextureFromQImage(const QImage& image, Qt3DCore::QNode* pParent = nullptr)
	{
		auto* texture = new Qt3DRender::QTexture2D(pParent);
		QImage formattedImage = image.convertToFormat(QImage::Format_RGBA8888);

		auto* textureImage = new impl::QImageTextureImage(formattedImage, pParent);

		texture->addTextureImage(textureImage);
		texture->setMinificationFilter(Qt3DRender::QTexture2D::LinearMipMapLinear);
		texture->setMagnificationFilter(Qt3DRender::QTexture2D::Linear);
		texture->setWrapMode(Qt3DRender::QTextureWrapMode(Qt3DRender::QTextureWrapMode::Repeat));

		return texture;
	}

	GameScene::GameScene(Qt3DCore::QNode *parent)
	    : Qt3DCore::QEntity(parent)
	{
		m_pCameraController = new modmesh::RFirstPersonCameraController(this);
		m_pCameraController->setLinearSpeed(10.0f);
		m_pCameraController->setLookSpeed(5.0f);
	}

	modmesh::RCameraController *GameScene::getCameraController() const
	{ 
		return m_pCameraController; 
	}

	void GameScene::setCameraController(modmesh::RCameraController *controller)
	{
		m_pCameraController->deleteLater();
		m_pCameraController = controller;
	}

	GameSceneView::GameSceneView(QWidget *parent)
	    : QWidget(parent)
	    , m_pWindow(std::make_unique<Qt3DExtras::Qt3DWindow>())
	    , m_pContainer(createWindowContainer(m_pWindow.get(), this, Qt::Widget))
		, m_pScene(new GameScene())
	{
		m_pCamera = m_pWindow->camera();
		m_pCamera->setNearPlane(0.1f);
		m_pCamera->setFarPlane(10'000.f);
		m_pCamera->setProjectionType(Qt3DRender::QCameraLens::ProjectionType::PerspectiveProjection);
		m_pWindow->setRootEntity(m_pScene);
		m_pScene->getCameraController()->setCamera(m_pWindow->camera());
		m_pScene->getCameraController()->reset();
	}

	GameSceneView::~GameSceneView()
	{
		delete m_pScene;

		dropSceneHierarchy();
	}

	void GameSceneView::setLevel(gamelib::Level *pLevel)
	{
		if (pLevel != m_pLevel)
		{
			m_pLevel = pLevel;
			buildSceneHierarchy();
		}
	}

	void GameSceneView::resetLevel()
	{
		m_pLevel = nullptr;
		dropSceneHierarchy();
	}

	void GameSceneView::setSelectedObject(gamelib::scene::SceneObject* pObject)
	{}

	void GameSceneView::resetSelectedObject()
	{}

	void GameSceneView::moveCameraTo(const QVector3D &vPosition)
	{
		if (m_pCamera)
		{
			m_pCamera->translateWorld(vPosition);
		}
	}

	void GameSceneView::resizeEvent(QResizeEvent *event)
	{
		if (m_pWindow) m_pWindow->resize(event->size());
		if (m_pCamera) m_pCamera->setAspectRatio(float(width()) / float(height()));
		if (m_pContainer) m_pContainer->resize(event->size());

		QWidget::resizeEvent(event);
	}

	void GameSceneView::gameObjectPropertyChanged(gamelib::scene::SceneObject *pObject, const QString &propertyId)
	{
		// TODO: Impl me
	}

	void GameSceneView::buildSceneHierarchy()
	{
		if (!m_pLevel) return; // nothing to build here

		// anyway need to drop them all
		dropSceneHierarchy();

		m_pRoot = new Qt3DCore::QEntity(m_pScene);

		// Textures
		buildTextureCache();

		// Geometry
		buildGeometryCache();

		// Store ROOT into cache
		m_mObjectToEntity.clear();
		m_mObjectToEntity.insert("ROOT", m_pRoot);

		// Visit scene, build objects and convert to
		using VR = game_scene::SceneObject::EVisitResult;
		if (!m_pLevel->getSceneObjects().empty())
		{
			m_pLevel->getSceneObjects()[0]->visitChildren([this](const game_scene::SceneObject::Ptr& pObject) -> VR {
				auto id = QString::fromStdString(pObject->getGeomREF(false));
				auto parentID = QString::fromStdString(pObject->getParent().lock()->getGeomREF(false));

				auto parentIt = m_mObjectToEntity.find(parentID);
				if (parentIt == m_mObjectToEntity.end())
				{
					// Wtf???
					qWarning() << "At entity " << id << " we have unknown parent " << parentID << " and it's weird. Subtree skipped";
					return VR::VR_NEXT; // Skip subtree
				}

				auto entity = new Qt3DCore::QEntity(parentIt.value());

				// Store cache
				m_mObjectToEntity.insert(id, entity);

				// Setup entity
				setupEntity(entity, pObject);

				// Always next
				return VR::VR_CONTINUE;
			});
		}

		// dbg dump
		debug::printEntityTree(m_pScene);
	}

	void GameSceneView::dropSceneHierarchy()
	{
		// Enough here because it will be freed after m_pRoot
		m_mObjectToEntity.clear();
		m_mPrimIdToGeometry.clear();
		m_mTexNameToTexId.clear();

		// Textures
		m_mTexIdToTexture.clear();

		// Others
		m_pUnsupportedMaterialTexture = nullptr;
		m_pMissingTexture = nullptr;

		if (m_pRoot)
		{
			m_pRoot->deleteLater();
			m_pRoot = nullptr;
		}
	}

	void GameSceneView::buildTextureCache()
	{
		for (const auto& sTexture : m_pLevel->getSceneTextures()->entries)
		{
			if (sTexture.m_mipLevels.empty())
			{
				continue;
			}

			auto* pTexture = new Qt3DRender::QTexture2D(m_pRoot);
			pTexture->setWidth(sTexture.m_width);
			pTexture->setHeight(sTexture.m_height);
			pTexture->setFormat(Qt3DRender::QTexture2D::TextureFormat::RGBA8_UNorm);
			pTexture->setMipLevels(static_cast<int>(sTexture.m_mipLevels.size()));
			pTexture->setWrapMode(Qt3DRender::QTextureWrapMode(Qt3DRender::QTextureWrapMode::Repeat));
			pTexture->setMinificationFilter(Qt3DRender::QTexture2D::Linear);
			pTexture->setMagnificationFilter(Qt3DRender::QTexture2D::Linear);

			// Texture data
			auto* pTextureImage = new impl::TEXEntryTextureImage(sTexture.m_index, m_pLevel, pTexture);
			pTexture->addTextureImage(pTextureImage);

			// Store
			m_mTexIdToTexture[sTexture.m_index] = pTexture;
			if (sTexture.m_fileName.has_value())
			{
				m_mTexNameToTexId[sTexture.m_fileName.value()] = sTexture.m_index;
			}
		}

		{
			QImage missingTextureImage = QImage(":/bmedit/mtl_missing_texture.png").convertToFormat(QImage::Format_RGBA8888, Qt::AutoColor);
			m_pMissingTexture = makeTextureFromQImage(missingTextureImage, m_pRoot);
		}

		{
			QImage unsupportedMaterialImage = QImage(":/bmedit/mtl_unsupported.png").convertToFormat(QImage::Format_RGBA8888, Qt::AutoColor);
			m_pUnsupportedMaterialTexture = makeTextureFromQImage(unsupportedMaterialImage, m_pRoot);
		}
	}

	void GameSceneView::buildGeometryCache()
	{
		auto isValidVertexFormat = [](gamelib::prm::VertexFormat vf) -> bool
		{
			using VF = gamelib::prm::VertexFormat;
			return vf == VF::VF_10 || vf == VF::VF_24 || vf == VF::VF_28 || vf == VF::VF_34;
		};

		for (const auto& sModel : m_pLevel->getLevelGeometry()->primitives.models)
		{
			const auto iChunkIdx = sModel.chunk;

			if (sModel.meshes.empty())
			{
				m_mPrimIdToGeometry[iChunkIdx] = {};
				continue;
			}

			PrimitiveInfo& sInfo = m_mPrimIdToGeometry[iChunkIdx];

			// Lookup mesh
			int iMeshId = 0;
			for (const auto& sMesh : sModel.meshes)
			{
				++iMeshId;
				if (sMesh.vertices.empty() /*|| !isValidVertexFormat(sMesh.vertexFormat)*/)
				{
					// create empty mesh
					if (sMesh.vertices.empty())
					{
						qWarning() << "Skip mesh #" << (iMeshId - 1) << " for model #" << sModel.chunk << " (empty vertices)";
					}

//					if (!isValidVertexFormat(sMesh.vertexFormat) && sMesh.vertexFormat != gamelib::prm::VertexFormat::VF_ERROR /* no need to log VF_ERROR here*/)
//					{
//						qWarning() << "Skip mesh #" << (iMeshId - 1) << " for model #" << sModel.chunk << " (INVALID VERTEX FORMAT " << static_cast<int>(sMesh.vertexFormat) << ")";
//					}
					continue;
				}

				auto *pGeometry = new Qt3DCore::QGeometry(m_pRoot);

				// Make mem great again
				auto* pMeshBuffer = new Qt3DCore::QBuffer(pGeometry);
				size_t vertexCount = sMesh.vertices.size();
				size_t vertexDataSize = vertexCount * GenericVertex::iByteStride;

				QByteArray vertexData;
				vertexData.resize(static_cast<qsizetype>(vertexDataSize));

				auto* vertexDataPtr = reinterpret_cast<GenericVertex*>(vertexData.data());

				for (size_t i = 0; i < vertexCount; ++i)
				{
					float *pData = reinterpret_cast<float*>(vertexData.data() + (GenericVertex::iByteStride * i));

					pData[0] = sMesh.vertices[i].x;
					pData[1] = sMesh.vertices[i].y;
					pData[2] = sMesh.vertices[i].z;
					pData[3] = sMesh.uvs.empty() ? 0.f : sMesh.uvs[i].x;
					pData[4] = sMesh.uvs.empty() ? 0.f : sMesh.uvs[i].y;
				}

				pMeshBuffer->setData(vertexData);

				// Indices
				auto* pIndexBuffer = new Qt3DCore::QBuffer(pGeometry);

				size_t indexCount = sMesh.indices.size() * 3;
				size_t indexDataSize = indexCount * sizeof(std::uint16_t);

				QByteArray indexData;
				indexData.resize(static_cast<qsizetype>(indexDataSize));
				auto* indexDataPtr = reinterpret_cast<std::uint16_t*>(indexData.data());

				// Fill mem
				size_t index = 0;
				for (const auto& [a, b, c] : sMesh.indices)
				{
					// lmao
					//indexDataPtr[index++] = a;
					//indexDataPtr[index++] = b;
					//indexDataPtr[index++] = c;

					// not lmao?
					indexDataPtr[index++] = a;
					indexDataPtr[index++] = c;
					indexDataPtr[index++] = b;
				}

				pIndexBuffer->setData(indexData);

				// Setup vertex format
				// Position
				auto *positionAttribute = new Qt3DCore::QAttribute(pGeometry);
				positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
				positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
				positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
				
				positionAttribute->setVertexSize(decltype(GenericVertex::vPos)::length());
				positionAttribute->setByteOffset(GenericVertex::iPosOffset);
				positionAttribute->setByteStride(GenericVertex::iByteStride);
				
				positionAttribute->setCount(static_cast<int>(vertexCount));
				positionAttribute->setBuffer(pMeshBuffer);

				// UV
				auto *uvAttribute = new Qt3DCore::QAttribute(pGeometry);
				uvAttribute->setName(Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName());
				uvAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
				uvAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);

				uvAttribute->setVertexSize(decltype(GenericVertex::vUV)::length());
				uvAttribute->setByteOffset(GenericVertex::iUVOffset);
				uvAttribute->setByteStride(GenericVertex::iByteStride);

				uvAttribute->setCount(static_cast<int>(vertexCount));
				uvAttribute->setBuffer(pMeshBuffer);

				// Indices attr
				auto *indexAttribute = new Qt3DCore::QAttribute(pGeometry);
				indexAttribute->setVertexBaseType(Qt3DCore::QAttribute::UnsignedShort);
				indexAttribute->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
				indexAttribute->setBuffer(pIndexBuffer);
				indexAttribute->setCount(static_cast<int>(indexCount));

				// Finish geometry
				pGeometry->addAttribute(positionAttribute);
				pGeometry->setBoundingVolumePositionAttribute(positionAttribute);
				pGeometry->addAttribute(uvAttribute);
				pGeometry->addAttribute(indexAttribute);

				if (sMesh.material_id > 0)
				{
					const auto& instances = m_pLevel->getLevelMaterials()->materialInstances;
					const auto& matInstance = instances[sMesh.material_id - 1];

					if (const auto& parentName = matInstance.getParentName(); parentName == "StaticShadow" || parentName == "StaticShadowTextureShadow" || matInstance.getName().find("AlwaysInShadow") != std::string::npos)
					{
						// Nothing?
					}
					else if (parentName == "Bad")
					{
//						auto* pMaterial = new Qt3DExtras::QTextureMaterial(m_pRoot);
//						pMaterial->setTexture(m_pUnsupportedMaterialTexture);
//						sInfo.vpMaterials.emplace_back(pMaterial);
//						bMaterialInstanced = true;
					}
					else
					{
						bool bTextureFound = false;

						for (const auto& binder : matInstance.getBinders())
						{
							if (bTextureFound)
								break;

							for (const auto& texture : binder.textures)
							{
								if (bTextureFound)
									break;

								if (texture.getName() == "mapDiffuse" && (texture.getTextureId() != 0 || !texture.getTexturePath().empty()))
								{
									switch (texture.getPresentedTextureSources())
									{
										case gamelib::mat::PresentedTextureSource::PTS_NOTHING:
											continue;  // Nothing

										case gamelib::mat::PresentedTextureSource::PTS_TEXTURE_ID:
										{
											// Only texture id
										    if (auto it = m_mTexIdToTexture.find(texture.getTextureId()); it != m_mTexIdToTexture.end())
										    {
											    MeshInfo& mi = sInfo.vMeshes.emplace_back();
											    mi.pGeometry = pGeometry;
											    mi.pTexture = it->second;
											    mi.bAlphaBlendEnabled = binder.renderStates.empty() ? false : binder.renderStates[0].isBlendEnabled();

											    bTextureFound = true;
										    }
										}
										break;
										case gamelib::mat::PresentedTextureSource::PTS_TEXTURE_PATH:
										{
											// Only path
										    if (auto texNameToIdIt = m_mTexNameToTexId.find(texture.getTexturePath()); texNameToIdIt != m_mTexNameToTexId.end())
										    {
											    if (auto it = m_mTexIdToTexture.find(texNameToIdIt->second); it != m_mTexIdToTexture.end())
											    {
												    MeshInfo& mi = sInfo.vMeshes.emplace_back();
												    mi.pGeometry = pGeometry;
												    mi.pTexture = it->second;
												    mi.bAlphaBlendEnabled = binder.renderStates.empty() ? false : binder.renderStates[0].isBlendEnabled();

													bTextureFound = true;
											    }
										    }
										}
										break;
										default:
										{
											// Bad case! Undefined behaviour!
											assert(false && "Impossible case!");
											continue;
										}
									}
								}
							}
						}

						if (!bTextureFound)
						{
							// Use missing texture
						}
					}
				}
				else if (sMesh.textureId > 0)
				{
					// Sprites: each ZWinPIC has textureId reference
					if (auto it = m_mTexIdToTexture.find(sMesh.textureId); it != m_mTexIdToTexture.end())
					{
						MeshInfo& mi = sInfo.vMeshes.emplace_back();
						mi.pGeometry = pGeometry;
						mi.pTexture = it->second;
					}
				}
			}
		}
	}

	void GameSceneView::setupEntity(Qt3DCore::QEntity *pEntity, const game_scene::SceneObject::Ptr &pObject)
	{
		// Take primary parameters
		const int iPrimId = pObject->getProperties().getObject<int>("PrimId", 0);
		const auto mLocalTransform = pObject->getLocalTransform();
		
		// View & geometry
		if (iPrimId)
		{
			// We have a view here
			if (auto primIt = m_mPrimIdToGeometry.find(iPrimId); primIt != m_mPrimIdToGeometry.end())
			{
				for (const auto& sMesh : primIt->second.vMeshes)
				{
					auto* pMeshEntity = new Qt3DCore::QEntity(pEntity);

					auto* pRenderer = new Qt3DRender::QGeometryRenderer(pMeshEntity);
					pRenderer->setGeometry(sMesh.pGeometry);
					pRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);

					auto* pMaterial = new Qt3DExtras::QTextureMaterial(pMeshEntity);
					pMaterial->setTexture(sMesh.pTexture);
					pMaterial->setAlphaBlendingEnabled(sMesh.bAlphaBlendEnabled);

					// add components
					pMeshEntity->addComponent(pRenderer);
					pMeshEntity->addComponent(pMaterial);
				}
			}
			else qDebug() << "PrimId " << iPrimId << " NOT FOUND";
		}

		// Transform matrix
		{
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(mLocalTransform, scale, rotation, translation, skew, perspective);

			QVector3D qtTranslation(translation.x, translation.y, translation.z);
			QVector3D qtScale(scale.x, scale.y, scale.z);
			QQuaternion qtRotation(rotation.w, rotation.x, rotation.y, rotation.z);

			auto *pTransform = new Qt3DCore::QTransform(pEntity);
			pTransform->setTranslation(qtTranslation);
			pTransform->setScale3D(qtScale);
			pTransform->setRotation(qtRotation);

			pEntity->addComponent(pTransform);
		}
	}
}