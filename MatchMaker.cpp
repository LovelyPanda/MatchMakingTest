#include "stdafx.h"

#include "MatchMaker.h"

#include <algorithm>
#include <math.h>

    MatchMaker* MatchMaker::ourInstance = 0;

    MatchMaker::MatchMaker()
        : myNodePool(20000), //~6 MB
          //myPointPool(200000), //16 MB
          myPlayers(120000), 
          myPartitioningTree(myNodePool)
    {
    }

    MatchMaker::~MatchMaker()
    {
    }

    MatchMaker&
    MatchMaker::GetInstance()
    {
        if(!ourInstance) ourInstance = new MatchMaker(); 
        return *ourInstance; 
    }

    void
    MatchMaker::RemoveInstance()
    {
        if(ourInstance) delete ourInstance;
        ourInstance = 0;
    }

    bool
    MatchMaker::AddUpdatePlayer(
        unsigned int    aPlayerId, 
        float           aPreferenceVector[20])
    {
        MutexLock lock(myLock); 

        /*
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
            */

        //check for player presence
        HashMap::iterator iter = myPlayers.find(aPlayerId);

        if(iter == myPlayers.end())
        {
            //add new player
            Player& newPlayer = myPlayers[aPlayerId];
            newPlayer.myPlayerId = aPlayerId;

            newPlayer.myPreferenceVector = aPreferenceVector;

            //add new point to space partitioning tree
            myPartitioningTree.AddPoint(&newPlayer.myPreferenceVector, aPlayerId);
        }
        else
        {
            //update player's preferences
            Point<PLAYER_PREFERENCES_NUM> oldPreferences = iter->second.myPreferenceVector;
            iter->second.myPreferenceVector = aPreferenceVector;

            //remove old point and insert the new one
            myPartitioningTree.RemovePoint(&oldPreferences);
            myPartitioningTree.AddPoint(&iter->second.myPreferenceVector, aPlayerId);
        }

        if(myPlayers.size() % 100 == 0)
            printf("num players in system %u\n", myPlayers.size());

        return true; 
    }

    bool
    MatchMaker::SetPlayerAvailable(
        unsigned int    aPlayerId)
    {
        MutexLock lock(myLock); 

        /*
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
        */

        //check for player presence
        HashMap::iterator iter = myPlayers.find(aPlayerId);

        if(iter == myPlayers.end())
        {
            return false;
        }
        else
        {
            //update player's availability
            iter->second.myIsAvailable = true;

            //add point
            myPartitioningTree.AddPoint(&iter->second.myPreferenceVector, aPlayerId);

            return true;
        }
    }

    bool
    MatchMaker::SetPlayerUnavailable(
        unsigned int    aPlayerId)
    {
        MutexLock lock(myLock); 

        /*
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
        */

        //check for player presence
        HashMap::iterator iter = myPlayers.find(aPlayerId);

        if(iter == myPlayers.end())
        {
            return false;
        }
        else
        {
            //update player's availability
            iter->second.myIsAvailable = false;

            //remove old point (we don't want to find offline users during MM)
            myPartitioningTree.RemovePoint(&iter->second.myPreferenceVector);

            return true;
        }
    }

    float 
    Dist(
        float    aA[20], 
        float    aB[20])
    {
        float dist = 0.0f; 
        for(int i = 0; i < 20; i++)
            dist += pow((aA[i] - aB[i]), 2.0f); 

        return sqrt(dist); 
    }

    class Matched
    {
    public:

        float            myDist; 
        unsigned int    myId; 
    };

    static int 
    MatchComp(
        Matched*    aA, 
        Matched*    aB)
    {
        if(aA->myDist < aB->myDist)
            return 1; 

        return 0; 
    }

    bool
    MatchMaker::MatchMake(
        unsigned int    aPlayerId, 
        unsigned int    aPlayerIds[20], 
        int&            aOutNumPlayerIds)
    {
        MutexLock lock(myLock); 

        //check for player presence
        HashMap::iterator iter = myPlayers.find(aPlayerId);

        if(iter == myPlayers.end())
        {
            return false;
        }


        std::pair<Point<PLAYER_PREFERENCES_NUM>*, unsigned int> usersFound[MATCHMAKE_PLAYERS_NUM];
        
        aOutNumPlayerIds = myPartitioningTree.FindNearestNeighbors(iter->second.myPreferenceVector, MATCHMAKE_PLAYERS_NUM, usersFound);

        for(int i = 0; i < aOutNumPlayerIds; ++i)
        {
            aPlayerIds[i] = usersFound[i].second;
        }

        return (aOutNumPlayerIds != 0);

        /*
        Player* playerToMatch = NULL; 
        for(unsigned int i = 0; i < myNumPlayers; i++)
        {
            if(myPlayers[i]->myPlayerId == aPlayerId)
            {
                playerToMatch                    = new Player();
                playerToMatch->myPlayerId        = myPlayers[i]->myPlayerId;
                playerToMatch->myIsAvailable    = myPlayers[i]->myIsAvailable; 
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
            matched[i]            = new Matched(); 
            matched[i]->myDist    = -1.0f; 
            matched[i]->myId    = -1; 
        }

        int matchCount = 0; 

        for(unsigned int i = 0; i < myNumPlayers; i++)
        {
            if(matchCount < 20)
            {
                matched[matchCount]->myId    = myPlayers[i]->myPlayerId; 
                matched[matchCount]->myDist    = Dist(myPlayers[i]->myPreferenceVector, playerToMatch->myPreferenceVector);
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
                matched[j]->myDist    = matched[j - 1]->myDist; 
                matched[j]->myId    = matched[j - 1]->myId; 
            }

            matched[index]->myDist    = dist;
            matched[index]->myId    = myPlayers[i]->myPlayerId; 

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
        */
    }
