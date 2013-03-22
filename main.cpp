#include "stdafx.h"

#include "MatchMaker.h"
#include "MatchMaker_old.h"
#include <algorithm>

class QPTimer 
{
public: 

    QPTimer()
    {
        LARGE_INTEGER freq; 
        QueryPerformanceFrequency(&freq); 

        myInvFreq = 1000000.0 / (double) freq.QuadPart; 
    }

    void
        Start()
    {
        QueryPerformanceCounter(&myStart); 
    }

    // returns micro seconds
    double
        Get()
    {
        QueryPerformanceCounter(&myStop);

        double t = (double)(myStop.QuadPart - myStart.QuadPart) * myInvFreq;

        myStart = myStop; 

        return t; 
    }

    //returns micro seconds without stop
    double
        Read()
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        return (double)(now.QuadPart - myStart.QuadPart) * myInvFreq;
    }

    LARGE_INTEGER myStart; 
    LARGE_INTEGER myStop; 
    double myInvFreq; 
};

class ScopedQPTimer
{
public: 

    ScopedQPTimer(
        const char*        aMsg = NULL)
    {
        myMsg = aMsg; 
        myTimer.Start(); 
    }

    ~ScopedQPTimer()
    {
        double used = myTimer.Get(); 
        printf("%s %f\n", myMsg, used / 1000.0); 
    }

    QPTimer        myTimer; 
    const char*    myMsg; 
}; 

//#define USE_PREDICTABLE_RANDOMNESS

unsigned int 
    RandomUInt32()
{
#ifdef USE_PREDICTABLE_RANDOMNESS

    return (((unsigned int) rand()) << 16) | rand(); // I think this code is wrong. Results have strange ranges.

#else

    // better random function 
    unsigned int r; 
    rand_s(&r);
    return r; 

#endif
}

float 
    RandomFloat32()
{
    return (float) RandomUInt32() / (float) 0xFFFFFFFF; 
}

CRITICAL_SECTION criticalSection;

namespace 
{
    void 
        Run(
        void*    aIgnore)
    {
        unsigned int matchmakingNum = 0;
        double totalMatchmakingTime = 0.0;

        for(;;)
        {
            //EnterCriticalSection(&criticalSection);

            // add or update a random player to the system 
            float preferenceVector[20]; 
            for(int i = 0; i < 20; i++)
                preferenceVector[i] = RandomFloat32(); 

            unsigned int playerId = RandomUInt32() % 1000000; 
            MatchMaker::GetInstance().AddUpdatePlayer(playerId, preferenceVector); 
            MatchMaker_old::GetInstance().AddUpdatePlayer(playerId, preferenceVector); 

            // players goes on-line / off-line all the time 
            if(RandomFloat32() < 0.05f)
            {
                MatchMaker::GetInstance().SetPlayerAvailable(playerId); 
                MatchMaker_old::GetInstance().SetPlayerAvailable(playerId); 
            }
            else 
            {
                MatchMaker::GetInstance().SetPlayerUnavailable(playerId); 
                MatchMaker_old::GetInstance().SetPlayerUnavailable(playerId); 
            }

            // match make a player
            unsigned int ids[20]; 
            unsigned int referenceIds[20];
            int numPlayers;
            int numReferencePlayers;

            {
                ScopedQPTimer timer("    matching time in milliseconds"); 
                MatchMaker::GetInstance().MatchMake(playerId, ids, numPlayers);
                totalMatchmakingTime += timer.myTimer.Read();
            }

            //{
            //    ScopedQPTimer timer("old matching time in milliseconds"); 
            //    MatchMaker_old::GetInstance().MatchMake(playerId, referenceIds, numReferencePlayers);
            //}

            //LeaveCriticalSection(&criticalSection);
            /*
            //check correctness of results
            std::sort(ids, ids + numPlayers);
            std::sort(referenceIds, referenceIds + numReferencePlayers);

            if(numPlayers != numReferencePlayers || memcmp(ids, referenceIds, sizeof(unsigned int) * numPlayers))
            {
                printf("WRONG RESULTS!!!!!\n");
            }
            */
            if(++matchmakingNum > 10)
            {
                printf("average matchmaking time: %f\n", totalMatchmakingTime / 10000.0);

                matchmakingNum = 0;
                totalMatchmakingTime = 0.0;
            }

        }
    }
}

class RequestThread 
{
public: 

    RequestThread()
    {
    }

    ~RequestThread()
    {
    }

    void 
        Start()
    {
        _beginthread(Run, 0, NULL);
    }
};

int main(int argc, char* argv[])
{
    MatchMaker::GetInstance(); 
    MatchMaker_old::GetInstance();

    // add 100000 players to the system before starting any request threads 
    for(int i = 0; i < 100000; i++)
    {
        float preferenceVector[20]; 
        for(int i = 0; i < 20; i++)
            preferenceVector[i] = RandomFloat32(); 

        unsigned int playerId = RandomUInt32() % 1000000; 
        MatchMaker::GetInstance().AddUpdatePlayer(playerId, preferenceVector); 
        MatchMaker_old::GetInstance().AddUpdatePlayer(playerId, preferenceVector);
    }

    InitializeCriticalSection(&criticalSection);

    printf("starting worker threads\n"); 

    for(int i = 0; i < 16; i++)
        (new RequestThread())->Start(); 

    Sleep(-1); 

    return 0;
}

