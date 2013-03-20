#include "KDPartitioningTree.h"
#include <cassert>

namespace kdpartitiontree_impl
{
    template <typename T, unsigned int DIMENSION_NUM>
    void
    FreeNode(
        SimplePool<typename KDPartitioningTree<T, DIMENSION_NUM>::Node>& pool,
        typename KDPartitioningTree<T, DIMENSION_NUM>::Node* node)
    {
        if(!node) return;

        FreeNode<T, DIMENSION_NUM>(pool, node->myFirstChild);
        FreeNode<T, DIMENSION_NUM>(pool, node->mySecondChild);

        pool.Remove(node);
    }

}

template <typename T, unsigned int DIMENSION_NUM>
KDPartitioningTree<T, DIMENSION_NUM>::~KDPartitioningTree()
{
    //recursively release all nodes
    kdpartitiontree_impl::FreeNode<T, DIMENSION_NUM>(myPool, myRoot);
}

namespace kdpartitiontree_impl
{

    //splits the provided node to form new subnodes filled with points from original node.
    //depth parameter is used to determine how to split the node (along which axis)
    template <typename T, unsigned int DIMENSION_NUM>
    void
    SplitPartition(
        typename KDPartitioningTree<T, DIMENSION_NUM>::Node* node,
        SimplePool<typename KDPartitioningTree<T, DIMENSION_NUM>::Node>& pool,
        unsigned int depth)
    {
        assert(node);
        assert(node->myFirstChild == 0 && node->mySecondChild == 0);

        
        //split along one of the dimensions using depth value
        unsigned int dimension = depth % DIMENSION_NUM;
        float splitBorder = (node->mySpaceUpperBoundaries[dimension] + node->mySpaceLowerBoundaries[dimension]) / 2.0f;

        node->myFirstChild = pool.Allocate();
        node->mySecondChild = pool.Allocate();

        node->myFirstChild->myParent = node;
        node->mySecondChild->myParent = node;

        //copy boundaries
        node->myFirstChild->mySpaceUpperBoundaries = node->mySpaceUpperBoundaries;
        node->myFirstChild->mySpaceLowerBoundaries = node->mySpaceLowerBoundaries;
        node->mySecondChild->mySpaceUpperBoundaries = node->mySpaceUpperBoundaries;
        node->mySecondChild->mySpaceLowerBoundaries = node->mySpaceLowerBoundaries;

        //change the values
        node->myFirstChild->mySpaceUpperBoundaries[dimension] = splitBorder;
        node->mySecondChild->mySpaceLowerBoundaries[dimension] = splitBorder;

        //split points
        for(size_t i = 0; i < node->myPointsNum; ++i)
        {
            typename KDPartitioningTree<T, DIMENSION_NUM>::Node* nodeToPutInto;
            if((*node->myPoints[i])[dimension] < splitBorder)
                nodeToPutInto = node->myFirstChild;
            else
                nodeToPutInto = node->mySecondChild;

            //add point
            nodeToPutInto->myPoints[nodeToPutInto->myPointsNum] = node->myPoints[i];
            nodeToPutInto->myPointData[nodeToPutInto->myPointsNum] = node->myPointData[i];
            ++nodeToPutInto->myPointsNum;
        }

        node->myChildPointsNum = node->myPointsNum;
        
        //clear points
        node->myPointsNum = 0;
        memset(node->myPoints, 0, sizeof(node->myPoints));
        memset(node->myPointData, 0, sizeof(node->myPointData));
    }

    //recursive add point
    template <typename T, unsigned int DIMENSION_NUM>
    void
    RecursiveAddPoint(
        Point<DIMENSION_NUM>* newPoint,
        T newPointData,
        typename KDPartitioningTree<T, DIMENSION_NUM>::Node* node,
        SimplePool<typename KDPartitioningTree<T, DIMENSION_NUM>::Node>& pool,
        unsigned int depth = 0)
    {
        if(!node->myFirstChild && !node->mySecondChild)
        {
            //leaf

            //check for space before addition
            if(node->myPointsNum < node->MAX_POINTS_NUM)
            {
                //just add
                node->myPoints[node->myPointsNum] = newPoint;
                node->myPointData[node->myPointsNum] = newPointData;
                ++node->myPointsNum;

                return;
            }
            else
            {
                //split (point will be added later)
                SplitPartition<T, DIMENSION_NUM>(node, pool, depth);
            }
        }

        //recursively go deeper
        ++node->myChildPointsNum;

        unsigned int dimension = depth % DIMENSION_NUM;
        float splitBorder = (node->mySpaceUpperBoundaries[dimension] + node->mySpaceLowerBoundaries[dimension]) / 2.0f;

        if((*newPoint)[dimension] < splitBorder)
            RecursiveAddPoint<T, DIMENSION_NUM>(newPoint, newPointData, node->myFirstChild, pool, depth + 1);
        else
            RecursiveAddPoint<T, DIMENSION_NUM>(newPoint, newPointData, node->mySecondChild, pool, depth + 1);
    }

}

template <typename T, unsigned int DIMENSION_NUM>
void 
KDPartitioningTree<T, DIMENSION_NUM>::AddPoint(Point<DIMENSION_NUM>* newPoint, T newPointData)
{
    kdpartitiontree_impl::RecursiveAddPoint<T, DIMENSION_NUM>(newPoint, newPointData, myRoot, myPool);
}

template <typename T, unsigned int DIMENSION_NUM>
void 
KDPartitioningTree<T, DIMENSION_NUM>::RemovePoint(const Point<DIMENSION_NUM>* pointToRemove)
{

}

template <typename T, unsigned int DIMENSION_NUM>
void 
KDPartitioningTree<T, DIMENSION_NUM>::OptimizeTree()
{

}

template <typename T, unsigned int DIMENSION_NUM>
unsigned int 
KDPartitioningTree<T, DIMENSION_NUM>::FindNearestNeighbors(
    const Point<DIMENSION_NUM>& point,
    int numberOfNeightboursToSearchFor, 
    std::pair<Point<DIMENSION_NUM>*, T>* outNeighbors) const
{
    return 0;
}
