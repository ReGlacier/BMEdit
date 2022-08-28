#include <GameLib/Scene/SceneObjectPropertiesDumper.h>
#include <GameLib/Scene/SceneObject.h>
#include <GameLib/PRP/PRPInstruction.h>
#include <GameLib/PRP/PRPZDefines.h>
#include <GameLib/PRP/PRPWriter.h>
#include <GameLib/Level.h>
#include <cassert>

using namespace gamelib::scene;
using namespace gamelib::prp;


namespace gamelib::scene
{
	struct SceneObjectPropertiesDumper::DumperContext
	{
		DumperContext() = default;
		~DumperContext() = default;

		const Level *level { nullptr };
		std::vector<uint8_t> *outBuffer { nullptr };
		std::vector<PRPInstruction> instructions {};
	};
}

SceneObjectPropertiesDumper::SceneObjectPropertiesDumper() = default;
SceneObjectPropertiesDumper::~SceneObjectPropertiesDumper() = default;

void SceneObjectPropertiesDumper::dump(const gamelib::Level *level, std::vector<uint8_t> *outBuffer)
{
	if (!level || !outBuffer)
	{
		assert(level != nullptr && "Bad level instance");
		assert(outBuffer != nullptr && "Bad out buffer instance");
		return;
	}

	m_localContext = std::make_unique<SceneObjectPropertiesDumper::DumperContext>();
	m_localContext->level = level;
	m_localContext->outBuffer = outBuffer;

	std::vector<prp::PRPInstruction> generatedInstructions;
	if (!m_localContext->level->getSceneObjects().empty())
	{
		visitSceneObject(m_localContext->level->getSceneObjects()[0].get()); // Take root and start visitor
	}

	m_localContext->instructions.reserve(2); // Add extra instructions
	m_localContext->instructions.emplace_back(PRPOpCode::Bool, PRPOperandVal(false)); // Unknown tag, but it needs to be here
	m_localContext->instructions.emplace_back(PRPOpCode::EndOfStream);

	prp::PRPWriter::write(
	    m_localContext->level->getLevelProperties()->ZDefines,
	    m_localContext->instructions,
	    m_localContext->level->getLevelProperties()->header.isRaw(),
	    *outBuffer);
}

void SceneObjectPropertiesDumper::visitSceneObject(const SceneObject *sceneObject)
{
	if (!sceneObject)
	{
		assert(false && "Bad sceneObjectInstance!");
		return;
	}

	if (!m_localContext)
	{
		assert(false && "Local context must be initialized here!");
		return;
	}

	auto& ctx = *m_localContext;
	auto& out = ctx.instructions;

	{
		// Properties
		out.reserve(2 + sceneObject->getProperties().getInstructions().size());
		out.emplace_back(PRPOpCode::BeginObject);
		for (const auto& instruction: sceneObject->getProperties().getInstructions())
		{
			out.emplace_back(instruction);
		}
		out.emplace_back(PRPOpCode::EndObject);
	}

	{
		// Controllers
		out.emplace_back(PRPInstruction(PRPOpCode::Container, PRPOperandVal(static_cast<int>(sceneObject->getControllers().size()))));

		for (const auto& [name, properties] : sceneObject->getControllers())
		{
			out.reserve(3 + properties.getInstructions().size());

			out.emplace_back(PRPOpCode::String, PRPOperandVal(name));
			out.emplace_back(PRPOpCode::BeginObject);

			for (const auto& instruction : properties.getInstructions())
			{
				out.emplace_back(instruction);
			}

			out.emplace_back(PRPOpCode::EndObject);
		}
	}

	{
		// Children
		out.reserve(1);

		out.emplace_back(PRPOpCode::Container, PRPOperandVal(static_cast<int>(sceneObject->getChildren().size())));

		for (const auto& childRef : sceneObject->getChildren())
		{
			auto child = childRef.lock();
			if (!child)
			{
				assert(false && "Invalid child instance");
				return;
			}

			visitSceneObject(child.get());
		}
	}
}