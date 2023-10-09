#pragma once

#include <unordered_map>
#include <cstdint>
#include <variant>
#include <vector>
#include <string>
#include <memory>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::mat
{
	/**
	 * Describe type of operand
	 * @note See sub_471820 for details
	 */
	enum class MATValueType
	{
		PT_FLOAT  = 0,
		PT_CHAR   = 1,
		PT_UINT32 = 2,
		PT_LIST   = 3,
		PT_UNKNOWN
	};

	/**
	 * @brief All (almost) possible material system properties from Glacier 1. There are divided by few groups:
	 * 		  1) STRREF - reference to string
	 * 		  2) TRIVIAL - trivial value (int, float, bool)
	 * 		  3) MATLayer - layer
	 * 		  4) MATClass - material class
	 * 		  5) MATSubClass - material subclass
	 * 		  6) MATSprite - material sprite
	 * 		  7) MATBind - some binding
	 * 		  8) MATInstance - material instance
	 * 		  9) MATRenderState - render state
	 * 		  10) MATTexture - texture description
	 * 		  11) MATColorChannel - color channel description
	 * 		  12) MATOption - some boolean option with ref to shader param
	 * @note See sub_471D70 (ZRenderMaterialBinderParser::ParseMaterialProperties) for details
	 */
	enum class MATPropertyKind : uint32_t
	{
		PK_NULL_PROPERTY   = 0u,           ///< Nothing, used only as 'error' value. Not used by game!
		PK_NAME            = 0x4E414D45u,  ///< [STRREF] Reference to string represents name of something
		PK_TYPE            = 0x54595045u,  ///< [STRREF] Reference to string represents type of object
		PK_PATH            = 0x50415448u,  ///< [STRREF] Reference to string with part to something (asset, scene object, texture name, etc)
		PK_IDEN            = 0x4944454Eu,  ///< [STRREF] Reference to string represents some kind of id
		PK_OTYP            = 0x4F545950u,  ///< [STRREF] Reference to string, idk what represents
		PK_STYP            = 0x53545950u,  ///< [STRREF] Reference to string, idk what represents
		PK_LAYER           = 0x4C415945u,  ///< [MATLayer] Reference to list of properties represents 1 layer. Contains PK_PATH block with path to shader program (with extension)
		PK_CLASS           = 0x434C4153u,  ///< [MATClass] Reference to list of properties represents material class
		PK_SUB_CLASS       = 0x53554243u,  ///< [MATSubClass] Reference to list of properties represents material subclass
		PK_ENABLE          = 0x454E4142u,  ///< [TRIVIAL] Hold boolean value, in `reference` stored value (0 or 1)
		PK_SPRITE          = 0x53505249u,  ///< [MATSprite] Reference to list of properties represents sprite. Contains PK_NAME block (name of sprite?) and one or more enable blocks. @note Need investigate this place more carefully
		PK_BIND            = 0x42494E44u,  ///< [MATBind] Reference to list of properties
		PK_INSTANCE        = 0x494E5354u,  ///< [MATInstance] Reference to list of properties represents material instance on scene (scene object may refs to instance)
		PK_BLEND_ENABLE    = 0x42454E41u,  ///< [TRIVIAL] Hold boolean value, in `reference` stored value (0 or 1 - disable or enable blending)
		/**
		 * @brief [STRREF] Reference to string contains blend mode string repr (part of enum?).
		 * Possible values: TRANS, TRANS_ON_OPAQUE, TRANSADD_ON_OPAQUE, ADD_BEFORE_TRANS, ADD_ON_OPAQUE, ADD, SHADOW, STATICSHADOW
		 */
		PK_BLEND_MODE      = 0x424D4F44u,
		PK_OPACITY         = 0x4F504143u,  ///< [TRIVIAL] Hold float value, in `reference` stored float (fp32) value represents opacity of something
		PK_ALPHA_TEST      = 0x41545354u,  ///< [TRIVIAL] Hold boolean value, in `reference` stored bool (0 or 1)
		PK_ALPHA_REFERENCE = 0x41524546u,  ///< [TRIVIAL] Hold uint32 value, in `reference` stored uint32_t (u32) value represents some alpha reference scalar value
		PK_FOG_ENABLED     = 0x46454E41u,  ///< [TRIVIAL] Hold boolean value, in `reference` stored bool (0 or 1). Value represents fog target state
		PK_CULL_MODE       = 0x43554C4Cu,  ///< [STRREF] Reference to string represents culling mode. Possible values: DontCare, OneSided, TwoSided. @note I'm not sure that game uses this values. Be careful!
		PK_Z_BIAS          = 0x5A424941u,  ///< [TRIVIAL] Hold boolean value, in `reference`  stored bool (0 or 1)
		PK_Z_OFFSET        = 0x5A4F4646u,  ///< [TRIVIAL] Hold float value (fp32) in `reference`, represents Z offset
		PK_TEXTURE_ID      = 0x54584944u,  ///< [TRIVIAL] Hold uint32 value, in `reference` stored ID of texture (TEXEntry id, see TEX parser for details)
		PK_TILINIG_U       = 0x54494C55u,  ///< [STRREF] Reference to string represents U tiling mode. Possible values: NONE, TILED. See MATTilingMode enum for details
		PK_TILINIG_V       = 0x54494C56u,  ///< [STRREF] Reference to string represents V tiling mode. Possible values: NONE, TILED
		PK_TILINIG_W       = 0x54494C57u,  ///< [STRREF] Reference to string represents W tiling mode. Possible values are unknown. Probably unused, but declared in Glacier1 code.
		PK_VAL_I           = 0x56414C49u,  ///< [STRREF] Reference to string. I guess it's something about 'set boolean parameter by string literal'. `ReflectionEnabled` as example.
		PK_VAL_U           = 0x56414C55u,  ///< [TRIVIAL] Hold float or int value
		PK_VAL_F           = 0x554C4156u,  ///< Idk, unused
		PK_RENDER_STATE    = 0x52535441u,  ///< [MATRenderState] Reference to list of properties which represents render state.
		PK_TEXTURE         = 0x54455854u,  ///< [MATTexture] Reference to list of properties represents texture (PK_PATH - path to texture/PK_TEXTURE_ID - index of texture)
		PK_COLOR           = 0x434F4C4Fu,  ///< [MATColorChannel] Reference to list of properties represents usage of color channel (as example v4IlluminationColor, presented via 2 properties: PK_NAME (name of channel) and PK_ENABLED)
		PK_BOOLEAN         = 0x424F4F4Cu,  ///< [MATOption] Reference to list of properties represents boolean flag. Presented via 3 properties: PK_NAME: AlphaFadeEnabled, PK_ENABLED - use parameter or not and PK_VAL_U - value of flag
		PK_FLOAT_VALUE     = 0x464C5456u,  ///< [MATFloat] Reference to list of properties represents some float argument (or group of floats)
		PK_DMAP            = 0x50414D44u,  ///< Idk, unused (Glacier supports, but no usage)
		PK_FMIN            = 0x4E494D46u,  ///< Min filter (Idk, looks like legacy from OpenGL times)
		PK_FMAG            = 0x47414D46u,  ///< Mag filter (Idk, looks like legacy from OpenGL times)
		PK_FMIP            = 0x50494D46u,  ///< Idk, unused
		PK_SCROLL          = 0x5343524Cu,  ///< [MATScroll] Some scrollable something...
		PK_SCROLL_SPEED    = 0x53504544u,  ///< Idk, unused
		PK_ENUM            = 0x4D554E45u   ///< Idk, unused
	};

	struct MATHeader
	{
		uint32_t classListOffset { 0 };
		uint32_t instancesListOffset { 0 };
		uint32_t zeroed { 0 };
		uint32_t unknownTableOffset { 0 };

		static void deserialize(MATHeader& header, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct MATPropertyEntry
	{
		/**
		 * @brief Kind of this entry
		 * @note When PK_NULL_PROPERTY all other data will be invalid!
		 */
		MATPropertyKind kind { MATPropertyKind::PK_NULL_PROPERTY };

		/**
		 * @brief Reference to another block
		 */
		uint32_t reference { 0 };

		/**
		 * @brief Represents how much elements will be captured inside reference.
		 * 		  For int & float - always 1
		 * 		  For str - length of string
		 * 		  For list - elements count in list
		 */
		uint32_t containerCapacity { 0u };

		/**
		 * @brief Represents type of value inside reference (if reference value or not, idk)
		 */
		MATValueType valueType { MATValueType::PT_UNKNOWN };

		/**
		 * @brief Deserialize binary stream into object
		 * @param entry
		 * @param binaryReader
		 */
		static void deserialize(MATPropertyEntry& entry, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct MATClassDescription
	{
		std::string parentClass {};  // Name of parent class (a reference to PK_NAME property block)
		uint32_t unk4 { 0 };
		uint32_t unk8 { 0 };
		uint32_t unkC { 0 };
		uint32_t unk10 { 0 };
		uint32_t unk14 { 0 };
		uint32_t unk18 { 0 };
		uint32_t classDeclarationOffset {0}; // Class declaration offset
		uint32_t unk20 { 0 };
		uint32_t unk24 { 0 };
		uint32_t unk28 { 0 };
		uint32_t unk2C { 0 };

		static void deserialize(MATClassDescription& classDescription, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};

	struct MATInstanceDescription
	{
		std::string instanceParentClassName {};  // Name of instance class (a reference to PK_NAME property block)
		uint32_t unk4 { 0 }; // Usually 0x1
		uint32_t unk8 { 0 }; // 0x0
		uint32_t unkC { 0 };  // Usually 0x77
		uint32_t unk10 { 0 }; // 0x2
		uint32_t unk14 { 0 }; // 0x1
		uint32_t unk18 { 0 }; // 0x1
		uint32_t instanceDeclarationOffset {0}; // Material instance declaration offset
		uint32_t unk20 { 0 }; // 0x2
		uint32_t unk24 { 0 }; // 0x0
		uint32_t unk28 { 0 }; // 0x0
		uint32_t unk2C { 0 }; // 0x0

		static void deserialize(MATInstanceDescription& instanceDescription, ZBio::ZBinaryReader::BinaryReader* binaryReader);
	};
}