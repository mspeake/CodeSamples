#include "KD_Tree.h"

KD_Node::KD_Node()
{
	Left = NULL;
	Right = NULL;
	MyBox = NULL;
	Depth = 1;
}

KD_Node::KD_Node(const AABB& b)
{
	MyBox = new AABB(b.C, b.E);
	Left = NULL;
	Right = NULL;
	Depth = 1;
}

KD_Node::~KD_Node()
{
	if (MyBox != NULL)
		delete MyBox;
	if (Left != NULL)
		delete Left;
	if (Right != NULL)
		delete Right;

	ObjectList.clear();
}

void KD_Tree::DestructorHelper(KD_Node* n)
{
	if (n == NULL)
		return;
	else
	{
		DestructorHelper(n->Left);
		DestructorHelper(n->Right);
		delete n;
	}
}

KD_Tree* KD_Tree::GenerateKDTree(void)
{
	float SmallestX, LargestX, SmallestY, LargestY, SmallestZ, LargestZ;
	KD_Tree* KDT = new KD_Tree();
	KDT->Root = new KD_Node();
	for(unsigned i = 0; i < ObjectList.size(); i++)
	{
		for(unsigned j = 0; j < ObjectList[i]->MyShape->FaceCount; j++)
		{
			unsigned short ind1 = ObjectList[i]->MyShape->FaceArray[j * 3 + 0];
			unsigned short ind2 = ObjectList[i]->MyShape->FaceArray[j * 3 + 1];
			unsigned short ind3 = ObjectList[i]->MyShape->FaceArray[j * 3 + 2];
			Point p1(ObjectList[i]->MyShape->VertexArray[ind1 * 3 + 0], ObjectList[i]->MyShape->VertexArray[ind1 * 3 + 1], ObjectList[i]->MyShape->VertexArray[ind1 * 3 + 2]);
			Point p2(ObjectList[i]->MyShape->VertexArray[ind2 * 3 + 0], ObjectList[i]->MyShape->VertexArray[ind2 * 3 + 1], ObjectList[i]->MyShape->VertexArray[ind2 * 3 + 2]);
			Point p3(ObjectList[i]->MyShape->VertexArray[ind3 * 3 + 0], ObjectList[i]->MyShape->VertexArray[ind3 * 3 + 1], ObjectList[i]->MyShape->VertexArray[ind3 * 3 + 2]);

			Triangle T(p1, p2, p3);
			KDT->Root->TriangleList.push_back(T);
			float MinX = Smallest(p1.X, p2.X, p3.X),
				  MaxX = Largest(p1.X, p2.X, p3.X),
				  MinY = Smallest(p1.Y, p2.Y, p3.Y),
				  MaxY = Largest(p1.Y, p2.Y, p3.Y),
				  MinZ = Smallest(p1.Z, p2.Z, p3.Z),
				  MaxZ = Largest(p1.Z, p2.Z, p3.Z);
			if(i == 0 && j == 0)
			{
				SmallestX = MinZ, LargestX = MaxX;
				SmallestY = MinY, LargestY = MaxY;
				SmallestZ = MinZ, LargestZ = MaxZ;
			}
			else
			{
				SmallestX = MyMin(SmallestX, MinX), LargestX = MyMax(LargestX, MaxX);
				SmallestY = MyMin(SmallestY, MinY), LargestY = MyMax(LargestY, MaxY);
				SmallestZ = MyMin(SmallestZ, MinZ), LargestZ = MyMax(LargestZ, MaxZ);
			}

		}
	}
	Point BoxCenter(LargestX - ((LargestX - SmallestX) / 2.0f), LargestY - ((LargestY - SmallestY) / 2.0f), LargestZ - ((LargestZ - SmallestZ) / 2.0f));
	Point MaxExtent(LargestX, LargestY, LargestZ);
	Vector BoxExtent = MaxExtent - BoxCenter;
	KDT->Root->MyBox = new AABB(BoxCenter, BoxExtent);
	KDT->Root->SplitMe();
	return KDT;
}

void KD_Node::SplitMe()
{
	if(TriangleList.size() <= 1)
		return;

	float MinX = MyBox->C.X - MyBox->E.X, MaxX = MyBox->C.X + MyBox->E.X;
	float MinY = MyBox->C.Y - MyBox->E.Y, MaxY = MyBox->C.Y + MyBox->E.Y;
	float MinZ = MyBox->C.Z - MyBox->E.Z, MaxZ = MyBox->C.Z + MyBox->E.Z;
	float RootW = MaxX - MinX, RootH = MaxY - MinY, RootD = MaxZ - MinZ;
	float RootSA = SurfaceArea(RootW, RootH, RootD);

	std::vector<Event> Xlist, Ylist, Zlist;
	//generate the event list for each axis
	GetExtentsX(Xlist);
	GetExtentsY(Ylist);
	GetExtentsZ(Zlist);

	SplitData* Split = GetSplitData(Xlist, Ylist, Zlist);
	//if the lowest cost is the same as not splitting, don't split
	if (Split->Cost >= TriangleList.size())
	{
		Left = NULL;
		Right = NULL;
		return;
	}

	//else, split the AABB at the calculated plane
	SplitBox(Split);

	//place the triangles in their corresponding boxes
	int PointsLeft = 0, PointsRight = 0, PointsOn = 0;
	DivideTriangles(PointsLeft, PointsRight, PointsOn);

	int LeftSize = Left->TriangleList.size(), RightSize = Right->TriangleList.size();
	if( (Left->TriangleList.empty()) || (Right->TriangleList.empty()) )
	{
		delete Left;
		Left = NULL;
		delete Right;
		Right = NULL;
		return;
	}

	if(Depth == MAX_TREE_DEPTH) //dont split anymore when the max tree depth is reached
		return;

	//now split the left and right boxes until an exit condition is met
	Left->SplitMe();
	Right->SplitMe();
	TriangleList.clear();
}

void KD_Node::GetExtentsX(std::vector<Event>& xlist) const
{
	for (unsigned i = 0; i < TriangleList.size(); i++) //find the min/max VertexArraylue of each triangle for the current axis
	{
		float MinV, MaxV;
		Event E1, E2;
		bool Event2 = false;
		E1.Type[STARTING] = 0, E1.Type[ENDING] = 0, E1.Type[COPLANAR] = 0;
		//
		MinV = Smallest(TriangleList[i].P.X, TriangleList[i].Q.X, TriangleList[i].R.X);
		MaxV = Largest(TriangleList[i].P.X, TriangleList[i].Q.X, TriangleList[i].R.X);
		if (MinV == MaxV)
		{
			E1.Type[COPLANAR] = 1;
			E1.Value = MinV;
		}
		else
		{
			E1.Type[STARTING] = 1;
			E1.Value = MinV;
			E2.Type[STARTING] = 0;
			E2.Type[ENDING] = 1;
			E2.Type[COPLANAR] = 0;
			E2.Value = MaxV;
			Event2 = true;
		}

		if (xlist.empty()) //if the event list is empty, add this first VertexArraylue in
		{
			xlist.push_back(E1);
			if (Event2 == true)
				xlist.push_back(E2);
		}
		else //if not, find where it belongs(smallest to largest order)
		{
			std::vector<Event>::iterator it = xlist.begin(), it2;
			while (it != xlist.end())
			{
				if (it->Value >= E1.Value) //find if the VertexArraylue is already present, or the first larger VertexArraylue
					break;
				it++;
			}
			if (it == xlist.end()) //if VertexArraylue is largest, add to end of list
			{
				xlist.push_back(E1);
				if (Event2 == true)
					xlist.push_back(E2);
			}
			else if (it->Value == E1.Value) //if VertexArraylue already present, increase the counters
			{
				it->Type[STARTING] += E1.Type[STARTING];
				it->Type[ENDING] += E1.Type[ENDING];
				it->Type[COPLANAR] += E1.Type[COPLANAR];
			}
			else //if VertexArraylue not present, insert it before the next largest present VertexArraylue
				xlist.insert(it, E1);

			if (Event2 == true)
			{
				it2 = xlist.begin();
				while (it2 != xlist.end())
				{
					if (it2->Value >= E2.Value) //find if the VertexArraylue is already present, or the first larger VertexArraylue
						break;
					it2++;
				}
				if (it2 == xlist.end()) //if VertexArraylue is largest, add to end of list
					xlist.push_back(E2);
				else if (it2->Value == E2.Value) //if VertexArraylue already present, increase the counters
				{
					it2->Type[STARTING] += E2.Type[STARTING];
					it2->Type[ENDING] += E2.Type[ENDING];
					it2->Type[COPLANAR] += E2.Type[COPLANAR];
				}
				else //if VertexArraylue not present, insert it before the next largest present VertexArraylue
					xlist.insert(it2, E2);
			}
		}
	}
}

void KD_Node::GetExtentsY(std::vector<Event>& ylist) const
{
	for (unsigned i = 0; i < TriangleList.size(); i++) //find the min/max VertexArraylue of each triangle for the current axis
	{
		float MinV, MaxV;
		Event E1, E2;
		bool Event2 = false;
		E1.Type[STARTING] = 0, E1.Type[ENDING] = 0, E1.Type[COPLANAR] = 0;
		//
		MinV = Smallest(TriangleList[i].P.Y, TriangleList[i].Q.Y, TriangleList[i].R.Y);
		MaxV = Largest(TriangleList[i].P.Y, TriangleList[i].Q.Y, TriangleList[i].R.Y);
		if (MinV == MaxV)
		{
			E1.Type[COPLANAR] = 1;
			E1.Value = MinV;
		}
		else
		{
			E1.Type[STARTING] = 1;
			E1.Value = MinV;
			E2.Type[STARTING] = 0;
			E2.Type[ENDING] = 1;
			E2.Type[COPLANAR] = 0;
			E2.Value = MaxV;
			Event2 = true;
		}

		if (ylist.empty()) //if the event list is empty, add this first VertexArraylue in
		{
			ylist.push_back(E1);
			if (Event2 == true)
				ylist.push_back(E2);
		}
		else //if not, find where it belongs(smallest to largest order)
		{
			std::vector<Event>::iterator it = ylist.begin(), it2;
			while (it != ylist.end())
			{
				if (it->Value >= E1.Value) //find if the VertexArraylue is already present, or the first larger VertexArraylue
					break;
				it++;
			}
			if (it == ylist.end()) //if VertexArraylue is largest, add to end of list
			{
				ylist.push_back(E1);
				if (Event2 == true)
					ylist.push_back(E2);
			}
			else if (it->Value == E1.Value) //if VertexArraylue already present, increase the counters
			{
				it->Type[STARTING] += E1.Type[STARTING];
				it->Type[ENDING] += E1.Type[ENDING];
				it->Type[COPLANAR] += E1.Type[COPLANAR];
			}
			else //if VertexArraylue not present, insert it before the next largest present VertexArraylue
				ylist.insert(it, E1);

			if (Event2 == true)
			{
				it2 = ylist.begin();
				while (it2 != ylist.end())
				{
					if (it2->Value >= E2.Value) //find if the VertexArraylue is already present, or the first larger VertexArraylue
						break;
					it2++;
				}
				if (it2 == ylist.end()) //if VertexArraylue is largest, add to end of list
					ylist.push_back(E2);
				else if (it2->Value == E2.Value) //if VertexArraylue already present, increase the counters
				{
					it2->Type[STARTING] += E2.Type[STARTING];
					it2->Type[ENDING] += E2.Type[ENDING];
					it2->Type[COPLANAR] += E2.Type[COPLANAR];
				}
				else //if VertexArraylue not present, insert it before the next largest present VertexArraylue
					ylist.insert(it2, E2);
			}
		}
	}
}

void KD_Node::GetExtentsZ(std::vector<Event>& zlist) const
{
	for (unsigned i = 0; i < TriangleList.size(); i++) //find the min/max VertexArraylue of each triangle for the current axis
	{
		float MinV, MaxV;
		Event E1, E2;
		bool Event2 = false;
		E1.Type[STARTING] = 0, E1.Type[ENDING] = 0, E1.Type[COPLANAR] = 0;
		//
		MinV = Smallest(TriangleList[i].P.Z, TriangleList[i].Q.Z, TriangleList[i].R.Z);
		MaxV = Largest(TriangleList[i].P.Z, TriangleList[i].Q.Z, TriangleList[i].R.Z);
		if (MinV == MaxV)
		{
			E1.Type[COPLANAR] = 1;
			E1.Value = MinV;
		}
		else
		{
			E1.Type[STARTING] = 1;
			E1.Value = MinV;
			E2.Type[STARTING] = 0;
			E2.Type[ENDING] = 1;
			E2.Type[COPLANAR] = 0;
			E2.Value = MaxV;
			Event2 = true;
		}

		if (zlist.empty()) //if the event list is empty, add this first VertexArraylue in
		{
			zlist.push_back(E1);
			if (Event2 == true)
				zlist.push_back(E2);
		}
		else //if not, find where it belongs(smallest to largest order)
		{
			std::vector<Event>::iterator it = zlist.begin(), it2;
			while (it != zlist.end())
			{
				if (it->Value >= E1.Value) //find if the VertexArraylue is already present, or the first larger VertexArraylue
					break;
				it++;
			}
			if (it == zlist.end()) //if VertexArraylue is largest, add to end of list
			{
				zlist.push_back(E1);
				if (Event2 == true)
					zlist.push_back(E2);
			}
			else if (it->Value == E1.Value) //if VertexArraylue already present, increase the counters
			{
				it->Type[STARTING] += E1.Type[STARTING];
				it->Type[ENDING] += E1.Type[ENDING];
				it->Type[COPLANAR] += E1.Type[COPLANAR];
			}
			else //if VertexArraylue not present, insert it before the next largest present VertexArraylue
				zlist.insert(it, E1);

			if (Event2 == true)
			{
				it2 = zlist.begin();
				while (it2 != zlist.end())
				{
					if (it2->Value >= E2.Value) //find if the VertexArraylue is already present, or the first larger VertexArraylue
						break;
					it2++;
				}
				if (it2 == zlist.end()) //if VertexArraylue is largest, add to end of list
					zlist.push_back(E2);
				else if (it2->Value == E2.Value) //if VertexArraylue already present, increase the counters
				{
					it2->Type[STARTING] += E2.Type[STARTING];
					it2->Type[ENDING] += E2.Type[ENDING];
					it2->Type[COPLANAR] += E2.Type[COPLANAR];
				}
				else //if VertexArraylue not present, insert it before the next largest present VertexArraylue
					zlist.insert(it2, E2);
			}
		}
	}
}

SplitData* KD_Node::GetSplitData(std::vector<Event>& xlist, std::vector<Event>& ylist, std::vector<Event>& zlist) const
{
	float MinX = MyBox->C.X - MyBox->E.X, MaxX = MyBox->C.X + MyBox->E.X;
	float MinY = MyBox->C.Y - MyBox->E.Y, MaxY = MyBox->C.Y + MyBox->E.Y;
	float MinZ = MyBox->C.Z - MyBox->E.Z, MaxZ = MyBox->C.Z + MyBox->E.Z;
	float RootW = MaxX - MinX, RootH = MaxY - MinY, RootD = MaxZ - MinZ;
	float RootSA = SurfaceArea(RootW, RootH, RootD);

	//event lists generated, use them to determine the lowest cost, the plane to split with and the VertexArraylue to split at
	int NumStarting = 0, NumEnding = 0, NumCoplanar = 0, TrianglesLeft = 0, TrianglesRight = TriangleList.size();
	SplitData* Split = new SplitData(0, xlist[0].Value, TriangleList.size());

	GetSplitDataX(xlist, Split);
	GetSplitDataY(xlist, ylist, Split);
	GetSplitDataZ(ylist, zlist, Split);
	
	return Split;
}

void KD_Node::GetSplitDataX(std::vector<Event>& xlist, SplitData*& splitdata) const
{
	float MinX = MyBox->C.X - MyBox->E.X, MaxX = MyBox->C.X + MyBox->E.X;
	float MinY = MyBox->C.Y - MyBox->E.Y, MaxY = MyBox->C.Y + MyBox->E.Y;
	float MinZ = MyBox->C.Z - MyBox->E.Z, MaxZ = MyBox->C.Z + MyBox->E.Z;
	float RootW = MaxX - MinX, RootH = MaxY - MinY, RootD = MaxZ - MinZ;
	float RootSA = SurfaceArea(RootW, RootH, RootD);
	int NumStarting = 0, NumEnding = 0, NumCoplanar = 0, TrianglesLeft = 0, TrianglesRight = TriangleList.size();

	for (unsigned i = 0; i < xlist.size(); i++)
	{
		int Ns1 = 0, Nc1 = 0; //deFaceArrayults
		int TrianglesCoplanar = 0;
		NumStarting = xlist[i].Type[STARTING];
		NumEnding = xlist[i].Type[ENDING];
		NumCoplanar = xlist[i].Type[COPLANAR];
		TrianglesLeft += (Ns1 + Nc1);
		TrianglesRight -= (NumEnding + NumCoplanar);
		float WidthLeft = xlist[i].Value - MinX, WidthRight = RootW - WidthLeft;
		float SAL = SurfaceArea(WidthLeft, RootH, RootD), SAR = SurfaceArea(WidthRight, RootH, RootD);
		float Cost = ((TrianglesLeft*SAL + TrianglesRight*SAR) / RootSA) + TrianglesCoplanar;
		if (Cost < splitdata->Cost)
		{
			splitdata->Cost = Cost;
			splitdata->Axis = X_AXIS;
			splitdata->Value = xlist[i].Value;
		}
	}
}

void KD_Node::GetSplitDataY(std::vector<Event>& xlist, std::vector<Event>& ylist, SplitData*& splitdata) const
{
	float MinX = MyBox->C.X - MyBox->E.X, MaxX = MyBox->C.X + MyBox->E.X;
	float MinY = MyBox->C.Y - MyBox->E.Y, MaxY = MyBox->C.Y + MyBox->E.Y;
	float MinZ = MyBox->C.Z - MyBox->E.Z, MaxZ = MyBox->C.Z + MyBox->E.Z;
	float RootW = MaxX - MinX, RootH = MaxY - MinY, RootD = MaxZ - MinZ;
	float RootSA = SurfaceArea(RootW, RootH, RootD);
	int NumStarting = 0, NumEnding = 0, NumCoplanar = 0, TrianglesLeft = 0, TrianglesRight = TriangleList.size();

	for (unsigned i = 0; i < ylist.size(); i++)
	{
		int Ns1 = xlist[i].Type[STARTING], Nc1 = xlist[i].Type[COPLANAR];//uses the previous axis list VertexArraylues
		int TrianglesCoplanar = xlist[i].Type[COPLANAR];
		NumStarting = ylist[i].Type[STARTING];
		NumEnding = ylist[i].Type[ENDING];
		NumCoplanar = ylist[i].Type[COPLANAR];
		TrianglesLeft += (Ns1 + Nc1);
		TrianglesRight -= (NumEnding + NumCoplanar);
		float HeightLeft = ylist[i].Value - MinY, HeightRight = RootH - HeightLeft;
		float SAL = SurfaceArea(RootW, HeightLeft, RootD), SAR = SurfaceArea(RootW, HeightRight, RootD);
		float Cost = ((TrianglesLeft*SAL + TrianglesRight*SAR) / RootSA) + TrianglesCoplanar;
		if (Cost < splitdata->Cost)
		{
			splitdata->Cost = Cost;
			splitdata->Axis = Y_AXIS;
			splitdata->Value = ylist[i].Value;
		}
	}
}

void KD_Node::GetSplitDataZ(std::vector<Event>& ylist, std::vector<Event>& zlist, SplitData*& splitdata) const
{
	float MinX = MyBox->C.X - MyBox->E.X, MaxX = MyBox->C.X + MyBox->E.X;
	float MinY = MyBox->C.Y - MyBox->E.Y, MaxY = MyBox->C.Y + MyBox->E.Y;
	float MinZ = MyBox->C.Z - MyBox->E.Z, MaxZ = MyBox->C.Z + MyBox->E.Z;
	float RootW = MaxX - MinX, RootH = MaxY - MinY, RootD = MaxZ - MinZ;
	float RootSA = SurfaceArea(RootW, RootH, RootD);
	int NumStarting = 0, NumEnding = 0, NumCoplanar = 0, TrianglesLeft = 0, TrianglesRight = TriangleList.size();

	for (unsigned i = 0; i < ylist.size(); i++)
	{
		int Ns1 = ylist[i].Type[STARTING], Nc1 = ylist[i].Type[COPLANAR];//uses the previous axis list VertexArraylues
		int TrianglesCoplanar = ylist[i].Type[COPLANAR];
		NumStarting = zlist[i].Type[STARTING];
		NumEnding = zlist[i].Type[ENDING];
		NumCoplanar = zlist[i].Type[COPLANAR];
		TrianglesLeft += (Ns1 + Nc1);
		TrianglesRight -= (NumEnding + NumCoplanar);
		float DepthLeft = zlist[i].Value - MinZ, DepthRight = RootD - DepthLeft;
		float SAL = SurfaceArea(RootW, RootH, DepthLeft), SAR = SurfaceArea(RootW, RootH, DepthRight);
		float Cost = ((TrianglesLeft*SAL + TrianglesRight*SAR) / RootSA) + TrianglesCoplanar;
		if (Cost < splitdata->Cost)
		{
			splitdata->Cost = Cost;
			splitdata->Axis = Z_AXIS;
			splitdata->Value = zlist[i].Value;
		}
	}
}

void KD_Node::SplitBox(SplitData*& splitdata)
{
	float MinX = MyBox->C.X - MyBox->E.X, MaxX = MyBox->C.X + MyBox->E.X;
	float MinY = MyBox->C.Y - MyBox->E.Y, MaxY = MyBox->C.Y + MyBox->E.Y;
	float MinZ = MyBox->C.Z - MyBox->E.Z, MaxZ = MyBox->C.Z + MyBox->E.Z;
	float RootW = MaxX - MinX, RootH = MaxY - MinY, RootD = MaxZ - MinZ;
	float RootSA = SurfaceArea(RootW, RootH, RootD);
	float LeftWidth, LeftHeight, LeftDepth, RightWidth, RightHeight, RightDepth;

	switch (splitdata->Axis)
	{
		case X_AXIS:
		{
			LeftHeight = RootH, RightHeight = RootH;
			LeftDepth = RootD, RightDepth = RootD;
			LeftWidth = splitdata->Value - MinX, RightWidth = RootW - LeftWidth;
			AABB* LeftBox = new AABB(Point(MinX + (LeftWidth / 2.0f), MyBox->C.Y, MyBox->C.Z),
				Vector(LeftWidth / 2.0f, LeftHeight / 2.0f, LeftDepth / 2.0f));
			AABB* RightBox = new AABB(Point(MaxX - (RightWidth / 2.0f), MyBox->C.Y, MyBox->C.Z),
				Vector(RightWidth / 2.0f, RightHeight / 2.0f, RightDepth / 2.0f));
			Left = new KD_Node(*LeftBox);
			Left->Depth = Depth + 1;
			Right = new KD_Node(*RightBox);
			Right->Depth = Depth + 1;
			break;
		}
		case Y_AXIS:
		{
			LeftWidth = RootW, RightWidth = RootW;
			LeftDepth = RootD, RightDepth = RootD;
			LeftHeight = splitdata->Value - MinY, RightHeight = RootH - LeftHeight;
			AABB* LeftBox = new AABB(Point(MyBox->C.X, MinY + (LeftHeight / 2.0f), MyBox->C.Z),
				Vector(LeftWidth / 2.0f, LeftHeight / 2.0f, LeftDepth / 2.0f));
			AABB* RightBox = new AABB(Point(MyBox->C.X, MaxY - (RightHeight / 2.0f), MyBox->C.Z),
				Vector(RightWidth / 2.0f, RightHeight / 2.0f, RightDepth / 2.0f));
			Left = new KD_Node(*LeftBox);
			Left->Depth = Depth + 1;
			Right = new KD_Node(*RightBox);
			Right->Depth = Depth + 1;
			break;
		}
		case Z_AXIS:
		{
			LeftWidth = RootW, RightWidth = RootW;
			LeftHeight = RootH, RightHeight = RootH;
			LeftDepth = splitdata->Value - MinZ, RightDepth = RootD - LeftDepth;
			AABB* LeftBox = new AABB(Point(MyBox->C.X, MyBox->C.Y, MinZ + (LeftDepth / 2.0f)),
				Vector(LeftWidth / 2.0f, LeftHeight / 2.0f, LeftDepth / 2.0f));
			AABB* RightBox = new AABB(Point(MyBox->C.X, MyBox->C.Y, MaxZ - (RightDepth / 2.0f)),
				Vector(RightWidth / 2.0f, RightHeight / 2.0f, RightDepth / 2.0f));
			Left = new KD_Node(*LeftBox);
			Left->Depth = Depth + 1;
			Right = new KD_Node(*RightBox);
			Right->Depth = Depth + 1;
			break;
		}
	}
}

void KD_Node::DivideTriangles(int& pointsleft, int& pointsright, int& pointson) const
{
	for (unsigned i = 0; i < TriangleList.size(); i++)
	{
		Triangle CurrentTriangle = TriangleList[i];
		DivideTrianglePoint(CurrentTriangle.P, pointsleft, pointsright, pointson);
		DivideTrianglePoint(CurrentTriangle.Q, pointsleft, pointsright, pointson);
		DivideTrianglePoint(CurrentTriangle.R, pointsleft, pointsright, pointson);

		switch (pointsleft)
		{
			case 0:
			{
				Right->TriangleList.push_back(CurrentTriangle); //add to right side if all points on the right or the edge
				break;
			}
			case 1:
			{
				if (pointson == 0 || pointson == 1)
				{
					Triangle Copy = CurrentTriangle;
					Right->TriangleList.push_back(CurrentTriangle);
					Left->TriangleList.push_back(Copy);
				}
				else
					Left->TriangleList.push_back(CurrentTriangle);
				break;
			}
			case 2:
			{
				if (pointson == 1)
					Left->TriangleList.push_back(CurrentTriangle);
				else
				{
					Triangle Copy = CurrentTriangle;
					Right->TriangleList.push_back(CurrentTriangle);
					Left->TriangleList.push_back(Copy);
				}
				break;
			}
			case 3:
			{
				Left->TriangleList.push_back(CurrentTriangle);
				break;
			}
		}
	}
}

void KD_Node::DivideTrianglePoint(const Point& p, int& pointsleft, int& pointsright, int& pointson) const
{
	if (p.IsIn(*Left->MyBox))
	{
		if ((p.IsOn(*Left->MyBox) && (p.IsOn(*Right->MyBox))))
			++pointson;
		else
			++pointsleft;
	}
	else if (p.IsIn(*Right->MyBox))
	{
		if ((p.IsOn(*Left->MyBox) && (p.IsOn(*Right->MyBox))))
			++pointson;
		else
			++pointsright;
	}
	else
	{
		if (p.Distance(*Left->MyBox) < p.Distance(*Right->MyBox))
			++pointsleft;
		else
			++pointsright;
	}
}