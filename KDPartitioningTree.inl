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
            if(node->myPoints[i][dimension] < splitBorder)
                nodeToPutInto = node->myFirstChild;
            else
                nodeToPutInto = node->mySecondChild;

            //nodeToPutInto->myPoints[nodeToPutInto->myPointsNum] = 
        }
    }

    //recursive add point
    template <typename T, unsigned int DIMENSION_NUM>
    void
    RecursiveAddPoint(
        Point<DIMENSION_NUM>* newPoint,
        typename KDPartitioningTree<T, DIMENSION_NUM>::Node* node,
        SimplePool<typename KDPartitioningTree<T, DIMENSION_NUM>::Node>& pool,
        unsigned int depth = 0)
    {
        if(!node->myFirstChild && !node->mySecondChild)
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

template <typename T, unsigned int DIMENSION_NUM>
void 
KDPartitioningTree<T, DIMENSION_NUM>::AddPoint(Point<DIMENSION_NUM>* newPoint)
{
    kdpartitiontree_impl::RecursiveAddPoint<T, DIMENSION_NUM>(newPoint, myRoot, myPool);
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
