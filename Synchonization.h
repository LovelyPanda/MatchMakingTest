#ifndef MUTEX_H
#define MUTEX_H

#include <intrin.h>
#include <cstdint>

#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedCompareExchange)

	//This mutex is bad because it is not starvation-free.
	//In order to make it starvation-free we would need to implement queue of some kind
	//and we should be able to manage threads manually.
	//Still this mutex would be worse than the one from that is provided by OS
	//as (I believe) task scheduler is aware of default sync. primitives and can make
	//some manipulations with threads (like 'priority ceiling'). But he can't do that
	//in case of our own mutex he is on aware of. See GoodMutex implementation based on critical section from WinAPI.
    class BadMutex
    {
    public:

        BadMutex()
            : mySpinLock(0)
        {
        }

        ~BadMutex()
        {
        }

        void
        Lock() //optimized not to make unnecessary bus locks. Still it is not starvation-free.
        {
            while(true)
            {
                //first we check the value in memory (by simple read)
                //note that reading of aligned 32-bit value is atomic on most systems. That's why we will not get garbage at this point.
                if(mySpinLock == 0)
                {
                    //looks like the lock is free
                    //try to lock it with compare and swap
                    if(_InterlockedCompareExchange(&mySpinLock, 1, 0) == 0)
                    {
                        //we managed to lock it, exit the loop
                        break;
                    }
                }

                //lets switch context because we need to wait for lock (and in our case we may wait for some time)
				//Note: in case when we know that wait won't take long we can 'busyloop'.
				//Unfortunately we have 16 threads (more than the number of hardware threads on both of my machines),
				//that means some threads should go to sleep while waiting to give others excecution time.
				//Also we could busyloop for some time and fall back to sleep after.
                Sleep(0);
            }
        }

        void 
        Unlock()
        {
            if(mySpinLock) //we don't want to lock the bus every time
                _InterlockedExchange(&mySpinLock, 0);
        }

        volatile LONG mySpinLock;
    };

	//I would prefer such Mutex in production code :)
    class GoodMutex
    {
    public:
        GoodMutex()
        {
            InitializeCriticalSection(&criticalSection);
        }

        ~GoodMutex()
        {
            DeleteCriticalSection(&criticalSection);
        }

        void
            Lock()
        {
            EnterCriticalSection(&criticalSection);
        }

        void 
            Unlock()
        {
            LeaveCriticalSection(&criticalSection);
        }
    private:
        CRITICAL_SECTION criticalSection;
    };

    template <typename MUTEX>
    class LockGuard
    {
    public:

        LockGuard(
            MUTEX*    aLock)
            : myLock(aLock)
        {
            myLock->Lock(); 
        }

        LockGuard(
            MUTEX&    aLock)
            : myLock(&aLock)
        {
            myLock->Lock(); 
        }

        ~LockGuard()
        {
            myLock->Unlock();  
        }

    private:
        //let's make it uncopyable
        LockGuard(
            const LockGuard&);
        LockGuard&
        operator=(
            const LockGuard&);

        MUTEX*    myLock;
    };

	//utility classes based on conditional variables

	struct ConditionData
	{
		ConditionData()
		{
			InitializeCriticalSection(&myCrit);
			InitializeConditionVariable(&myCond);
		}

		~ConditionData()
		{
			DeleteCriticalSection(&myCrit);
		}

		CONDITION_VARIABLE myCond;
		CRITICAL_SECTION   myCrit;
	};

	class ReaderCondition
	{
	public:
		ReaderCondition(
			ConditionData* data,
			int* readers,
			int* writers):
				myData(data),
				myReaders(readers),
				myWriters(writers)
		{
			//lock crit
			EnterCriticalSection(&myData->myCrit);

			while(*myWriters > 0)
			{
				SleepConditionVariableCS(&myData->myCond, &myData->myCrit, INFINITE);
			}

			//increment readers num
			++(*myReaders);

			LeaveCriticalSection(&myData->myCrit);
			WakeAllConditionVariable(&myData->myCond);
		}

		~ReaderCondition()
		{
			EnterCriticalSection(&myData->myCrit);

			--(*myReaders);

			LeaveCriticalSection(&myData->myCrit);
			WakeAllConditionVariable(&myData->myCond);
		}

	private:
		ConditionData* myData;
		int* myWriters;
		int* myReaders;
	};

	class WriterCondition
	{
	public:
		WriterCondition(
			ConditionData* data,
			int* readers,
			int* writers):
				myData(data),
				myReaders(readers),
				myWriters(writers)
		{
			//lock crit
			EnterCriticalSection(&myData->myCrit);

			while(*writers > 0 && *readers > 0)
			{
				SleepConditionVariableCS(&myData->myCond, &myData->myCrit, INFINITE);
			}

			//increment writers num
			++(*myWriters);

			LeaveCriticalSection(&myData->myCrit);
			WakeAllConditionVariable(&myData->myCond);
		}

		~WriterCondition()
		{
			EnterCriticalSection(&myData->myCrit);

			--(*myWriters);

			LeaveCriticalSection(&myData->myCrit);
			WakeAllConditionVariable(&myData->myCond);
		}

	private:
		ConditionData* myData;
		int* myWriters;
		int* myReaders;
	};



	//we use bad mutex by default, you may change it to compare with the other one
    typedef BadMutex Mutex;
    typedef LockGuard<Mutex> MutexLock;

#endif // MUTEX_H