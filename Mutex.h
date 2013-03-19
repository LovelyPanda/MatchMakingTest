#ifndef MUTEX_H
#define MUTEX_H

#include <intrin.h>
#include <cstdint>

#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedCompareExchange)

	class Mutex
	{
	public:

		Mutex()
			: mySpinLock(0)
		{
		}

		~Mutex()
		{
		}

		void
		Lock() //optimized not to make unnecessary bus locks. Still it is not starvation-free. I would prefer some primitives from OS-SDKs.
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

                //lets switch context because we need to wait for lock
                Sleep(0);
            }
		}

		void 
		Unlock()
		{
            if(mySpinLock) //we don't want to lock the bus every time
			    _InterlockedExchange(&mySpinLock, 0);
		}

		volatile long mySpinLock; // I am not sure about LONG size on 64-bit platform, that's why I decided to use better (more explicit and cross-platform) data type.
	};

    //perfect place for template (in case we are going to have few different locks which is normally the case)

    template <typename MUTEX>
	class LockGuard
	{
	public:

		LockGuard(
			MUTEX*	aLock)
			: myLock(aLock)
		{
			myLock->Lock(); 
		}

		LockGuard(
			MUTEX&	aLock)
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

		MUTEX*	myLock;
	};

    typedef LockGuard<Mutex> MutexLock;

#endif // MUTEX_H