#include "stdafx.h"
#include "KDPartitioningTree.h"
#include <cassert>

namespace
{
	template <unsigned int DIMENSION_NUM>
	void
	FreeNode(
		SimplePool<Point<DIMENSION_NUM> >& pool,
		typename KDPartitioningTree<DIMENSION_NUM>::Node* node)
	{
		if(!node) return;

		FreeNode(node->firstChild);
		FreeNode(node->secondChild);

		pool.Remove(node);
	}

}

template <unsigned int DIMENSION_NUM>
KDPartitioningTree<DIMENSION_NUM>::~KDPartitioningTree()
{
	//recursively release all nodes
	FreeNode<DIMENSION_NUM>(myPool, myRoot);
}

namespace
{
	template <unsigned int DIMENSION_NUM>
	void
	SplitPartition(
		typename KDPartitioningTree<DIMENSION_NUM>::Node* node,
		SimplePool<typename KDPartitioningTree<DIMENSION_NUM>::Node>& pool,
		unsigned int depth)
	{
		assert(node);
		assert(node->firstChild == 0 && node->secondChild == 0);

		
		//split along one of the dimensions using depth value
		unsigned int dimension = depth % DIMENSION_NUM;
		float splitBorder = (node->mySpaceUpperBoundaries[dimension] + node->mySpaceLowerBoundaries[dimension]) / 2.0f;

		node->firstChild = pool.Allocate();
		node->secondChild = pool.Allocate();

		node->firstChild->parent = node;
		node->secondChild->parent = node;

		//copy boundaries
		//memcpy(&node->firstChild->mySpaceUpperBoundaries, node->mySpaceUpperBoundaries, sizeof(typename KDPartitioningTree<DIMENSION_NUM>::Point));
		//memcpy(&node->firstChild->mySpaceLowerBoundaries, node->mySpaceLowerBoundaries, sizeof(typename KDPartitioningTree<DIMENSION_NUM>::Point));
		//memcpy(&node->secondChild->mySpaceUpperBoundaries, node->mySpaceUpperBoundaries, sizeof(typename KDPartitioningTree<DIMENSION_NUM>::Point));
		//memcpy(&node->secondChild->mySpaceLowerBoundaries, node->mySpaceLowerBoundaries, sizeof(typename KDPartitioningTree<DIMENSION_NUM>::Point));
		node->firstChild->mySpaceUpperBoundaries = node->mySpaceUpperBoundaries;
		node->firstChild->mySpaceLowerBoundaries = node->mySpaceLowerBoundaries;
		node->secondChild->mySpaceUpperBoundaries = node->mySpaceUpperBoundaries;
		node->secondChild->mySpaceLowerBoundaries = node->mySpaceLowerBoundaries;

		//change the values
		node->firstChild->mySpaceUpperBoundaries[dimension] = splitBorder;
		node->secondChild->mySpaceLowerBoundaries[dimension] = splitBorder;

		//split points
		for(size_t i = 0; i < node->myPointsNum; ++i)
		{
			typename KDPartitioningTree<DIMENSION_NUM>::Node* nodeToPutInto;
			if(node->myPoints[i][dimension] < splitBorder)
				nodeToPutInto = node->myFirstChild;
			else
				nodeToPutInto = node->mySecondChild;

			//nodeToPutInto->myPoints[nodeToPutInto->myPointsNum] = 
		}
	}

	//recursive add point
	template <unsigned int DIMENSION_NUM>
	void
	RecursiveAddPoint(
		typename KDPartitioningTree<DIMENSION_NUM>::Point* newPoint,
		typename KDPartitioningTree<DIMENSION_NUM>::Node* node,
		SimplePool<typename KDPartitioningTree<DIMENSION_NUM>::Node>& pool,
		unsigned int depth = 0)
	{
		if(!firstChild && !secondChild)
		{
			//leaf

		}
		else
		{
			//recursively go deeper

			//if(
			//RecursiveAddPoint(newPoint, , pool, depth + 1);
		}
	}

}

template <unsigned int DIMENSION_NUM>
void 
KDPartitioningTree<DIMENSION_NUM>::AddPoint(Point<DIMENSION_NUM>* newPoint)
{
	RecursiveAddPoint(newPoint, myRoot, myPool);
}

template <unsigned int DIMENSION_NUM>
void 
KDPartitioningTree<DIMENSION_NUM>::RemovePoint(Point<DIMENSION_NUM>* pointToRemove)
{

}

template <unsigned int DIMENSION_NUM>
void 
KDPartitioningTree<DIMENSION_NUM>::OptimizeTree()
{

}
