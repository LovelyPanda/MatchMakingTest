#ifndef MATCHMAKER_H
#define MATCHMAKER_H

#define MAX_NUM_PLAYERS (1000000)
#define PLAYER_PREFERENCES_NUM (20)
#define MATCHMAKE_PLAYERS_NUM (20)

#include "Mutex.h"
#include "SimplePool.h"
#include "KDPartitioningTree.h"
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

    // I don't care if you change anything below this 

    struct Player 
    {
        unsigned int    myPlayerId; 
        Point<PLAYER_PREFERENCES_NUM> myPreferenceVector;
        bool            myIsAvailable; 
    };

    Mutex                myLock; 

    //int                    myNumPlayers; 
    //Player*                myPlayers[MAX_NUM_PLAYERS]; 

    typedef std::unordered_map<unsigned int, Player>                 HashMap;
    typedef KDPartitioningTree<unsigned int, PLAYER_PREFERENCES_NUM> PartitioningTree;

    SimplePool<PartitioningTree::Node>         myNodePool;
    //SimplePool<Point<PLAYER_PREFERENCES_NUM> > myPointPool;

    HashMap          myPlayers;
    PartitioningTree myPartitioningTree;

    MatchMaker(); 

    ~MatchMaker();

    static MatchMaker*         ourInstance; //have no idea how you name static variables. If object's variable is 'my', class' variables should be 'our' :)
};

#endif // MATCHMAKER_H