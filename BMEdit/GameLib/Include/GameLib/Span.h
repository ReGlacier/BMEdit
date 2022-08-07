#pragma once

#include <cstdint>


namespace gamelib
{
	template <typename T>
	concept IsUnpackableConcept = requires(T t)
	{
		t.offset();
		t.size();
	};

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

		[[nodiscard]] const T* cbegin() const { return data; }
		[[nodiscard]] const T* cend() const { return data ? data + size : nullptr; }

		[[nodiscard]] T* begin() { return const_cast<T*>(data); }
		[[nodiscard]] T* end() { return data ? const_cast<T*>(data + size) : nullptr; }

		[[nodiscard]] bool empty() const { return size == 0; }

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

		template <typename TV>
		Span<T> slice(const TV& v) const requires(IsUnpackableConcept<TV>)
		{
			return slice(v.offset(), v.size());
		}

		template <typename TR>
		TR as() const requires(std::is_constructible_v<TR, decltype(data), decltype(data + size)> && std::is_default_constructible_v<TR>)
		{
			return empty() ? TR {} : TR { data, data + size };
		}
	};
}