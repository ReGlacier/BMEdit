#include <GameLib/TypeComplex.h>
#include <cassert>


namespace gamelib
{
	using namespace prp;

	TypeComplex::TypeComplex(std::string typeName,
	                         std::vector<ValueView> &&instructionViews,
	                         Type *parent,
	                         bool allowUnexposedInstructions)
         : Type(TypeKind::COMPLEX, std::move(typeName))
         , m_instructionViews(std::move(instructionViews))
         , m_parent(parent)
         , m_allowUnexposedInstructions(allowUnexposedInstructions)
	{
	}

	TypeComplex::TypeComplex(std::string typeName,
	                         std::vector<ValueView> &&instructionViews,
	                         std::string parentType,
	                         bool allowUnexposedInstructions)
         : Type(TypeKind::COMPLEX, std::move(typeName))
         , m_instructionViews(std::move(instructionViews))
         , m_parent(std::move(parentType))
         , m_allowUnexposedInstructions(allowUnexposedInstructions)
	{
	}

	const std::vector<ValueView> &TypeComplex::getInstructionViews() const
	{
		return m_instructionViews;
	}

	const Type *TypeComplex::getParent() const
	{
		if (auto parentTypePtr = std::get_if<const Type *>(&m_parent); parentTypePtr != nullptr)
		{
			return *parentTypePtr;
		}

		return nullptr;
	}

	bool TypeComplex::areUnexposedInstructionsAllowed() const
	{
		return m_allowUnexposedInstructions;
	}

	Span<PRPInstruction> TypeComplex::verifyInstructionSet(const Span<PRPInstruction> &instructions) const
	{
		if (!instructions)
		{
			assert(false);
			return {};
		}

		Span<PRPInstruction> ourSlice = instructions;

		if (auto parent = getParent(); parent != nullptr) {
			const auto nextSlice = parent->verifyInstructionSet(instructions);
			if (!nextSlice)
			{
				// Verify failed
				return {};
			}

			ourSlice = nextSlice;
		}

		/// And here we working with 'outSlice' variable!
		for (const auto &view: m_instructionViews)
		{
			if (view.isTrivialType()) // If property presented as trivial type - process it as single op code
			{
				auto trivialType = view.getTrivialType();
				if (!OPCODE_VALID(trivialType))
				{
					assert(false);
					return {};
				}

				if (ourSlice.data[0].getOpCode() != trivialType) {
					assert(false);
					return {};
				}

				ourSlice = ourSlice.slice(1, ourSlice.size - 1);
				continue; // skip next part
			}

			auto type = view.getType();
			if (!type)
			{
				assert(false);
				return {};
			}

			ourSlice = type->verifyInstructionSet(ourSlice);
			if (!ourSlice)
			{
				return {};
			}
		}

		return ourSlice;
	}

	Type::DataMappingResult TypeComplex::map(const Span<PRPInstruction> &instructions) const
	{
		if (!verifyInstructionSet(instructions))
		{
			return Type::DataMappingResult();
		}

		auto ourSlice = instructions;
		Value resultValue(this, {});

		// Map parent
		if (auto parent = getParent(); parent != nullptr)
		{
			const auto [value, newSlice] = parent->map(ourSlice);

			if (!newSlice || !value.has_value())
			{
				// Mapping failed
				return Type::DataMappingResult();
			}

			resultValue += value.value();
			ourSlice = newSlice;
		}

		// Map properties
		for (const auto &view: m_instructionViews)
		{
			if (view.isTrivialType())
			{
				auto trivialType = view.getTrivialType();
				if (!OPCODE_VALID(trivialType))
				{
					assert(false);
					return {};
				}

				if (ourSlice.data[0].getOpCode() != trivialType) {
					assert(false);
					return {};
				}

				resultValue += Value(this, { ourSlice[0] }, { view });
				ourSlice = ourSlice.slice(1, ourSlice.size - 1);
				continue; // skip next part
			}

			auto viewType = view.getType();
			if (!viewType)
			{
				assert(false);
				return {};
			}

			const auto [value, newSlice] = viewType->map(ourSlice);
			if (!newSlice || !value.has_value())
			{
				// Property mapping failed
				return Type::DataMappingResult();
			}

			resultValue += value.value();
			ourSlice = newSlice;
		}

		// Done
		return Type::DataMappingResult(resultValue, ourSlice);
	}
}