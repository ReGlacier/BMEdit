#include <Render/GizmoRenderer.h>
#include <QMatrix4x4>
#include <QFile>
#include <glm/gtc/type_ptr.hpp>

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
            gapi->glEnableVertexAttribArray(0);
            gapi->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), reinterpret_cast<void*>(0));
            gapi->glEnableVertexAttribArray(1);
            gapi->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), reinterpret_cast<void*>(sizeof(glm::vec2)));
            gapi->glEnableVertexAttribArray(2);
            gapi->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(TextVertex), reinterpret_cast<void*>(sizeof(glm::vec2)*2));
            gapi->glBindVertexArray(0);

            return true;
    }

    bool GizmoRenderer::setFont(GLFunctions* gapi, QFile &fontFile, int pixelSize)
    {
            if (!m_ft || !gapi)
                    return false;

            if (!fontFile.open(QIODevice::ReadOnly))
                    return false;
            QByteArray fontData = fontFile.readAll();
            fontFile.close();

            if (m_face)
            {
                    FT_Done_Face(m_face);
                    m_face = nullptr;
            }

            for (auto &g : m_glyphs)
            {
                    gapi->glDeleteTextures(1, &g.second.texture);
            }
            m_glyphs.clear();

            if (FT_New_Memory_Face(m_ft, reinterpret_cast<const FT_Byte*>(fontData.data()), static_cast<FT_Long>(fontData.size()), 0, &m_face))
                    return false;
            FT_Set_Pixel_Sizes(m_face, 0, pixelSize);

            for (unsigned char c = 32; c < 128; ++c)
            {
                    if (FT_Load_Char(m_face, c, FT_LOAD_DEFAULT))
                            continue;
                    if (FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_SDF))
                            continue;
                    FT_Bitmap &bmp = m_face->glyph->bitmap;
                    GLuint tex = 0;
                    gapi->glGenTextures(1, &tex);
                    gapi->glBindTexture(GL_TEXTURE_2D, tex);
                    gapi->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bmp.width, bmp.rows, 0, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);
                    gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    gapi->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    m_glyphs[c] = {tex,
                                   glm::ivec2(bmp.width, bmp.rows),
                                   glm::ivec2(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top),
                                   static_cast<GLuint>(m_face->glyph->advance.x)};
            }
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

    static glm::vec4 ParseColor(const std::string& hex)
    {
            if (hex.length() != 8) return {1.f,1.f,1.f,1.f};
            unsigned int value = std::stoul(hex, nullptr, 16);
            return {
                ((value >> 24) & 0xFF) / 255.f,
                ((value >> 16) & 0xFF) / 255.f,
                ((value >> 8) & 0xFF) / 255.f,
                (value & 0xFF) / 255.f
            };
    }

    void GizmoRenderer::addText(const std::string& text, const glm::vec2& screenPos, float size)
    {
            glm::vec4 currentColor(1.f,1.f,1.f,1.f);
            std::string buffer;
            float x = screenPos.x;
            float y = screenPos.y;
            auto flush = [&]()
            {
                    float scale = size / 48.f;
                    for (char ch : buffer)
                    {
                            auto gIt = m_glyphs.find(ch);
                            if (gIt == m_glyphs.end()) continue;
                            const Glyph& g = gIt->second;
                            float xpos = x + g.bearing.x * scale;
                            float ypos = y - (g.size.y - g.bearing.y) * scale;
                            float w = g.size.x * scale;
                            float h = g.size.y * scale;

                            float ndcX0 = (xpos / m_screenW) * 2.f - 1.f;
                            float ndcY0 = 1.f - (ypos / m_screenH) * 2.f;
                            float ndcX1 = ((xpos + w)/m_screenW)*2.f - 1.f;
                            float ndcY1 = 1.f - ((ypos + h)/m_screenH)*2.f;

                            TextVertex verts[6] = {
                                {{ndcX0, ndcY0}, {0.f,1.f}, currentColor},
                                {{ndcX1, ndcY0}, {1.f,1.f}, currentColor},
                                {{ndcX0, ndcY1}, {0.f,0.f}, currentColor},
                                {{ndcX0, ndcY1}, {0.f,0.f}, currentColor},
                                {{ndcX1, ndcY0}, {1.f,1.f}, currentColor},
                                {{ndcX1, ndcY1}, {1.f,0.f}, currentColor}
                            };
                            m_text.push_back({g.texture, {verts[0],verts[1],verts[2],verts[3],verts[4],verts[5]}});
                            x += (g.advance >> 6) * scale;
                    }
                    buffer.clear();
            };

            for (size_t i = 0; i < text.size(); )
            {
                    if (text[i] == '<')
                    {
                            if (text.compare(i,3,"<p ") == 0)
                            {
                                    size_t cpos = text.find("color=\"", i);
                                    if (cpos != std::string::npos)
                                    {
                                            cpos += 7;
                                            size_t end = text.find('"', cpos);
                                            std::string col = text.substr(cpos, end - cpos);
                                            currentColor = ParseColor(col);
                                            i = text.find('>', end);
                                            ++i;
                                            continue;
                                    }
                            }
                            else if (text.compare(i,4,"</p>") == 0)
                            {
                                    flush();
                                    currentColor = glm::vec4(1.f);
                                    i += 4;
                                    continue;
                            }
                    }
                    buffer += text[i];
                    ++i;
            }
            if (!buffer.empty())
                    flush();
    }

    void GizmoRenderer::addMesh(const std::string& path, const glm::vec4& color)
    {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
            if (!scene) return;
            for (unsigned int mi = 0; mi < scene->mNumMeshes; ++mi)
            {
                    const aiMesh* mesh = scene->mMeshes[mi];
                    for (unsigned int fi = 0; fi < mesh->mNumFaces; ++fi)
                    {
                            const aiFace& face = mesh->mFaces[fi];
                            if (face.mNumIndices != 3) continue;
                            aiVector3D v[3] = {
                                mesh->mVertices[face.mIndices[0]],
                                mesh->mVertices[face.mIndices[1]],
                                mesh->mVertices[face.mIndices[2]]
                            };
                            addLine(glm::vec3(v[0].x, v[0].y, v[0].z), glm::vec3(v[1].x, v[1].y, v[1].z), color);
                            addLine(glm::vec3(v[1].x, v[1].y, v[1].z), glm::vec3(v[2].x, v[2].y, v[2].z), color);
                            addLine(glm::vec3(v[2].x, v[2].y, v[2].z), glm::vec3(v[0].x, v[0].y, v[0].z), color);
                    }
            }
    }

    void GizmoRenderer::render(GLFunctions* gapi,
                               QOpenGLShaderProgram *shader,
                               GLint cameraProjViewLoc,
                               const glm::mat4 &projView,
                               QOpenGLShaderProgram *textShader)
    {
            if (!gapi || !shader) return;
            shader->bind();
            shader->setUniformValue(cameraProjViewLoc, QMatrix4x4(glm::value_ptr(projView)).transposed());
            if (!m_tris.empty())
            {
                    gapi->glBindVertexArray(m_triVao);
                    gapi->glBindBuffer(GL_ARRAY_BUFFER, m_triVbo);
                    gapi->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * m_tris.size()), m_tris.data(), GL_DYNAMIC_DRAW);
                    gapi->glEnable(GL_BLEND);
                    gapi->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    gapi->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLint>(m_tris.size()));
                    gapi->glDisable(GL_BLEND);
            }
            if (!m_lines.empty())
            {
                    gapi->glBindVertexArray(m_lineVao);
                    gapi->glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
                    gapi->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * m_lines.size()), m_lines.data(), GL_DYNAMIC_DRAW);
                    gapi->glDrawArrays(GL_LINES, 0, static_cast<GLint>(m_lines.size()));
            }
            gapi->glBindVertexArray(0);
            shader->release();

            if (textShader && !m_text.empty())
            {
                    textShader->bind();
                    textShader->setUniformValue("outlineThickness", 1.2f);
                    textShader->setUniformValue("textAtlas", 0);
                    gapi->glBindVertexArray(m_textVao);
                    gapi->glEnable(GL_BLEND);
                    gapi->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    for (const auto& glyph : m_text)
                    {
                            gapi->glActiveTexture(GL_TEXTURE0);
                            gapi->glBindTexture(GL_TEXTURE_2D, glyph.texture);
                            gapi->glBindBuffer(GL_ARRAY_BUFFER, m_textVbo);
                            gapi->glBufferData(GL_ARRAY_BUFFER, sizeof(TextVertex) * 6, glyph.verts, GL_DYNAMIC_DRAW);
                            gapi->glDrawArrays(GL_TRIANGLES, 0, 6);
                    }
                    gapi->glDisable(GL_BLEND);
                    gapi->glBindVertexArray(0);
                    textShader->release();
            }

            clear();
    }
}
