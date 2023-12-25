#include <Render/GlacierVertex.h>


namespace render
{
	const VertexFormatDescription GlacierVertex::g_FormatDescription =
	    VertexFormatDescription()
			.addField(0, VertexDescriptionEntryType::VDE_Vec3, false)
			.addField(1, VertexDescriptionEntryType::VDE_Vec2, false);

	const VertexFormatDescription SimpleVertex::g_FormatDescription =
	    VertexFormatDescription()
	        .addField(0, VertexDescriptionEntryType::VDE_Vec3, false);
}