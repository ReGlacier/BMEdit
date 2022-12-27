#pragma once

#include <GameLib/Vector3.h>
#include <cstdint>
#include <array>


namespace gamelib
{
	struct BoundingBox
	{
		Vector3 min;
		Vector3 max;

		BoundingBox() = default;
		BoundingBox(const Vector3 &vMin, const Vector3 &vMax);

		[[nodiscard]] Vector3 getCenter() const;

		template <typename TVertex, typename TIndex>
		void getCubeAsLines(std::array<TVertex, 8> &aVertices, std::array<TIndex, 24> &aIndices) requires (std::is_integral_v<TIndex> && std::is_constructible_v<TVertex, float, float, float>)
		{
			getVertices(aVertices);

			constexpr TIndex indices[] = {
			    0, 3,
			    3, 5,
			    5, 6,
			    6, 0,

			    0, 2,
			    2, 7,
			    7, 6,
			    2, 4,

			    4, 1,
			    1, 7,
			    1, 5,
			    4, 3
			};

			std::memcpy(&aIndices[0], &indices[0], sizeof(indices));
		}

		template <typename TVertex, typename TIndex>
		void getCubeAsTriangles(std::array<TVertex, 8> &aVertices, std::array<TIndex, 36> &aIndices) requires (std::is_integral_v<TIndex> && std::is_constructible_v<TVertex, float, float, float>)
		{
			getVertices(aVertices);

			constexpr TIndex indices[] = {
			    0, 3, 5,
			    0, 6, 5,
			    3, 4, 1,
			    1, 3, 5,

			    3, 4, 2,
			    3, 0, 2,
			    0, 2, 7,
			    0, 6, 7,

			    5, 6, 7,
			    5, 1, 7,
			    4, 1, 2,
			    2, 1, 7
			};

			std::memcpy(&aIndices[0], &indices[0], sizeof(indices));
		}

	private:
		template <typename TVertex>
		void getVertices(std::array<TVertex, 8> &aVertices) requires (std::is_constructible_v<TVertex, float, float, float>)
		{
			/*
			 * Cube schematic:
			 *
			 * (0) - min
			 * (1) - max
			 *
			 *     (4)-------(1)
			 *     /|        /|
			 *    / |       / |
			 *   /  |      /  |
			 * (3)--*----(5)  |
			 *  |  (2)----|--(7)
			 *  |  /      |  /
			 *  | /       | /
			 * (0)-------(6)
			 */
			aVertices[0] = TVertex { min.x, min.y, min.z };
			aVertices[1] = TVertex { max.x, max.y, max.z };
			aVertices[2] = TVertex { min.x, min.y, max.z };
			aVertices[3] = TVertex { min.x, max.y, min.z };
			aVertices[4] = TVertex { min.x, max.y, max.z };
			aVertices[5] = TVertex { max.x, max.y, min.z };
			aVertices[6] = TVertex { max.x, min.y, min.z };
			aVertices[7] = TVertex { max.x, min.y, max.z };

		}
	};
}