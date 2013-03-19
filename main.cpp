#include "stdafx.h"

#include "MatchMaker.h"

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

		LARGE_INTEGER myStart; 
		LARGE_INTEGER myStop; 
		double myInvFreq; 
	};

	class ScopedQPTimer
	{
	public: 

		ScopedQPTimer(
			const char*		aMsg = NULL)
		{
			myMsg = aMsg; 
			myTimer.Start(); 
		}

		~ScopedQPTimer()
		{
			double used = myTimer.Get(); 
			printf("%s %f\n", myMsg, used / 1000.0); 
		}

		QPTimer		myTimer; 
		const char*	myMsg; 
	}; 

	#define USE_PREDICTABLE_RANDOMNESS

	unsigned int 
	RandomUInt32()
	{
	#ifdef USE_PREDICTABLE_RANDOMNESS

		return (((unsigned int) rand()) << 16) | rand(); 

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

namespace 
{
	void 
	Run(
		void*	aIgnore)
	{
		for(;;)
		{
			// add or update a random player to the system 
			float preferenceVector[20]; 
			for(int i = 0; i < 20; i++)
				preferenceVector[i] = RandomFloat32(); 

			unsigned int playerId = RandomUInt32() % 1000000; 
			MatchMaker::GetInstance().AddUpdatePlayer(playerId, preferenceVector); 

			// players goes on-line / off-line all the time 
			if(RandomFloat32() < 0.05f)
				MatchMaker::GetInstance().SetPlayerAvailable(playerId); 
			else 
				MatchMaker::GetInstance().SetPlayerUnavailable(playerId); 

			// match make a player
			unsigned int ids[20]; 
			int numPlayers; 

			ScopedQPTimer timer("matching time in milliseconds"); 
			MatchMaker::GetInstance().MatchMake(playerId, ids, numPlayers); 
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

	// add 100000 players to the system before starting any request threads 
	for(int i = 0; i < 100000; i++)
	{
		float preferenceVector[20]; 
		for(int i = 0; i < 20; i++)
			preferenceVector[i] = RandomFloat32(); 

		unsigned int playerId = RandomUInt32() % 1000000; 
		MatchMaker::GetInstance().AddUpdatePlayer(playerId, preferenceVector); 
	}

	printf("starting worker threads\n"); 

	for(int i = 0; i < 16; i++)
		(new RequestThread())->Start(); 

	Sleep(-1); 

	return 0;
}

