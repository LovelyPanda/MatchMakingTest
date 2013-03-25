#ifndef MATCHMAKER_OLD_H
#define MATCHMAKER_OLD_H

//This is original implementation of match making system.
//I kept it for correctness checking.
//No changes were made except for bug fixing.

#define MAX_NUM_PLAYERS (1000000)

#include "Synchonization.h"

class MatchMaker_old
{
public:

	// keep these methods
	static MatchMaker_old&	GetInstance(); 

	bool				AddUpdatePlayer(
							unsigned int	aPlayerId, 
							float			aPreferenceVector[20]); 

	bool				SetPlayerAvailable(
							unsigned int	aPlayerId); 

	bool				SetPlayerUnavailable(
							unsigned int	aPlayerId); 

	bool				MatchMake(
							unsigned int	aPlayerId, 
							unsigned int	aPlayerIds[20], 
							int&			aOutNumPlayerIds); 

private: 

	// I don't care if you change anything below this 

	class Player 
	{
	public:

		Player()
		{
			myPreferenceVector = new float[20]; 
		}

		~Player()
		{
			delete [] myPreferenceVector; 
		}

		unsigned int	myPlayerId; 
		float*			myPreferenceVector; 
		bool			myIsAvailable; 
	};

	Mutex				myLock; 
	int					myNumPlayers; 
	Player*				myPlayers[MAX_NUM_PLAYERS]; 

						MatchMaker_old(); 

						~MatchMaker_old(); 
};

#endif // MATCHMAKER_H