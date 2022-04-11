#include <GameLib/PRPByteCodeContext.h>


namespace gamelib::prp
{
	PRPByteCodeContext::PRPByteCodeContext(int opCodeIndex)
		: m_instructionIndex(opCodeIndex)
	{
	}

	int PRPByteCodeContext::getIndex() const
	{
		return m_instructionIndex;
	}

	int PRPByteCodeContext::getFlags() const
	{
		return m_flags;
	}

	bool PRPByteCodeContext::isEndOfStream() const
	{
		return m_flags & ContextFlags::CF_END_OF_STREAM;
	}

	bool PRPByteCodeContext::isArray() const
	{
		return m_flags & ContextFlags::CF_READ_ARRAY;
	}

	bool PRPByteCodeContext::isContainer() const
	{
		return m_flags & ContextFlags::CF_READ_CONTAINER;
	}

	bool PRPByteCodeContext::isObject() const
	{
		return m_flags & ContextFlags::CF_READ_OBJECT;
	}

	bool PRPByteCodeContext::isSetFlag(ContextFlags flag) const
	{
		return m_flags & flag;
	}

	void PRPByteCodeContext::setFlag(ContextFlags flag)
	{
		m_flags |= flag;
	}

	void PRPByteCodeContext::unsetFlag(ContextFlags flag)
	{
		m_flags &= ~flag;
	}

	void PRPByteCodeContext::setIndex(int newIndex)
	{
		m_instructionIndex = newIndex;
	}

	PRPByteCodeContext &PRPByteCodeContext::operator+=(int off)
	{
		m_instructionIndex += off;
		return *this;
	}

	PRPByteCodeContext &PRPByteCodeContext::operator-=(int off)
	{
		m_instructionIndex -= off;
		return *this;
	}
}