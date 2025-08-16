#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <GameLib/BoundingBox.h>
#include <Render/GL.h>

namespace render
{
        class GizmoRenderer
        {
        public:
                bool setup(GLFunctions *gapi);
                void clear();
                void addLine(const glm::vec3& a, const glm::vec3& b, const glm::vec4& color);
                void addAABB(const gamelib::BoundingBox& box, const glm::vec4& color);
                void render(GLFunctions *gapi, QOpenGLShaderProgram *shader, GLint cameraProjViewLoc, const glm::mat4 &projView);

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
        };
}
