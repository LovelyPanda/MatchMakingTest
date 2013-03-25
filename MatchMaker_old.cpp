#include "stdafx.h"

#include "MatchMaker_old.h"

#include <algorithm>
#include <math.h>

	MatchMaker_old::MatchMaker_old()
		: myNumPlayers(0)
	{
	}

	MatchMaker_old::~MatchMaker_old()
	{
	}

	MatchMaker_old&
	MatchMaker_old::GetInstance()
	{
		static MatchMaker_old* instance = new MatchMaker_old(); 
		return *instance; 
	}

	bool
	MatchMaker_old::AddUpdatePlayer(
		unsigned int	aPlayerId, 
		float			aPreferenceVector[20])
	{
		MutexLock lock(myLock); 

		for(unsigned int i = 0; i < myNumPlayers; i++)
		{
			if(myPlayers[i]->myPlayerId == aPlayerId)
			{
				Player* t = new Player(); 
				t->myPlayerId = myPlayers[i]->myPlayerId; 
				t->myIsAvailable = myPlayers[i]->myIsAvailable; 
				for(int j = 0; j < 20; j++)
					t->myPreferenceVector[j] = aPreferenceVector[j]; 

				delete myPlayers[i]; 
				myPlayers[i] = t; 

				return true; 
			}
		}

		if(myNumPlayers == MAX_NUM_PLAYERS)
			return false; 

		myPlayers[myNumPlayers] = new Player(); 
		myPlayers[myNumPlayers]->myPlayerId = aPlayerId; 
		for(int i = 0; i < 20; i++)
			myPlayers[myNumPlayers]->myPreferenceVector[i] = aPreferenceVector[i]; 
		myPlayers[myNumPlayers]->myIsAvailable = false; 

		myNumPlayers++; 

		if(myNumPlayers % 100 == 0)
			printf("num players in system %u\n", myNumPlayers); 

		return true; 
	}

	bool
	MatchMaker_old::SetPlayerAvailable(
		unsigned int	aPlayerId)
	{
		MutexLock lock(myLock); 

		for(unsigned int i = 0; i < myNumPlayers; i++)
		{
			if(myPlayers[i]->myPlayerId == aPlayerId)
			{
				Player* t = new Player(); 
				t->myPlayerId = myPlayers[i]->myPlayerId; 
				t->myIsAvailable = true; 
				for(int j = 0; j < 20; j++)
					t->myPreferenceVector[j] = myPlayers[i]->myPreferenceVector[j]; 

				delete myPlayers[i]; 
				myPlayers[i] = t; 

				return true; 
			}
		}

		return false; 
	}

	bool
	MatchMaker_old::SetPlayerUnavailable(
		unsigned int	aPlayerId)
	{
		MutexLock lock(myLock); 

		for(unsigned int i = 0; i < myNumPlayers; i++)
		{
			if(myPlayers[i]->myPlayerId == aPlayerId)
			{
				Player* t = new Player(); 
				t->myPlayerId = myPlayers[i]->myPlayerId; 
				t->myIsAvailable = false; 
				for(int j = 0; j < 20; j++)
					t->myPreferenceVector[j] = myPlayers[i]->myPreferenceVector[j]; 

				delete myPlayers[i]; 
				myPlayers[i] = t; 

				return true; 
			}
		}

		return false; 
	}

	static float 
	Dist(
		float	aA[20], 
		float	aB[20])
	{
		float dist = 0.0f; 
		for(int i = 0; i < 20; i++)
			dist += pow((aA[i] - aB[i]), 2.0f); 

		return sqrt(dist); 
	}

	class Matched
	{
	public:

		float			myDist; 
		unsigned int	myId; 
	};

	static int 
	MatchComp(
		Matched*	aA, 
		Matched*	aB)
	{
		if(aA->myDist < aB->myDist)
			return 1; 

		return 0; 
	}

	bool
	MatchMaker_old::MatchMake(
		unsigned int	aPlayerId, 
		unsigned int	aPlayerIds[20], 
		int&			aOutNumPlayerIds)
	{
		MutexLock lock(myLock); 

		Player* playerToMatch = NULL; 
		for(unsigned int i = 0; i < myNumPlayers; i++)
		{
			if(myPlayers[i]->myPlayerId == aPlayerId)
			{
				playerToMatch					= new Player();
				playerToMatch->myPlayerId		= myPlayers[i]->myPlayerId;
				playerToMatch->myIsAvailable	= myPlayers[i]->myIsAvailable; 
				for(int j = 0; j < 20; j++)
					playerToMatch->myPreferenceVector[j] = myPlayers[i]->myPreferenceVector[j]; 

				break; 
			}
		}

		if(!playerToMatch)
			return false; 

		Matched** matched = new Matched*[20]; 
		for(unsigned int i = 0; i < 20; i++)
		{
			matched[i]			= new Matched(); 
			matched[i]->myDist	= -1.0f; 
			matched[i]->myId	= -1; 
		}

		int matchCount = 0; 

		for(unsigned int i = 0; i < myNumPlayers; i++)
		{
            //BUG FIX: we don't want to return off-line players and we don't want to return our own player
            if(!myPlayers[i]->myIsAvailable || myPlayers[i]->myPlayerId == aPlayerId)
                continue;
			//END BUG FIX

			if(matchCount < 20)
			{
				matched[matchCount]->myId	= myPlayers[i]->myPlayerId; 
				matched[matchCount]->myDist	= Dist(myPlayers[i]->myPreferenceVector, playerToMatch->myPreferenceVector);
				matchCount++; 

				using std::sort; 
				sort(matched, matched + matchCount, MatchComp);

				continue; 
			}

			float dist = Dist(playerToMatch->myPreferenceVector, myPlayers[i]->myPreferenceVector); 

			int index = -1; 
			for(int j = 19; j >= 0; j--)
			{
				if(matched[j]->myDist < dist)
					break; 

				index = j; 
			}

			if(index == -1)
				continue; 

			if(!myPlayers[i]->myIsAvailable)
				continue; 

			for(int j = 19; j > index; j--)
			{
				matched[j]->myDist	= matched[j - 1]->myDist; 
				matched[j]->myId	= matched[j - 1]->myId; 
			}

			matched[index]->myDist	= dist;
			matched[index]->myId	= myPlayers[i]->myPlayerId; 

			for(int j = 0; j < 20; j++)
				aPlayerIds[j] = matched[j]->myId; 
		}

		aOutNumPlayerIds = matchCount; 
		for(unsigned int j = 0; j < matchCount; j++)
			aPlayerIds[j] = matched[j]->myId; 

		for(unsigned int i = 0; i < 20; i++)
			delete matched[i]; 
		delete [] matched; 

		delete playerToMatch; 

		return true; 
	}
