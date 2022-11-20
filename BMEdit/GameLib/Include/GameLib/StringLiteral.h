#pragma once

namespace gamelib
{
	/**
	 * @credits https://github.com/OrfeasZ/ZHMModSDK/blob/8517abd144f12544310061d46ea1d63d3806cee4/ZHMModSDK/Include/Glacier/CompileReflection.h#L119
	 */
	template <std::size_t N>
	struct StringLiteral
	{
		constexpr StringLiteral(const char(&p_Str)[N])
		{
			std::copy_n(p_Str, N, Value);
		}

		char Value[N];
	};
}