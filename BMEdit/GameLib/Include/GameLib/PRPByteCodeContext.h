#pragma once

#include <cstdint>


namespace gamelib::prp
{
	class PRPByteCodeContext
	{
	public:
		enum ContextFlags : int
		{
			CF_READ_ARRAY     = 1 << 0,
			CF_READ_CONTAINER = 1 << 1,
			CF_READ_OBJECT    = 1 << 2,
			CF_END_OF_STREAM  = 1 << 3
		};

		explicit PRPByteCodeContext(int opCodeIndex = 0);

		[[nodiscard]] int getIndex() const;
		[[nodiscard]] int getFlags() const;
		[[nodiscard]] bool isEndOfStream() const;
		[[nodiscard]] bool isArray() const;
		[[nodiscard]] bool isContainer() const;
		[[nodiscard]] bool isObject() const;
		[[nodiscard]] bool isSetFlag(ContextFlags flag) const;

		void setFlag(ContextFlags flag);
		void unsetFlag(ContextFlags flag);
		void setIndex(int newIndex);

		PRPByteCodeContext& operator+=(int off);
		PRPByteCodeContext& operator-=(int off);
	private:
		int m_flags            :  4 { 0 }; //See ContextFlags for details
		int m_instructionIndex : 28 { 0 }; //That should be enough
	};
}