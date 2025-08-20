#include <Render/GizmoRenderer.h>
#include <QMatrix4x4>
#include <glm/gtc/type_ptr.hpp>

namespace render
{
    bool GizmoRenderer::setup(GLFunctions* gapi)
    {
            if (!gapi) return false;

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

            return true;
    }

    void GizmoRenderer::clear()
    {
            m_lines.clear();
            m_tris.clear();
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

    void GizmoRenderer::render(GLFunctions* gapi, QOpenGLShaderProgram *shader, GLint cameraProjViewLoc, const glm::mat4 &projView)
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
            clear();
    }
}
