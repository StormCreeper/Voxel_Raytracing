#ifndef OCTREE_HPP
#define OCTREE_HPP

#include <memory>
#include <iostream>

#include "gl_includes.hpp"

const int value_flag = 0xF0000000;
const int value_mask = 0x00FFFFFF;

const int address_flag = 0x0F000000;
const int address_mask = 0x00FFFFFF;

struct OctreeNode;

typedef std::shared_ptr<OctreeNode> OctreeNodePtr;

struct OctreeNode {
    int value;
    OctreeNodePtr children[8];
    bool empty;
    bool leaf;
};

class Octree {
private:
    OctreeNodePtr root;
public:
    GLuint textureID;
    GLuint treeDepth;
    
    Octree(int depth) {
        root = std::make_shared<OctreeNode>();
        root->empty = true;
        root->leaf = false;
        root->value = 0;
        for (int i = 0; i < 8; i++) {
            root->children[i] = nullptr;
        }
        treeDepth = depth;
    }

    void insert(OctreeNodePtr node, int d, int x, int y, int z, int value) {
        if (d == treeDepth) {
            node->value = value;
            node->leaf = true;
            node->empty = false;
            return;
        }
        int c = treeDepth - d - 1;
        int xnorm = (x & (1 << c)) >> c;
        int ynorm = (y & (1 << c)) >> c;
        int znorm = (z & (1 << c)) >> c;

        int coord = xnorm + (ynorm << 1) + (znorm << 2);

        if (node->children[coord] == nullptr) {
            node->children[coord] = std::make_shared<OctreeNode>();
            node->children[coord]->empty = true;
            node->children[coord]->leaf = false;
            node->children[coord]->value = 0;
            for (int i = 0; i < 8; i++) {
                node->children[coord]->children[i] = nullptr;
            }
            node->empty = false;
        }

        insert(node->children[coord], d + 1, x, y, z, value);
    }

    void insert(int x, int y, int z, int value) {
        insert(root, 0, x, y, z, value);
    }

    void print(OctreeNodePtr node, int depth) {
        if (node == nullptr) {
            return;
        }
        for (int i = 0; i < depth; i++) {
            std::cout << " ";
        }
        std::cout << node->value << std::endl;
        for (int i = 0; i < 8; i++) {
            print(node->children[i], depth + 1);
        }
    }

    void print() {
        print(root, 0);
    }

    int writeData(OctreeNodePtr node, int depth, int* data, int* index) {
        if (node == nullptr) {
            return -1;
        }
        if (node->leaf) {
            return -1;
        }
        int numCellsX = 1 << (treeDepth - 1);
        int numCellsY = 1 << (treeDepth - 1);
        int numCellsZ = 1 << (treeDepth - 1);

        int cellX = *index % numCellsX;
        int cellY = (*index / numCellsX) % numCellsY;
        int cellZ = (*index / (numCellsX * numCellsY)) % numCellsZ;

        int currentIndex = *index;

        *index = *index + 1;
        for (int i = 0; i < 8; i++) {
            int encodedValue = 0;
            if(node->children[i] != nullptr) {
                if(node->children[i]->leaf) {
                    int value = node->children[i]->value;
                    encodedValue = value & value_mask | value_flag;
                } else {
                    int childIndex = writeData(node->children[i], depth + 1, data, index);
                    encodedValue = childIndex & address_mask | address_flag;
                }
            }
            int subCellX = cellX * 2 + (i & 1);
            int subCellY = cellY * 2 + ((i & 2) >> 1);
            int subCellZ = cellZ * 2 + ((i & 4) >> 2);

            int subCellIndex = subCellX + subCellY * numCellsX * 2 + subCellZ * numCellsX * numCellsY * 4;
            data[subCellIndex] = encodedValue;
        }

        return currentIndex;
    }

    void generateTexture() {
        int* texture = new int[1 << (3 * treeDepth)];
        /*for (int i = 0; i < (1 << (3 * treeDepth)); i++) {
            texture[i] = 0;
        }*/
        int index = 0;
        writeData(root, 0, texture, &index);

        // Opengl texture generation

        glActiveTexture(GL_TEXTURE0);

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_3D, textureID);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, 1 << treeDepth, 1 << treeDepth, 1 << treeDepth, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, texture);
        glBindTexture(GL_TEXTURE_3D, 0);

        delete[] texture;
    }

    ~Octree() {
        if(textureID) {
            glDeleteTextures(1, &textureID);
        }
    }
};


#endif // OCTREE_HPP