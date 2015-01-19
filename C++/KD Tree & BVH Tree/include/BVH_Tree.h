#ifndef BVH_TREE_H
#define BVH_TREE_H

#include "MathLib.h"
#include <vector>

enum NodeType
{
	NODE,
	LEAF
};

enum BVH_Generation_Method
{
	BOTTOM_UP,
	INSERTION
};

class BVH_Node
{
public:
	BVH_Node();
	BVH_Node(const BVH_Node& n);
	~BVH_Node();

	void static RotateLeft(BVH_Node* n);
	void static RotateRight(BVH_Node* n);

	BVH_Node *Left;
	BVH_Node *Right;
	BVH_Node *Parent;
	int Type;
	AABB *MyBox;
	Object *Obj;
};

class BVH_Tree
{
private:
	void CreateBoxes(void) const;
	static float BVH_Cost(const AABB &b1, const AABB &b2);
	void InsertNode(BVH_Node* n);
	void RemoveNode(BVH_Node* n);
	static int TreeDepth(const BVH_Node* n);
	void BalanceNode(BVH_Node* n);
	void BalanceHelper(BVH_Node* n);
	void BalanceTree(void);

public:
	BVH_Tree(int gm = BOTTOM_UP) : GenerationMethod(gm), Root(NULL) {};
	~BVH_Tree();

	static BVH_Tree* GenerateBVHTree(int gm = BOTTOM_UP);

	BVH_Node* Root;
	std::vector<BVH_Node*> NodesList;
	int GenerationMethod;

	friend class BVH_Node;
};

extern std::vector<Object *> ObjectList;

#endif