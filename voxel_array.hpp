#ifndef VOXEL_ARRAY_HPP
#define VOXEL_ARRAY_HPP

#include "gl_includes.hpp"
#include <random>
#include <iostream>

class VoxelArray {
public:
    GLuint sizeX, sizeY, sizeZ;
    GLuint* data;

    GLuint textureID;

public:
    VoxelArray(GLuint sizeX, GLuint sizeY, GLuint sizeZ) {
        this->sizeX = sizeX;
        this->sizeY = sizeY;
        this->sizeZ = sizeZ;
        data = new GLuint[sizeX * sizeY * sizeZ];

        generateVoxelData();
        generateTexture();
    }

    /**
     * @brief Encode a color into a 9 bits unsigned integer.
     * 5 bits per component.
    */
    GLuint encodeColor(GLubyte x, GLubyte y, GLubyte z) {
        return (x & 0x1Fu) | ((y & 0x1Fu) << 5u) | ((z & 0x1Fu) << 10u);
    }

    /**
     * @brief Encode a normal into a 21 bits unsigned integer.
     * 5 bits per component.
    */
    GLuint encodeNormal(GLubyte x, GLubyte y, GLubyte z) {
        return (x & 0x1Fu) | ((y & 0x1Fu) << 5u) | ((z & 0x1Fu) << 10u);
    }

    /**
     * @brief Encode voxel data into a 3D texture.
     * 32 bits per voxel.
     * - 15 bits for the color (RGB)
     *    - 5 bits per component
     * - 15 bits for the normal (XYZ)
     *   - 5 bits per component
     * - 2 bits for the material
    */
    GLuint encodeVoxelData(GLuint color, GLuint normal, GLuint material) {
        return (color & 0x7FFFu) | ((normal & 0x7FFFu) << 15u) | ((material & 0x3u) << 30u);
    }

    void generateVoxelData() {
        for (int i = 0; i < sizeX * sizeY * sizeZ; i++) {
            GLuint x =  i % sizeX;
            GLuint y = (i / sizeX) % sizeY;
            GLuint z =  i / (sizeX * sizeY);

            glm::vec3 normalizedPos = glm::vec3(x, y, z) / glm::vec3(sizeX, sizeY, sizeZ) * 2.0f - 1.0f;

            data[i] = 0;
            if(glm::length(normalizedPos) < 1.0f) {
                GLubyte red = rand() % 32;
                GLubyte green = rand() % 32;
                GLubyte blue = rand() % 32;

                glm::vec3 normal = glm::normalize(normalizedPos);
                GLubyte nx = static_cast<GLubyte>((normal.x + 1.0f) * 0.5f * 32.0f);
                GLubyte ny = static_cast<GLubyte>((normal.y + 1.0f) * 0.5f * 32.0f);
                GLubyte nz = static_cast<GLubyte>((normal.z + 1.0f) * 0.5f * 32.0f);

                GLubyte material = rand() % 4;

                data[i] = encodeVoxelData(encodeColor(red, green, blue), encodeNormal(nx, ny, nz), material);
            }
        }
    }

    void generateTexture() {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_3D, textureID);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, sizeX, sizeY, sizeZ, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, data);
    }

    ~VoxelArray() {
        delete[] data;
        if (textureID != 0) {
            glDeleteTextures(1, &textureID);
        }
    }
};

#endif // !VOXEL_ARRAY_HPP