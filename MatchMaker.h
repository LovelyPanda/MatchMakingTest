#ifndef MATCHMAKER_H
#define MATCHMAKER_H

#define MAX_NUM_PLAYERS (1000000)
#define PLAYER_PREFERENCES_NUM 20

#include "Mutex.h"

class MatchMaker
{
public:

	// keep these methods
	static MatchMaker&	GetInstance(); 

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

    void                RemoveInstance();
private: 

	// I don't care if you change anything below this 

	struct Player 
	{
		unsigned int	myPlayerId; 
		float			myPreferenceVector[PLAYER_PREFERENCES_NUM];
		bool			myIsAvailable; 
	};

	Mutex				myLock; 
	int					myNumPlayers; 
	Player*				myPlayers[MAX_NUM_PLAYERS]; 

						MatchMaker(); 

						~MatchMaker();

    static MatchMaker*         ourInstance; //have no idea how you name static variables. Considering that object's variable is named as 'my', class' variables should be 'our' :)
};

#endif // MATCHMAKER_H