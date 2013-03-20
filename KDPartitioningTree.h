#ifndef KDPARTITIONINGTREE_H
#define KDPARTITIONINGTREE_H

/** 
 * Multidimensional space partitioning tree.
 *
 * Optimized for values in range [0.0f, 1.0f]
 */

#include <cstring>
#include <utility>
#include "SimplePool.h"

template <unsigned int DIMENSION_NUM>
struct Point
{
    Point()
    {
        memset(values, 0, sizeof(values));
    }

    Point(
        float value)
    {
        for(unsigned int i = 0; i < DIMENSION_NUM; ++i)
        {
            values[i] = value;
        }
    }

    Point(
        float data[DIMENSION_NUM])
    {
        memcpy(values, data, sizeof(values));
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

template <typename T, unsigned int DIMENSION_NUM>
class KDPartitioningTree /* noncopyable */
{
public:
    struct Node
    {
        Node()
            : myParent(0),
              myFirstChild(0),
              mySecondChild(0),
              myPointsNum(0),
              myChildPointsNum(0)
        {

        }

        Node* myParent;

        Node* myFirstChild;
        Node* mySecondChild;

        static const unsigned int MAX_POINTS_NUM = 100;
        Point<DIMENSION_NUM>* myPoints[MAX_POINTS_NUM];
        T myPointData[MAX_POINTS_NUM];
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

        myRoot->mySpaceUpperBoundaries = Point<DIMENSION_NUM>(1.0f);
        myRoot->mySpaceLowerBoundaries = Point<DIMENSION_NUM>(0.0f);
    }

    ~KDPartitioningTree();

    //note that no point copies are created
    void AddPoint(
        Point<DIMENSION_NUM>* newPoint,
        T newPointData);

    void RemovePoint(
        const Point<DIMENSION_NUM>* pointToRemove);

    //Further partition space if needed or eliminate sub-spaces that are to sparce.
    //You should call this function from time to time after significant number of point additions or removals. This may be an expensive operation.
    void OptimizeTree();

    //searches for nearest neighbors and returns the number of found neighbors (normally return value should be the same you pass as the first argument)
    unsigned int FindNearestNeighbors(
        const Point<DIMENSION_NUM>& point,
        int numberOfNeightborsToSearchFor, 
        std::pair<Point<DIMENSION_NUM>*, T>* outNeighbors) const;

private:
    KDPartitioningTree(const KDPartitioningTree&);
    KDPartitioningTree& operator=(const KDPartitioningTree&);

    Node* myRoot;
    //SimplePool<Point<DIMENSION_NUM> >& myPool;
    SimplePool<Node>& myPool;
};

#include "KDPartitioningTree.inl"

#endif //KDPARTITIONINGTREE_H