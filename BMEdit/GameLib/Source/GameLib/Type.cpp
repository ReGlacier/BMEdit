#include <GameLib/Type.h>
#include <GameLib/NotImplemented.h>

#include <utility>


namespace gamelib
{
	Type::Type(TypeKind kind, std::string name)
		: m_name(std::move(name)), m_kind(kind)
	{
	}

	Type::~Type() noexcept = default;

	const std::string &Type::getName() const
	{
		return m_name;
	}

	TypeKind Type::getKind() const
	{
		return m_kind;
	}

	Span<prp::PRPInstruction> Type::verifyInstructionSet(const Span<prp::PRPInstruction> &instructions) const
	{
		throw NotImplemented("You must implement this method in your own class!");
	}

	Type::DataMappingResult Type::map(const Span<prp::PRPInstruction> &instructions) const
	{
		throw NotImplemented("You must implement this method in your own class!");
	}
}