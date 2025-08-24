#include <Render/GizmoRenderer.h>
#include <QMatrix4x4>
#include <QFile>
#include <glm/gtc/type_ptr.hpp>

static constexpr float kTextOutlineThickness = 0.35f;

namespace render
{
    bool GizmoRenderer::setup(GLFunctions* gapi, int screenWidth, int screenHeight)
    {
            if (!gapi) return false;
            m_screenW = screenWidth;
            m_screenH = screenHeight;

            gapi->glGenVertexArrays(1, &m_lineVao);
            gapi->glGenBuffers(1, &m_lineVbo);
            gapi->glBindVertexArray(m_lineVao);
            gapi->glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
            gapi->glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
            gapi->glEnableVertexAttribArray(0);
            gapi->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
            gapi->glEnableVertexAttribArray(1);
            gapi->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));
            gapi->glBindVertexArray(0);

            gapi->glGenVertexArrays(1, &m_triVao);
            gapi->glGenBuffers(1, &m_triVbo);
            gapi->glBindVertexArray(m_triVao);
            gapi->glBindBuffer(GL_ARRAY_BUFFER, m_triVbo);
            gapi->glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
            gapi->glEnableVertexAttribArray(0);
            gapi->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
            gapi->glEnableVertexAttribArray(1);
            gapi->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec3)));
            gapi->glBindVertexArray(0);

            // --- Text rendering init ---
            if (FT_Init_FreeType(&m_ft))
                    return false;

            gapi->glGenVertexArrays(1, &m_textVao);
            gapi->glGenBuffers(1, &m_textVbo);
            gapi->glBindVertexArray(m_textVao);
            gapi->glBindBuffer(GL_ARRAY_BUFFER, m_textVbo);
            gapi->glBufferData(GL_ARRAY_BUFFER, sizeof(TextVertex) * 6, nullptr, GL_DYNAMIC_DRAW);

            gapi->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *) 0);
		    gapi->glEnableVertexAttribArray(0);

		    gapi->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *) offsetof(TextVertex, uv));
		    gapi->glEnableVertexAttribArray(1);

		    gapi->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *) offsetof(TextVertex, color));
		    gapi->glEnableVertexAttribArray(2);

            // Setup gizmo font
		    QFile fontFile{":/bmedit/gizmo_font_main.ttf"};
		    setFont(gapi, fontFile, 36);

            return true;
    }

    bool GizmoRenderer::setFont(GLFunctions *gapi, QFile &fontFile, int pixelSize)
	{
		if (!m_ft || !gapi)
			return false;
		if (!fontFile.open(QIODevice::ReadOnly))
			return false;
		QByteArray fontData = fontFile.readAll();
		fontFile.close();

		// Store fontData as member variable to prevent deallocation
		m_fontData = fontData;

		if (m_face) {
			FT_Done_Face(m_face);
			m_face = nullptr;
		}

		// Clean up existing atlas texture
		if (m_atlasTexture != 0) {
			gapi->glDeleteTextures(1, &m_atlasTexture);
			m_atlasTexture = 0;
		}
		m_glyphs.clear();

		// Use stored fontData instead of local variable
		if (FT_New_Memory_Face(m_ft,
		                       reinterpret_cast<const FT_Byte *>(m_fontData.data()),
		                       static_cast<FT_Long>(m_fontData.size()),
		                       0, &m_face))
			return false;

		if (FT_Set_Pixel_Sizes(m_face, 0, pixelSize))
			return false;

		// Set pixel mode to normal (not SDF) for regular rendering
		FT_Int32 load_flags = FT_LOAD_DEFAULT;
		FT_Render_Mode render_mode = FT_RENDER_MODE_NORMAL;

		// First pass: calculate required atlas dimensions
		struct GlyphInfo {
			FT_Bitmap bitmap;
			FT_Int bitmap_left;
			FT_Int bitmap_top;
			FT_Pos advance_x;
			std::vector<unsigned char> buffer;// Store processed bitmap data
		};

		std::map<unsigned char, GlyphInfo> tempGlyphs;
		int maxHeight = 0;
		int totalWidth = 0;
		int padding = 2;// Padding between glyphs

		// Load and process all glyphs first
		for (unsigned char c = 32; c < 128; ++c) {
			if (FT_Load_Char(m_face, c, load_flags))
				continue;
			if (FT_Render_Glyph(m_face->glyph, render_mode))
				continue;

			FT_Bitmap &bmp = m_face->glyph->bitmap;
			GlyphInfo info;
			info.bitmap = bmp;
			info.bitmap_left = m_face->glyph->bitmap_left;
			info.bitmap_top = m_face->glyph->bitmap_top;
			info.advance_x = m_face->glyph->advance.x;

			// Process bitmap data based on pixel mode
			if (bmp.width > 0 && bmp.rows > 0) {
				info.buffer.resize(bmp.width * bmp.rows);

				if (bmp.pixel_mode == FT_PIXEL_MODE_MONO) {
					// Convert 1-bit bitmap to 8-bit grayscale
					for (unsigned int y = 0; y < bmp.rows; ++y) {
						for (unsigned int x = 0; x < bmp.width; ++x) {
							unsigned char byte = bmp.buffer[y * bmp.pitch + x / 8];
							unsigned char bit = (byte >> (7 - (x % 8))) & 1;
							info.buffer[y * bmp.width + x] = bit ? 255 : 0;
						}
					}
				}
				else if (bmp.pixel_mode == FT_PIXEL_MODE_GRAY) {
					// Handle pitch properly for grayscale bitmaps
					for (unsigned int y = 0; y < bmp.rows; ++y) {
						memcpy(&info.buffer[y * bmp.width],
						       &bmp.buffer[y * bmp.pitch],
						       bmp.width);
					}
				}
				else {
					// Fallback for other formats - fill with white
					std::fill(info.buffer.begin(), info.buffer.end(), 255);
				}

				totalWidth += bmp.width + padding;
				maxHeight = std::max(maxHeight, static_cast<int>(bmp.rows));
			}

			tempGlyphs[c] = std::move(info);
		}

		// Calculate atlas dimensions (power of 2 for better GPU compatibility)
		int atlasWidth = 1;
		while (atlasWidth < totalWidth) atlasWidth <<= 1;

		int atlasHeight = 1;
		while (atlasHeight < maxHeight + padding * 2) atlasHeight <<= 1;

		// Create atlas buffer
		std::vector<unsigned char> atlasBuffer(atlasWidth * atlasHeight, 0);

		// Second pass: pack glyphs into atlas
		int currentX = padding;
		int currentY = padding;

		for (unsigned char c = 32; c < 128; ++c) {
			auto it = tempGlyphs.find(c);
			if (it == tempGlyphs.end()) {
				// Store empty glyph info for characters that couldn't be loaded
				m_glyphs[c] = {
				    glm::vec2(0.0f, 0.0f),            // texCoordMin
				    glm::vec2(0.0f, 0.0f),            // texCoordMax
				    glm::ivec2(0, 0),                 // size
				    glm::ivec2(0, 0),                 // bearing
				    static_cast<GLuint>(pixelSize / 2)// advance (rough estimate)
				};
				continue;
			}

			const GlyphInfo &info = it->second;

			// Check if glyph has actual bitmap data
			if (info.bitmap.width == 0 || info.bitmap.rows == 0) {
				// Store empty glyph info for spacing (like space character)
				m_glyphs[c] = {
				    glm::vec2(0.0f, 0.0f),                        // texCoordMin
				    glm::vec2(0.0f, 0.0f),                        // texCoordMax
				    glm::ivec2(0, 0),                             // size
				    glm::ivec2(info.bitmap_left, info.bitmap_top),// bearing
				    static_cast<GLuint>(info.advance_x)           // advance
				};
				continue;
			}

			// Check if we need to wrap to next row (simple left-to-right packing)
			if (currentX + info.bitmap.width + padding > atlasWidth) {
				currentX = padding;
				currentY += maxHeight + padding;

				// Check if we exceed atlas height
				if (currentY + info.bitmap.rows > atlasHeight) {
					// Atlas too small, this is a fallback - in production you'd want to resize
					break;
				}
			}

			// Copy glyph bitmap to atlas
			for (unsigned int y = 0; y < info.bitmap.rows; ++y) {
				for (unsigned int x = 0; x < info.bitmap.width; ++x) {
					int atlasIndex = (currentY + y) * atlasWidth + (currentX + x);
					int glyphIndex = y * info.bitmap.width + x;
					if (atlasIndex < atlasBuffer.size() && glyphIndex < info.buffer.size()) {
						atlasBuffer[atlasIndex] = info.buffer[glyphIndex];
					}
				}
			}

			// Calculate texture coordinates (normalized 0-1 range)
			float texMinX = static_cast<float>(currentX) / atlasWidth;
			float texMinY = static_cast<float>(currentY) / atlasHeight;
			float texMaxX = static_cast<float>(currentX + info.bitmap.width) / atlasWidth;
			float texMaxY = static_cast<float>(currentY + info.bitmap.rows) / atlasHeight;

			// Store glyph information
			m_glyphs[c] = {
			    glm::vec2(texMinX, texMinY),                    // texCoordMin
			    glm::vec2(texMaxX, texMaxY),                    // texCoordMax
			    glm::ivec2(info.bitmap.width, info.bitmap.rows),// size
			    glm::ivec2(info.bitmap_left, info.bitmap_top),  // bearing
			    static_cast<GLuint>(info.advance_x)             // advance
			};

			currentX += info.bitmap.width + padding;
		}

		// Create the atlas texture
		gapi->glGenTextures(1, &m_atlasTexture);
		gapi->glBindTexture(GL_TEXTURE_2D, m_atlasTexture);

		gapi->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight,
		                   0, GL_RED, GL_UNSIGNED_BYTE, atlasBuffer.data());

		gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		gapi->glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}

    void GizmoRenderer::setScreenSize(int w, int h)
    {
            m_screenW = w;
            m_screenH = h;
    }

    void GizmoRenderer::clear()
    {
            m_lines.clear();
            m_tris.clear();
            m_text.clear();
    }

    void GizmoRenderer::addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec4& color)
    {
            m_lines.push_back({a, color});
            m_lines.push_back({b, color});
    }

    void GizmoRenderer::addAABB(const gamelib::BoundingBox &box, const glm::vec4 &fillColor, const glm::vec4 &lineColor)
    {
            const glm::vec3& vMin = box.min;
            const glm::vec3& vMax = box.max;
            glm::vec3 p[8] = {
                {vMin.x, vMin.y, vMin.z}, {vMin.x, vMin.y, vMax.z},
                {vMin.x, vMax.y, vMin.z}, {vMin.x, vMax.y, vMax.z},
                {vMax.x, vMin.y, vMin.z}, {vMax.x, vMin.y, vMax.z},
                {vMax.x, vMax.y, vMin.z}, {vMax.x, vMax.y, vMax.z}
            };
            auto tri = [&](int a, int b, int c)
            {
			    m_tris.push_back({p[a], fillColor});
			    m_tris.push_back({p[b], fillColor});
			    m_tris.push_back({p[c], fillColor});
            };
            tri(0,1,2); tri(2,1,3);
            tri(4,6,5); tri(5,6,7);
            tri(0,4,1); tri(1,4,5);
            tri(2,3,6); tri(3,7,6);
            tri(0,2,4); tri(2,6,4);
            tri(1,5,3); tri(3,5,7);
            int edges[24] = {0,1,1,3,3,2,2,0,4,5,5,7,7,6,6,4,0,4,1,5,2,6,3,7};
            for (int i=0;i<24;i+=2)
                            addLine(p[edges[i]], p[edges[i + 1]], glm::vec4(lineColor.r, lineColor.g, lineColor.b, 1.0f));
    }

    static glm::vec4 ParseColor(const std::string &hex)
	{
		std::string str = hex;
		if (!str.empty() && str[0] == '#')
			str.erase(0, 1);

		if (str.length() != 6 && str.length() != 8)
			return {1.f, 1.f, 1.f, 1.f};

		uint32_t value = static_cast<uint32_t>(std::stoul(str, nullptr, 16));

		if (str.length() == 6) 
        {
			return 
            {
			    static_cast<float>((value >> 16) & 0xFF) / 255.f,
			    static_cast<float>((value >>  8) & 0xFF) / 255.f,
			    static_cast<float>((value >>  0) & 0xFF) / 255.f,
                1.f
            };
		}
		else 
        {
			return 
            {
			    static_cast<float>((value >> 24) & 0xFF) / 255.f,
			    static_cast<float>((value >> 16) & 0xFF) / 255.f,
			    static_cast<float>((value >>  8) & 0xFF) / 255.f,
			    static_cast<float>((value >>  0) & 0xFF) / 255.f
			};
		}
	}

    void GizmoRenderer::addText(const std::string &text, const glm::vec2 &screenPos, float size)
	{
		glm::vec4 currentColor(1.f, 1.f, 1.f, 1.f);
		std::string buffer;
		float x = screenPos.x;
		float y = screenPos.y;

		// Fixed version of the text positioning code
		auto Flush = [&]() {
			float scale = size / 48.f;
			for (char ch : buffer) {
				auto gIt = m_glyphs.find(ch);
				if (gIt == m_glyphs.end()) continue;
				const Glyph &g = gIt->second;

				// Skip empty glyphs (like space) but still advance
				if (g.size.x == 0 || g.size.y == 0) {
					x += (g.advance >> 6) * scale;
					continue;
				}

				// Calculate glyph position in screen coordinates
				// bearing.x: horizontal offset from pen position
				// bearing.y: vertical offset from baseline (positive = above baseline)
				float xpos = x + g.bearing.x * scale;
				float ypos = y - g.bearing.y * scale;// Subtract because screen Y increases downward
				float w = g.size.x * scale;
				float h = g.size.y * scale;

				// Convert screen coordinates to NDC coordinates
				// Screen: (0,0) = top-left, Y increases downward
				// NDC: (-1,-1) = bottom-left, Y increases upward
				float ndcX0 = (xpos / m_screenW) * 2.f - 1.f;
				float ndcY0 = 1.f - (ypos / m_screenH) * 2.f;// Top edge
				float ndcX1 = ((xpos + w) / m_screenW) * 2.f - 1.f;
				float ndcY1 = 1.f - ((ypos + h) / m_screenH) * 2.f;// Bottom edge

				// Create quad vertices in NDC space
				// The key is to match NDC vertex order with texture coordinate order
				TextVertex verts[6] = {
				    // Triangle 1
				    {{ndcX0, ndcY0}, {g.texCoordMin.x, g.texCoordMin.y}, currentColor},// top-left
				    {{ndcX1, ndcY0}, {g.texCoordMax.x, g.texCoordMin.y}, currentColor},// top-right
				    {{ndcX0, ndcY1}, {g.texCoordMin.x, g.texCoordMax.y}, currentColor},// bottom-left

				    // Triangle 2
				    {{ndcX0, ndcY1}, {g.texCoordMin.x, g.texCoordMax.y}, currentColor},// bottom-left
				    {{ndcX1, ndcY0}, {g.texCoordMax.x, g.texCoordMin.y}, currentColor},// top-right
				    {{ndcX1, ndcY1}, {g.texCoordMax.x, g.texCoordMax.y}, currentColor} // bottom-right
				};

				m_text.push_back({{verts[0], verts[1], verts[2], verts[3], verts[4], verts[5]}});
				x += (g.advance >> 6) * scale;
			}
			buffer.clear();
		};

		for (size_t i = 0; i < text.size();) {
			if (text[i] == '<') {
				if (text.compare(i, 3, "<p ") == 0) {
					// Flush current buffer before color change
					if (!buffer.empty()) {
						Flush();
					}
					size_t cpos = text.find("color=\"", i);
					if (cpos != std::string::npos) {
						cpos += 7;
						size_t end = text.find('"', cpos);
						std::string col = text.substr(cpos, end - cpos);
						currentColor = ParseColor(col);
						i = text.find('>', end);
						++i;
						continue;
					}
				}
				else if (text.compare(i, 4, "</p>") == 0) {
					Flush();
					currentColor = glm::vec4(1.f, 1.f, 1.f, 1.f);// Reset to white
					i += 4;
					continue;
				}
			}
			buffer += text[i];
			++i;
		}

		// Flush remaining text
		if (!buffer.empty()) {
			Flush();
		}
	}

	void GizmoRenderer::render(GLFunctions *gapi,
	                           QOpenGLShaderProgram *shader,
	                           GLint cameraProjViewLoc,
	                           const glm::mat4 &projView,
	                           QOpenGLShaderProgram *textShader)
	{
		if (!gapi || !shader) return;

		shader->bind();
		shader->setUniformValue(cameraProjViewLoc, QMatrix4x4(glm::value_ptr(projView)).transposed());

		if (!m_tris.empty()) {
			gapi->glBindVertexArray(m_triVao);
			gapi->glBindBuffer(GL_ARRAY_BUFFER, m_triVbo);
			gapi->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * m_tris.size()), m_tris.data(), GL_DYNAMIC_DRAW);
			gapi->glEnable(GL_BLEND);
			gapi->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			gapi->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLint>(m_tris.size()));
			gapi->glDisable(GL_BLEND);
		}

		if (!m_lines.empty()) {
			gapi->glBindVertexArray(m_lineVao);
			gapi->glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
			gapi->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * m_lines.size()), m_lines.data(), GL_DYNAMIC_DRAW);
			gapi->glDrawArrays(GL_LINES, 0, static_cast<GLint>(m_lines.size()));
		}

		gapi->glBindVertexArray(0);
		shader->release();

		// Text rendering
		if (textShader && !m_text.empty() && m_atlasTexture != 0) {
			textShader->bind();

			// Bind the single atlas texture once
			gapi->glActiveTexture(GL_TEXTURE0);
			gapi->glBindTexture(GL_TEXTURE_2D, m_atlasTexture);

			// Setup vars
			textShader->setUniformValue("outlineThickness", kTextOutlineThickness);
			textShader->setUniformValue("textAtlas", 0);

			gapi->glBindVertexArray(m_textVao);
			gapi->glEnable(GL_BLEND);
			gapi->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// Disable depth testing for text (optional, depending on your needs)
			gapi->glDisable(GL_DEPTH_TEST);

			for (const auto &textQuad : m_text) 
			{
				gapi->glBindBuffer(GL_ARRAY_BUFFER, m_textVbo);
				gapi->glBufferData(GL_ARRAY_BUFFER, sizeof(TextVertex) * 6, textQuad.verts, GL_DYNAMIC_DRAW);
				gapi->glDrawArrays(GL_TRIANGLES, 0, 6);
			}

			gapi->glEnable(GL_DEPTH_TEST);
			gapi->glDisable(GL_BLEND);
			gapi->glBindVertexArray(0);
			gapi->glBindTexture(GL_TEXTURE_2D, 0);
			textShader->release();
		}

		clear();
	}
}
