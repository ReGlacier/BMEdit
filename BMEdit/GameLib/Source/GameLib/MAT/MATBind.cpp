#include <GameLib/MAT/MATEntries.h>
#include <GameLib/MAT/MATBind.h>
#include <GameLib/ZBioHelpers.h>
#include <ZBinaryReader.hpp>


namespace gamelib::mat
{
	MATBind::MATBind() = default;

	MATBind MATBind::makeFromStream(ZBio::ZBinaryReader::BinaryReader* binaryReader, int propertiesCount)
	{
		MATBind b {};

		for (int i = 0; i < propertiesCount; i++)
		{
			MATPropertyEntry entry;
			MATPropertyEntry::deserialize(entry, binaryReader);

			if (entry.kind == MATPropertyKind::PK_NAME)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				b.name = binaryReader->readCString();
			}
			else if (entry.kind == MATPropertyKind::PK_RENDER_STATE)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				b.renderStates.emplace_back(MATRenderState::makeFromStream(binaryReader, entry.containerCapacity));
			}
			else if (entry.kind == MATPropertyKind::PK_TEXTURE)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				b.textures.emplace_back(MATTexture::makeFromStream(binaryReader, entry.containerCapacity));
			}
			else if (entry.kind == MATPropertyKind::PK_COLOR)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				b.colorChannels.emplace_back(MATColorChannel::makeFromStream(binaryReader, entry.containerCapacity));
			}
			else if (entry.kind == MATPropertyKind::PK_SPRITE)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				b.sprites.emplace_back(MATSprite::makeFromStream(binaryReader, entry.containerCapacity));
			}
			else if (entry.kind == MATPropertyKind::PK_BOOLEAN)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				b.options.emplace_back(MATOption::makeFromStream(binaryReader, entry.containerCapacity));
			}
			else if (entry.kind == MATPropertyKind::PK_SCROLL)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				b.scrolls.emplace_back(MATScroll::makeFromStream(binaryReader, entry.containerCapacity));
			}
			else if (entry.kind == MATPropertyKind::PK_FLOAT_VALUE)
			{
				ZBioSeekGuard guard { binaryReader };
				binaryReader->seek(entry.reference);

				b.floats.emplace_back(MATFloat::makeFromStream(binaryReader, entry.containerCapacity));
			}
			else
			{
				assert(false && "Unprocessed entry!");
			}
		}

		return b;
	}
}