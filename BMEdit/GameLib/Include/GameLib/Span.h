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
		Span(const T *d, int64_t s) : data(d), size(s)
		{
		}

		[[nodiscard]] explicit operator bool() const noexcept
		{
			return (data != nullptr) && (size > 0);
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
	};
}