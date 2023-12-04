#ifndef VOXEL_ARRAY_HPP
#define VOXEL_ARRAY_HPP

#include "gl_includes.hpp"
#include "octree.hpp"

#include <random>
#include <iostream>
#include <memory>


class VoxelArray {
private:
public:
    GLuint size;
    int depth;
    glm::vec3* colorData;

    std::shared_ptr<Octree> octree;

public:
    VoxelArray(GLuint depth) {
        this->depth = depth;
        size = 1 << depth;
        colorData = new glm::vec3[size * size * size];

        generateVoxelData();
        generateOctree();

    }

    void generateVoxelData() {
        for (int i = 0; i < size * size * size; i++) {
            GLuint x =  i % size;
            GLuint y = (i / size) % size;
            GLuint z =  i / (size * size);

            glm::vec3 normalizedPos = glm::vec3(x, y, z) / glm::vec3(size, size, size) * 2.0f - 1.0f;

            colorData[i] = glm::vec3(0.0f);
            if(glm::length(normalizedPos) < 1.0f && glm::length(normalizedPos) > 0.95f) {
                float red   = sin((normalizedPos.x + normalizedPos.y + normalizedPos.z)*10.0f) * 0.4f + 0.6f;
                float green = sin((normalizedPos.x + normalizedPos.y + normalizedPos.z)*10.0f + 2.0f) * 0.4f + 0.6f;
                float blue  = sin((normalizedPos.x + normalizedPos.y + normalizedPos.z)*10.0f + 4.0f) * 0.4f + 0.6f;
                colorData[i] = glm::vec3(red, green, blue);
            }
        }
    }

    void generateOctree() {
        octree = std::make_shared<Octree>(depth);
        for(int i=0; i<size; i++) {
            for(int j=0; j<size; j++) {
                for(int k=0; k<size; k++) {
                    glm::vec3 color = colorData[i + j * size + k * size * size];
                    if(glm::length(color) > 0.0f) {
                        unsigned int r = color.x * 255.0f;
                        unsigned int g = color.y * 255.0f;
                        unsigned int b = color.z * 255.0f;
                        octree->insert(i, j, k, r << 16 | g << 8 | b);
                    }
                }
            }
        }
        octree->generateTexture();
    }

    ~VoxelArray() {
        delete[] colorData;
    }
};

#endif // !VOXEL_ARRAY_HPP