#include <GameLib/TypeComplex.h>
#include <stdexcept>
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

	bool TypeComplex::isInheritedOf(const std::string &parentTypeName) const
	{
		const Type *parentType = getParent();
		return parentType && parentType->getName() == parentTypeName;
	}

	bool TypeComplex::hasGeomInfo() const
	{
		return m_geomInfo.has_value();
	}

	const GeomBasedTypeInfo &TypeComplex::getGeomInfo() const
	{
		static const GeomBasedTypeInfo kInvalidGeomInfo {};

		if (!m_geomInfo.has_value())
		{
			return kInvalidGeomInfo;
		}

		return m_geomInfo.value();
	}

	Type::VerificationResult TypeComplex::verify(const Span<prp::PRPInstruction>& instructions) const
	{
		if (!instructions)
		{
			assert(false);
			return std::make_pair(false, nullptr);
		}

		Span<PRPInstruction> ourSlice = instructions;

		if (auto parent = getParent(); parent != nullptr) {
			const auto& [result, nextSlice] = parent->verify(instructions);

			if (!result)
			{
				// Verify failed
				assert(false && "Verification failed");
				return std::make_pair(false, nullptr);
			}

			ourSlice = nextSlice;
		}

		/// And here we working with 'outSlice' variable!
		for (const auto &view: m_instructionViews)
		{
			if (view.isTrivialType()) // If property presented as trivial type - process it as single op code
			{
				auto trivialType = view.getTrivialType();
				const auto currentType = ourSlice[0].getOpCode();

				if (!OPCODE_VALID(trivialType))
				{
					assert(false && "Invalid opcode");
					return std::make_pair(false, nullptr);
				}

				if (ourSlice[0].getOpCode() != trivialType) {
					assert(false && "Unexpected type");
					return std::make_pair(false, nullptr);
				}

				ourSlice = ourSlice.slice(1, ourSlice.size() - 1);
				continue; // skip next part
			}

			// Type is not a trivial, need to find view's type pointer and validate it
			auto type = view.getType();
			if (!type)
			{
				assert(false && "Bad type reference");
				return std::make_pair(false, nullptr);
			}

			const auto& [result, nextSlice] = type->verify(ourSlice);
			if (!result)
			{
				return std::make_pair(false, nullptr);
			}

			ourSlice = nextSlice;
		}

		return std::make_pair(true, ourSlice);
	}

	GeomBasedTypeInfo &TypeComplex::createGeomInfo()
	{
		if (m_geomInfo.has_value()) {
			return m_geomInfo.value();
		}

		return m_geomInfo.emplace();
	}

	Type::DataMappingResult TypeComplex::map(const Span<PRPInstruction> &instructions) const
	{
		const auto& [result, _span] = verify(instructions);

		if (!result)
		{
			return {};
		}

		auto ourSlice = instructions;
		Value resultValue(this, {});

		// Map parent
		if (auto parent = getParent(); parent != nullptr)
		{
			const auto [value, newSlice] = parent->map(ourSlice);

			if (!value.has_value())
			{
				// Mapping failed
				return {};
			}

			// Import fields (but we need to change parenthesis referencing)
			for (const auto& [name, ip, views]: value->getEntries())
			{
				resultValue += std::make_pair(name, Value(value->getType(), Span(value->getInstructions()).slice(ip).as<std::vector<PRPInstruction>>(), views));
			}

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

				if (ourSlice[0].getOpCode() != trivialType) {
					assert(false);
					return {};
				}

				resultValue += std::make_pair(view.getName(), Value(this, { ourSlice[0] }, { view }));
				ourSlice = ourSlice.slice(1, ourSlice.size() - 1);
				continue; // skip next part
			}

			auto viewType = view.getType();
			if (!viewType)
			{
				assert(false);
				return {};
			}

			const auto [value, newSlice] = viewType->map(ourSlice);
			if (!value.has_value())
			{
				// Property mapping failed
				return {};
			}

			// Compress complex value into single view
			resultValue += std::make_pair(view.getName(), Value(viewType, value.value().getInstructions(), { ValueView(view.getName(), viewType, this) }));
			ourSlice = newSlice;
		}

		// Done
		return Type::DataMappingResult(resultValue, ourSlice);
	}

	Value TypeComplex::makeDefaultPropertiesPack() const
	{
		// NOTE: Complex type with unexposed properties will produce potentially wrong object. We will produce only exposed properties, nothing more.
		Value result {this, {}, {}};

		// 1: Get pack of parent properties before
		if (auto parent = getParent(); parent != nullptr)
		{
			// Really shitty code copypaste from ::map() method. What the hell was in my mind at that moment???
			auto parentPack = parent->makeDefaultPropertiesPack();
			for (const auto& [name, ip, views]: parentPack.getEntries())
			{
				result += std::make_pair(name, Value(parentPack.getType(), Span(parentPack.getInstructions()).slice(ip).as<std::vector<PRPInstruction>>(), views));
			}
		}

		// 2: Add our properties
		for (const auto &view: m_instructionViews)
		{
			if (view.isTrivialType())
			{
				// Trivial subject - just append
				result += std::pair(view.getName(), Value(this, { PRPInstruction(view.getTrivialType(), PRPOperandVal::kInitedOperandValue) }, { view }));
			}
			else
			{
				// Ok, need to ask inner type properties pack
				if (auto innerType = view.getType())
				{
					// Just add inner type properties pack
					auto pack = innerType->makeDefaultPropertiesPack();
					result += std::pair(view.getName(), Value(this, pack.getInstructions(), { view }));
				}
				else throw std::runtime_error("TypeComplex::makeDefaultPropertiesPack: unable to make default properties pack for " + view.getName());
			}
		}

		return result;
	}
}