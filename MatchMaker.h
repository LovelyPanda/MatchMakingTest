#ifndef MATCHMAKER_H
#define MATCHMAKER_H

#define MAX_NUM_PLAYERS (1000000)
#define MAX_ONLINE_PLAYERS (MAX_NUM_PLAYERS / 10)

#include "Synchonization.h"

//we are going to use SIMD extensions
#include <xmmintrin.h>

//we are not going to implement hash-table by ourselves
#include <unordered_map>

class MatchMaker
{
public:

    // keep these methods
    static MatchMaker&    GetInstance(); 

    bool                AddUpdatePlayer(
        unsigned int    aPlayerId, 
        float           aPreferenceVector[20]); 

    bool                SetPlayerAvailable(
        unsigned int    aPlayerId); 

    bool                SetPlayerUnavailable(
        unsigned int    aPlayerId); 

    bool                MatchMake(
        unsigned int    aPlayerId, 
        unsigned int    aPlayerIds[20], 
        int&            aOutNumPlayerIds); 

    void                RemoveInstance();
private: 
   typedef unsigned int PlayerId;

   //to make the code more cache-friendly I am going to use data-oriented approach and eliminate the Player structure
   //here is the preference vector that can be operated by SIMD instructions 
   //(actually in our case the default alignment should be fine, but this way of declaration is more explicit)
   struct Vector20f
   {
       __m128 data[5]; // 5 * 4 = 20
   };

   //we store online players' preferences in one alligned contiguous memory location to make performance of the traversal maximum
   Vector20f myOnlinePlayersPreferences[MAX_ONLINE_PLAYERS]; //we assume that all the players at once can not be online
   unsigned int myOnlinePlayersNum;

   //map to retrieve player ids from position in online players preferences array
   PlayerId myOnlinePlayersPrefencesToIdsMap[MAX_ONLINE_PLAYERS];

   //we are not going to iterate through this data, so we can store it in struct for simplicity
   //these structures will be stored in hashmap
   struct PlayerData
   {
       bool myIsAvailable;
       unsigned int myOnlinePosition; //for online user it contains the position in online preferences array
       float myPreferences[20];
   };

   //map of all the players
   typedef std::unordered_map<PlayerId, PlayerData> PlayerHashMap;
   PlayerHashMap myPlayerMap;

    MatchMaker(); 

    ~MatchMaker();

    static MatchMaker*         ourInstance; //have no idea how you name static variables. If object's variable is 'my', class' variables should be 'our' :)

	//synchronization data
	ConditionData myConditionData;

	int myReadersNum;
	int myWritersNum;
};

#endif // MATCHMAKER_H