#pragma once

#include <cstdint>


namespace gamelib
{
	template <typename T>
	struct Span
	{
		const T *data { nullptr };
		int64_t size { 0 };

		Span() = default;
		Span(std::nullptr_t) : data(nullptr), size(0) {}

		Span(const T *d, int64_t s) : data(d), size(s)
		{
		}
		explicit Span(const std::vector<T> &vector)
		{
			if (!vector.empty())
			{
				data = vector.data();
				size = static_cast<int64_t>(vector.size());
			}
		}

		[[nodiscard]] explicit operator bool() const noexcept
		{
			return (data != nullptr);
		}

		Span<T>& operator++()
		{
			if (data && size)
			{
				++data;
				--size;
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
			return data[idx];
		}

		void reset()
		{
			data = nullptr;
			size = 0;
		}

		Span<T> slice(int64_t offset, int64_t sliceSize) const
		{
			if (offset >= size || offset + sliceSize > size) {
				return Span<T> { nullptr, 0 };
			}

			return Span<T> { data + offset, sliceSize };
		}
	};
}