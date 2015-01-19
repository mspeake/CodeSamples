#include "BVH_Tree.h"

BVH_Node::BVH_Node(void)
{
	Left = NULL;
	Right = NULL;
	Parent = NULL;
	MyBox = NULL;
	Obj = NULL;
	Type = LEAF;
}

BVH_Node::BVH_Node(const BVH_Node& n)
{
	if (n.Left != NULL)
		Left = new BVH_Node(*(n.Left));
	else
		Left = NULL;

	if (n.Right != NULL)
		Right = new BVH_Node(*(n.Right));
	else
		Right = NULL;

	Parent = n.Parent;
	if (n.MyBox != NULL)
		MyBox = new AABB(n.MyBox->C, n.MyBox->E);
	else
		MyBox = NULL;
	if (n.Obj != NULL)
		Obj = n.Obj;
	else
		Obj = NULL;
	Type = n.Type;
}

BVH_Node::~BVH_Node(void)
{
	if (Left != NULL)
		delete Left;
	Left = NULL;

	if (Right != NULL)
		delete Right;
	Right = NULL;

	Parent = NULL;
	if (MyBox != NULL)
		delete MyBox;
	MyBox = NULL;
	
	if (Obj != NULL)
		delete Obj;
	Obj = NULL;
}

void BVH_Node::RotateLeft(BVH_Node* n)
{
	BVH_Node* OldNode = n;
	n = n->Left;
	int DepthLeft = BVH_Tree::TreeDepth(n->Left), DepthRight = BVH_Tree::TreeDepth(n->Right);

	if (DepthLeft <= DepthRight) //move the child with less depth over
	{
		OldNode->Left = n->Left;
		n->Left->Parent = OldNode;
		n->Left = OldNode;
		OldNode->Parent = n;
	}
	else
	{
		OldNode->Left = n->Right;
		n->Right->Parent = OldNode;
		n->Right = OldNode;
		OldNode->Parent = n;
	}
}

void BVH_Node::RotateRight(BVH_Node* n)
{
	BVH_Node* OldNode = n;
	n = n->Right;
	int DepthLeft = BVH_Tree::TreeDepth(n->Left), DepthRight = BVH_Tree::TreeDepth(n->Right);

	if (DepthLeft <= DepthRight)
	{
		OldNode->Right = n->Left;
		n->Left->Parent = OldNode;
		OldNode->Parent = n;
		n->Left = OldNode;
	}
	else
	{
		OldNode->Right = n->Right;
		n->Right->Parent = OldNode;
		OldNode->Parent = n;
		n->Right = OldNode;
	}
}

BVH_Tree* BVH_Tree::GenerateBVHTree(int gm)
{
	BVH_Tree* BVHT = new BVH_Tree(gm);
	BVHT->Root = new BVH_Node();

	BVHT->CreateBoxes();

	if (BVHT->GenerationMethod == BOTTOM_UP)
	{
		int NodeCount = BVHT->NodesList.size();
		while (NodeCount > 1)
		{
			std::vector<BVH_Node*>::iterator it1, it2, Node1, Node2;
			float MinCost;
			for (it1 = BVHT->NodesList.begin(); it1 != BVHT->NodesList.end(); it1++)
			{
				it2 = it1;
				++it2;
				if (it1 == BVHT->NodesList.begin())
				{
					Node1 = it1;
					Node2 = it2;
					MinCost = BVH_Cost(*(*Node1)->MyBox, *(*Node2)->MyBox);
				}
				for (; it2 != BVHT->NodesList.end(); it2++)
				{
					if ((*it2)->Parent != NULL)
						continue;
					float current_cost = BVH_Cost(*(*it1)->MyBox, *(*it2)->MyBox);
					if (current_cost < MinCost)
					{
						Node1 = it1;
						Node2 = it2;
						MinCost = current_cost;
					}
				}
			}
			BVH_Node *CombinedNode = new BVH_Node();
			CombinedNode->Left = new BVH_Node(*(*Node1));
			CombinedNode->Left->Parent = CombinedNode;

			CombinedNode->Right = new BVH_Node(*(*Node2));
			CombinedNode->Right->Parent = CombinedNode;

			CombinedNode->MyBox = new AABB(*(*Node1)->MyBox, *(*Node2)->MyBox);
			CombinedNode->Type = NODE;

			BVHT->NodesList.erase(Node2);
			BVHT->NodesList.erase(Node1);
			BVHT->NodesList.push_back(CombinedNode);
			NodeCount = BVHT->NodesList.size();
		}

		BVHT->Root = BVHT->NodesList.back();
	}
	else if (BVHT->GenerationMethod == INSERTION)
	{
		for (unsigned i = 0; i < BVHT->NodesList.size(); i++)
			BVHT->InsertNode(BVHT->NodesList[i]);
	}

	//balance the tree after generation
	BVHT->BalanceTree();
}

void BVH_Tree::InsertNode(BVH_Node* n)
{
	if (Root == NULL) //no root, this node is now the root
	{
		Root = n;
		return;
	}

	BVH_Node* LeafNode = Root;
	while (LeafNode->Type != LEAF) //go down the tree until a leaf node is found
	{
		float CostLeft = BVH_Cost(*(n->MyBox), *(LeafNode->Left->MyBox)); //find which side is cheaper to combine the node with
		float CostRight = BVH_Cost(*(n->MyBox), *(LeafNode->Right->MyBox));
		if (CostLeft <= CostRight) //if left is cheaper, go down to the left, else go down to the right
			LeafNode = LeafNode->Left;
		else
			LeafNode = LeafNode->Right;
	}

	BVH_Node* CombinedNode = new BVH_Node(); //make a new node, it's children will be the leaf node and the insertion node
	CombinedNode->Type = NODE;
	CombinedNode->Parent = LeafNode->Parent;

	if (LeafNode == Root) //if the leaf node is the root, have to make the new node be the root
		Root = CombinedNode;
	else if (LeafNode == LeafNode->Parent->Left) //find out which child the leaf node is so
		LeafNode->Parent->Left = CombinedNode;     //that the correct pointer can be updated
	else
		LeafNode->Parent->Right = CombinedNode;

	CombinedNode->Left = n;         //combine the given node and the chosen node into a new node,
	CombinedNode->Right = LeafNode;     //set the given and chosen nodes to its children
	n->Parent = CombinedNode;
	LeafNode->Parent = CombinedNode;

	CombinedNode->MyBox = new AABB(*(n->MyBox), *(LeafNode->MyBox));

	BVH_Node *Boxes = CombinedNode->Parent;
	while (Boxes != NULL) //go back up the tree and adjust the Boxes
	{
		delete Boxes->MyBox;
		Boxes->MyBox = new AABB(*(Boxes->Left->MyBox), *(Boxes->Right->MyBox));
		Boxes = Boxes->Parent;
	}
	//balance the tree after insertion
	BalanceTree();
}

void BVH_Tree::RemoveNode(BVH_Node* n)
{
	if (n->Type != LEAF) //only remove leaf nodes
		return;

	if (Root == n)
	{
		delete n;
		Root = NULL;
		return;
	}

	BVH_Node *NodeParent = n->Parent; //keep track of the node's parent and grandparent, if it has one
	BVH_Node *NodeGrandparent = NodeParent->Parent;
	BVH_Node *NodeSibling = NULL;

	if (n == NodeParent->Left) //find the node's NodeSiblingling
		NodeSibling = NodeParent->Right;
	else
		NodeSibling = NodeParent->Left;

	if (NodeGrandparent != NULL) //if the node has a grandparent, adjust it's pointers
	{
		if (NodeParent == NodeGrandparent->Left)
			NodeGrandparent->Left = NodeSibling;
		else
			NodeGrandparent->Right = NodeSibling;

		NodeSibling->Parent = NodeGrandparent;

		delete n;
		delete NodeParent;

		while (NodeGrandparent != NULL) //go back up the tree and update the boxes
		{
			delete NodeGrandparent->MyBox;
			NodeGrandparent->MyBox = new AABB(*(NodeGrandparent->Left->MyBox), *(NodeGrandparent->Right->MyBox));
			NodeGrandparent = NodeGrandparent->Parent;
		}
	}
	else //no grandparent to deal with
	{
		if (n == NodeParent->Left) //find which side to adjust the boxes of
		{
			NodeParent->Left = NULL;
			delete NodeParent->MyBox;
			NodeParent->MyBox = new AABB(*(NodeParent->Right->MyBox), *(NodeParent->Right->MyBox));
		}
		else
		{
			NodeParent->Right = NULL;
			delete NodeParent->MyBox;
			NodeParent->MyBox = new AABB(*(NodeParent->Left->MyBox), *(NodeParent->Left->MyBox));
		}

		delete n;
	}

	//balance the tree after removal
	BalanceTree();
}

int BVH_Tree::TreeDepth(const BVH_Node* n)
{
	if (n->Type == LEAF) //no children, depth = 0
		return 0;
	else if ((n->Left->Type != LEAF) && (n->Right->Type != LEAF)) //both children have children, take larger depth
		return int(MyMax(float(TreeDepth(n->Left)), float(TreeDepth(n->Right))));
	else if (n->Left->Type != LEAF) //only left child has children
		return 1 + TreeDepth(n->Left);
	else if (n->Right->Type != LEAF) //only right child has children
		return 1 + TreeDepth(n->Right);
	else //neither child has children(both are leaf nodes)
		return 1;
}

void BVH_Tree::BalanceNode(BVH_Node* n)
{
	if (n->Type == LEAF) //leaf nodes and nodes with 2 leaf children are already balanced
		return;
	else if ((n->Left->Type == LEAF) && (n->Right->Type == LEAF))
		return;

	int DepthLeft = TreeDepth(n->Left),
		DepthRight = TreeDepth(n->Right);

	if (abs(DepthLeft - DepthRight) < 2) //tree is already balanced
		return;

	BVH_Node* OldNode = n; //keep track of the node we want to balance, as well as it's parent
	BVH_Node* OldParent = n->Parent;

	if (DepthLeft > DepthRight)
		BVH_Node::RotateLeft(n);
	else
		BVH_Node::RotateRight(n);

	//update the box of OldNode, then n
	// (n's new box is dependent on the box of OldNode, so OldNode's box must be updated first)
	delete OldNode->MyBox;
	OldNode->MyBox = new AABB(*(OldNode->Left->MyBox), *(OldNode->Right->MyBox));
	delete n->MyBox;
	n->MyBox = new AABB(*(n->Left->MyBox), *(n->Right->MyBox));

	//if we were balancing the root, readjust the root pointer and parent pointer
	if (OldParent == NULL)
	{
		n->Parent = NULL;
		Root = n;
	}
	else
	{
		if (OldParent->Left == OldNode) //find which child pointer needs to be updated
		{
			OldParent->Left = n;
			n->Parent = OldParent;   //and update the node's parent pointer
		}
		else
		{
			OldParent->Right = n;
			n->Parent = OldParent;
		}
		//update the old parent's box
		delete OldParent->MyBox;
		OldParent->MyBox = new AABB(*(OldParent->Left->MyBox), *(OldParent->Right->MyBox));
	}
}

void BVH_Tree::BalanceHelper(BVH_Node* n)
{
	if (n->Type == LEAF)
		return;

	int DepthLeft = TreeDepth(n->Left), DepthRight = TreeDepth(n->Right);
	int Diff = abs(DepthLeft - DepthRight);
	while (Diff >= 2)
	{
		BalanceNode(n);
		DepthLeft = TreeDepth(n->Left), DepthRight = TreeDepth(n->Right);
		Diff = abs(DepthLeft - DepthRight);
	}
	BalanceHelper(n->Left);
	BalanceHelper(n->Right);
}

void BVH_Tree::BalanceTree(void)
{
	BalanceHelper(Root); //balance the tree at the node
}

void BVH_Tree::CreateBoxes(void) const
{
	for (unsigned i = 0; i < ObjectList.size(); i++) //create Boxes for every object
	{
		float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
		for (unsigned j = 0; j < ObjectList[i]->MyShape->VertexCount; j++)
		{
			Point P(ObjectList[i]->MyShape->VertexArray[j * 3 + 0], ObjectList[i]->MyShape->VertexArray[j * 3 + 1], ObjectList[i]->MyShape->VertexArray[j * 3 + 2]);
			if (j == 0)
			{
				MinX = P.X, MaxX = P.X,
					MinY = P.Y, MaxY = P.Y,
					MinZ = P.Z, MaxZ = P.Z;
			}

			if (P.X < MinX)
				MinX = P.X;
			if (P.X > MaxX)
				MaxX = P.X;
			if (P.Y < MinY)
				MinY = P.Y;
			if (P.Y > MaxY)
				MaxY = P.Y;
			if (P.Z < MinZ)
				MinZ = P.Z;
			if (P.Z > MaxZ)
				MaxZ = P.Z;
		}
		Point MaxP(MaxX, MaxY, MaxZ);
		Point C(MaxX - ((MaxX - MinX) / 2.0f), MaxY - ((MaxY - MinY) / 2.0f), MaxZ - ((MaxZ - MinZ) / 2.0f));
		Vector E = MaxP - C;
		BVH_Node *node = new BVH_Node();
		node->MyBox = new AABB(C, E);
		NodesList.push_back(node);
	}
}

float BVH_Tree::BVH_Cost(const AABB &b1, const AABB &b2)
{
	AABB CombinedBox(b1, b2);
	float Width = CombinedBox.E.X * 2.0f,
		Height = CombinedBox.E.Y * 2.0f,
		Depth = CombinedBox.E.Z * 2.0f;
	return SurfaceArea(Width, Height, Depth);
}
