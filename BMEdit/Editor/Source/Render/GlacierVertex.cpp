#include <Render/GlacierVertex.h>


namespace render
{
	const VertexFormatDescription SimpleVertex::g_FormatDescription =
	    VertexFormatDescription()
	        .addField(0, VertexDescriptionEntryType::VDE_Vec3, false);
}