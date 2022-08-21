#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#include <array>


namespace gamelib
{
	template <typename T>
	concept IsUnpackableConcept = requires(T t)
	{
		t.offset();
		t.size();
	};

	template <typename T>
	class Span
	{
		const T *m_data { nullptr };
		int64_t m_size { 0 };

	public:
		Span() = default;
		Span(std::nullptr_t) : m_data(nullptr), m_size(0) {}
		Span(const T *d, int64_t s) : m_data(d), m_size(s) {}
		template <size_t N> explicit Span(const std::array<T, N> &d) : m_data(d.data()), m_size(N) {}

		explicit Span(const std::vector<T> &vector)
		{
			if (!vector.empty())
			{
				m_data = vector.data();
				m_size = static_cast<int64_t>(vector.size());
			}
		}

		[[nodiscard]] const T* cbegin() const { return m_data; }
		[[nodiscard]] const T* cend() const { return m_data ? m_data + m_size : nullptr; }

		[[nodiscard]] T* begin() { return const_cast<T*>(m_data); }
		[[nodiscard]] T* end() { return m_data ? const_cast<T*>(m_data + m_size) : nullptr; }

		[[nodiscard]] T& front() { assert(!empty()); return const_cast<T&>(m_data[0]); }
		[[nodiscard]] T& back() { assert(!empty()); return const_cast<T&>(m_data[m_size - 1]); }

		[[nodiscard]] bool empty() const { return m_size == 0; }
		[[nodiscard]] int64_t size() const { return m_size; }
		[[nodiscard]] T* data() { return const_cast<T*>(m_data); }

		[[nodiscard]] explicit operator bool() const noexcept { return (m_data != nullptr); }

		Span<T>& operator++()
		{
			if (m_data && m_size)
			{
				++m_data;
				--m_size;
			}
			return *this;
		}

		Span<T> operator++(int)
		{
			Span<T> tmp { *this };
			++*this;
			return tmp;
		}

		[[nodiscard]] const T& operator[](int idx) const
		{
			return m_data[idx];
		}

		void reset()
		{
			m_data = nullptr;
			m_size = 0;
		}

		Span<T> slice(int64_t offset, int64_t sliceSize) const
		{
			if (offset >= m_size || offset + sliceSize > m_size) {
				return Span<T> { nullptr, 0 };
			}

			return Span<T> { m_data + offset, sliceSize };
		}

		template <typename TV>
		Span<T> slice(const TV& v) const requires(IsUnpackableConcept<TV>) { return slice(v.offset(), v.size()); }

		template <typename TR>
		TR as() const requires(std::is_constructible_v<TR, decltype(m_data), decltype(m_data + m_size)> && std::is_default_constructible_v<TR>) { return empty() ? TR {} : TR { m_data, m_data + m_size }; }
	};
}