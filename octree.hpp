#ifndef OCTREE_HPP
#define OCTREE_HPP

#include <memory>
#include <iostream>

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
    int treeDepth;
public:
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

        index ++;
        for (int i = 0; i < 8; i++) {
            int encodedValue = 0;
            if(node->children[i] != nullptr) {
                if(node->children[i]->leaf) {
                    int value = node->children[i]->value;
                    encodedValue = value & (~(1 << 31));
                } else {
                    int childIndex = writeData(node->children[i], depth + 1, data, index);
                    encodedValue = childIndex | (1 << 31);
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
        // Clear texture
        for (int i = 0; i < (1 << (3 * treeDepth)); i++) {
            texture[i] = 0;
        }
        int index = 0;
        writeData(root, 0, texture, &index);

        std::cout << "Texture generated:" << std::endl;
        for (int x=0; x < (1 << treeDepth); x++) {
            for (int y=0; y < (1 << treeDepth); y++) {
                for (int z=0; z < (1 << treeDepth); z++) {
                    std::cout << std::hex << texture[x + y * (1 << treeDepth) + z * (1 << treeDepth) * (1 << treeDepth)] << std::dec << " ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }

        delete[] texture;

    }

};


#endif // OCTREE_HPP