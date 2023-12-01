#ifndef OCTREE_HPP
#define OCTREE_HPP

#include <memory>

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
    Octree(int depth) {
        root = std::make_shared<OctreeNode>();
        root->empty = true;
        root->leaf = false;
        root->value = 0;
        for (int i = 0; i < 8; i++) {
            root->children[i] = nullptr;
        }
    }

    void insert(OctreeNodePtr where, int x, int y, int z, int value) {
        
    }

    void insert(int x, int y, int z, int value) {
        insert(root, x, y, z, value);
    }

};


#endif // OCTREE_HPP