#ifndef SIMPLEPOOL_H
#define SIMPLEPOOL_H

#include <vector>
#include <algorithm>

/**
 * Simple pool implementation to accelerate dynamic allocations.
 *
 */

template <typename T>
class SimplePool
{
public:
    SimplePool(
        unsigned int inMaxSize)
    {
        myPool.resize(inMaxSize);
        myPoolAvailability.resize(inMaxSize, false);
    }

    T*
    Allocate() //TODO: implement randomization
    {
        std::vector<bool>::iterator iter = std::find(myPoolAvailability.begin(), myPoolAvailability.end(), false);

        //check for space
        if(iter == myPoolAvailability.end())
        {
            myPoolAvailability.push_back(true);
            myPool.push_back(T());
            return &myPool[myPool.size() - 1];
        }

        *iter = true;
        size_t position = iter - myPoolAvailability.begin();
        return &myPool[position];
    }

    void
    Remove(
        T* pointer)
    {
        size_t position = &myPool[0] - pointer;
        *pointer = T(); //clear value
        myPoolAvailability[position] = false;
    }

private:
    SimplePool(
        const SimplePool&);
    SimplePool&
        operator=(
        const SimplePool&);

    std::vector<T> myPool;
    std::vector<bool> myPoolAvailability;
};

#endif //SIMPLEPOOL_H