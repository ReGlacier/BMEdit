#pragma once

#include <cstdint>
#include <variant>
#include <string>
#include <vector>

#include <GameLib/PRP/PRPOpCode.h>
#include <GameLib/PRP/PRPTokenTable.h>


namespace gamelib::prp
{
	using RawData = std::vector<uint8_t>;
	using StringArray = std::vector<std::string>;

	struct PRPOperandVal
	{
		union
		{
			bool b;
			char c;
			int8_t i8;
			int16_t i16;
			int32_t i32;
			float f32;
			double f64;
		} trivial{};
		std::string str{};
		RawData raw{};
		StringArray stringArray{};

		PRPOperandVal() = default;
		explicit PRPOperandVal(bool b)
		{ trivial.b = b; }
		explicit PRPOperandVal(char c)
		{ trivial.c = c; }
		explicit PRPOperandVal(int8_t v)
		{ trivial.i8 = v; }
		explicit PRPOperandVal(int16_t v)
		{ trivial.i16 = v; }
		explicit PRPOperandVal(int32_t v)
		{ trivial.i32 = v; }
		explicit PRPOperandVal(float v)
		{ trivial.f32 = v; }
		explicit PRPOperandVal(double v)
		{ trivial.f64 = v; }
		explicit PRPOperandVal(std::string v) : str(std::move(v))
		{
		}
		explicit PRPOperandVal(RawData v) : raw(std::move(v))
		{
		}
		explicit PRPOperandVal(StringArray v) : stringArray(std::move(v))
		{
		}

		template <typename T> T get() const;

		template <> bool get() const { return trivial.b; }
		template <> char get() const { return trivial.c; }
		template <> int8_t get() const { return trivial.i8; }
		template <> int16_t get() const { return trivial.i16; }
		template <> int32_t get() const { return trivial.i32; }
		template <> float get() const { return trivial.f32; }
		template <> double get() const { return trivial.f64; }
		template <> std::string_view get() const { return str; }
		template <> const std::string& get() const { return str; }
		template <> const RawData& get() const { return raw; }
		template <> const StringArray& get() const { return stringArray; }
	};

	class PRPInstruction
	{
	public:
		PRPInstruction() = default;
		explicit PRPInstruction(PRPOpCode opCode);
		PRPInstruction(PRPOpCode opCode, PRPOperandVal operand);

		[[nodiscard]] bool isSet() const;
		[[nodiscard]] bool isNamed() const;
		[[nodiscard]] bool isDeclarator() const;
		[[nodiscard]] bool isBeginObject() const;
		[[nodiscard]] bool isBeginArray() const;
		[[nodiscard]] bool isEndObject() const;
		[[nodiscard]] bool isEndArray() const;
		[[nodiscard]] bool isEndOfStream() const;
		[[nodiscard]] bool isTrivialValue() const;
		[[nodiscard]] bool isBool() const;
		[[nodiscard]] bool isString() const;
		[[nodiscard]] bool isNumber() const;
		[[nodiscard]] bool isEnum() const;
		[[nodiscard]] bool isContainer() const;
		[[nodiscard]] bool hasValue() const;
		[[nodiscard]] PRPOpCode getOpCode() const;
		[[nodiscard]] const PRPOperandVal &getOperand() const;

		[[nodiscard]] bool operator==(const PRPInstruction &other) const;
		[[nodiscard]] bool operator!=(const PRPInstruction &other) const;

	private:
		void updateFlags();
		void updateIsNamedFlag();
		void updateIsDeclaratorFlag();

	private:
		PRPOpCode m_opCode{PRPOpCode::ERR_UNKNOWN};
		PRPOperandVal m_operand{};
		bool m_isSet{false}; // Means that current instruction has any value
		bool m_isNamed{false}; // Means that instruction from 'named' subset
		bool m_isDeclarator{false}; // Means that instruction has no data, only action to engine
	};
}