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

    void insert(OctreeNodePtr node, int depth, int x, int y, int z, int value) {
        if(depth < 0) {
            std::cerr << "Depth is less than 0" << std::endl;
            return;
        }
        if (depth == 0) {
            node->value = value;
            node->leaf = true;
            node->empty = false;
            return;
        }
        int xnorm = (x & (1 << depth) ) >> depth;
        int ynorm = (y & (1 << depth) ) >> depth;
        int znorm = (z & (1 << depth) ) >> depth;

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

        insert(node->children[coord], depth - 1, x, y, z, value);
    }

    void insert(int x, int y, int z, int value) {
        insert(root, treeDepth-1, x, y, z, value);
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

};


#endif // OCTREE_HPP