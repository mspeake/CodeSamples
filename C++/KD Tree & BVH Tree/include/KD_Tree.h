#ifndef KD_TREE_H
#define KD_TREE_H

#include "MathLib.h"
#include <vector>

struct Event
{
	float Value;
	int Type[3];
};

enum EventType
{
	STARTING,
	ENDING,
	COPLANAR
};

enum Axes
{
	X_AXIS,
	Y_AXIS,
	Z_AXIS
};

class SplitData
{
public:
	SplitData(unsigned a, float v, float c) : Axis(a), Value(v), Cost(c) {};

	unsigned Axis;
	float Value, Cost;
};

class KD_Node
{
private:
	void GetExtentsX(std::vector<Event>& xlist) const;
	void GetExtentsY(std::vector<Event>& ylist) const;
	void GetExtentsZ(std::vector<Event>& zlist) const;
	SplitData* GetSplitData(std::vector<Event>& xlist, std::vector<Event>& ylist, std::vector<Event>& zlist) const;
	void GetSplitDataX(std::vector<Event>& xlist, SplitData*& splitdata) const;
	void GetSplitDataY(std::vector<Event>& xlist, std::vector<Event>& ylist, SplitData*& splitdata) const;
	void GetSplitDataZ(std::vector<Event>& ylist, std::vector<Event>& zlist, SplitData*& splitdata) const;
	void SplitBox(SplitData*& splitdata);
	void DivideTriangles(int& pointsleft, int& pointsright, int& pointson) const;
	void DivideTrianglePoint(const Point& p, int& pointsleft, int& pointsright, int& pointson) const;

public:
	KD_Node();
	KD_Node(const AABB& b);
	~KD_Node();

	void SplitMe(void);

	AABB* MyBox;
	KD_Node* Left;
	KD_Node* Right;
	std::vector<Triangle> TriangleList;
	unsigned Depth;
};

class KD_Tree
{
private:
	static void DestructorHelper(KD_Node* n);

public:
	KD_Tree() { Root = NULL; };
	~KD_Tree();

	static KD_Tree* GenerateKDTree(void);

	KD_Node* Root;
};

extern std::vector<Object *> ObjectList;

#endif