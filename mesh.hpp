#ifndef MESH_H
#define MESH_H

#include "gl_includes.hpp"

#include <memory>
#include <vector>

class Mesh {
public:
    void initGPUGeometry(const std::vector<float> &vertexPositions, const std::vector<unsigned int> &triangleIndices);
    void setGPUGeometry(GLuint posVbo, GLuint ibo, GLuint vao, size_t numIndices);
    void render() const;
    static std::shared_ptr<Mesh> genPlane(); // Should generate a unit plane
    

    ~Mesh();
    
    GLuint m_vao = 0;
    size_t m_numIndices = 0;
    
private:
    GLuint m_posVbo = 0;
    GLuint m_ibo = 0;
};


#endif // MESH_H