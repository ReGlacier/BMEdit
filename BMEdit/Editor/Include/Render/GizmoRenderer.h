#pragma once

#include <QOpenGLShaderProgram>
#include <QFile>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <map>
#include <string>
#include <GameLib/BoundingBox.h>
#include <Render/GL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace render
{
        class GizmoRenderer
        {
        public:
                bool setup(GLFunctions *gapi, int screenWidth, int screenHeight);
                bool setFont(GLFunctions *gapi, QFile &fontFile, int pixelSize);
                void setScreenSize(int w, int h);
                void clear();
                void addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec4& color);
                void addAABB(const gamelib::BoundingBox& box, const glm::vec4& fillColor, const glm::vec4& lineColor);
                void addText(const std::string& text, const glm::vec2& screenPos, float size);
                void addMesh(const std::string& path, const glm::vec4& color);
                void render(GLFunctions *gapi,
                            QOpenGLShaderProgram *shader,
                            GLint cameraProjViewLoc,
                            const glm::mat4 &projView,
                            QOpenGLShaderProgram *textShader);

        private:
                struct Vertex
                {
                        glm::vec3 pos;
                        glm::vec4 color;
                };

                GLuint m_lineVao {0}, m_lineVbo {0};
                GLuint m_triVao {0},  m_triVbo {0};
                std::vector<Vertex> m_lines;
                std::vector<Vertex> m_tris;

                struct Glyph
                {
                        GLuint texture {0};
                        glm::ivec2 size {0};
                        glm::ivec2 bearing {0};
                        GLuint advance {0};
                };

                struct TextVertex
                {
                        glm::vec2 pos;
                        glm::vec2 uv;
                        glm::vec4 color;
                };

                struct TextGlyph
                {
                        GLuint texture;
                        TextVertex verts[6];
                };

                std::vector<TextGlyph> m_text;
                std::map<char, Glyph> m_glyphs;
                FT_Library m_ft { nullptr };
                FT_Face m_face { nullptr };
                GLuint m_textVao {0}, m_textVbo {0};
                int m_screenW {1}, m_screenH {1};
        };
}
