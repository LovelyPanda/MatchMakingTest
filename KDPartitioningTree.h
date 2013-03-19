#ifndef KDPARTITIONINGTREE_H
#define KDPARTITIONINGTREE_H

/** 
 * Multidimensional space partitioning tree.
 *
 * Optimized for values in range [0.0f, 1.0f]
 */

#include <cstring>
#include "SimplePool.h"

template <unsigned int DIMENSION_NUM>
struct Point
{
	Point()
	{
		memset(values, 0, sizeof(values));
	}

	Point(float value)
	{
		int intValue = *reinterpret_cast<int*>(&value);
		memset(values, intValue, sizeof(values));
	}

	float& operator[](unsigned int index)
	{
		return values[index];
	}

    const float& operator[](unsigned int index) const
	{
		return values[index];
	}

	float values[DIMENSION_NUM];
};

template <unsigned int DIMENSION_NUM>
class KDPartitioningTree /* noncopyable */
{
public:
    struct Node
    {
        Node* myParent;

        Node* myFirstChild;
        Node* mySecondChild;

		static const unsigned int MAX_POINTS_NUM = 100;
        Point<DIMENSION_NUM>* myPoints[MAX_POINTS_NUM];
        unsigned int myPointsNum;

		//space partition boundaries
		Point<DIMENSION_NUM> mySpaceUpperBoundaries;
		Point<DIMENSION_NUM> mySpaceLowerBoundaries;

        unsigned int myChildPointsNum;
    };
    
    KDPartitioningTree(
        SimplePool<Node>& pool): myPool(pool)
    {
        myRoot = myPool.Allocate();

		myRoot.mySpaceUpperBoundaries = Point<DIMENSION_NUM>(1.0f);
		myRoot.mySpaceLowerBoundaries = Point<DIMENSION_NUM>(0.0f);
    }

	~KDPartitioningTree();

	//note that no point copies are created
	void AddPoint(Point<DIMENSION_NUM>* newPoint);
	void RemovePoint(Point<DIMENSION_NUM>* pointToRemove);

	//Further partition space if needed or eliminate sub-spaces that are to sparce.
	//You should call this function from time to time after significant number of point additions or removals. This may be an expensive operation.
	void OptimizeTree();

	//searches for nearest neighbours and returns the number of found neighbours (normally return value should be the same you pass as the first argument)
	unsigned int FindNearestNeighbours(int numberOfNeightbours, Point<DIMENSION_NUM>* outNeighbours) const;

private:
    KDPartitioningTree(const KDPartitioningTree&);
    KDPartitioningTree& operator=(const KDPartitioningTree&);

    Node* myRoot;
    SimplePool<Point<DIMENSION_NUM> >& myPool;
};


#endif //KDPARTITIONINGTREE_H