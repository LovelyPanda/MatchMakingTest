#include "stdafx.h"

#include "MatchMaker.h"

#include <algorithm>
//#include <math.h>

#include <mmintrin.h>
#include <cassert>

    MatchMaker* MatchMaker::ourInstance = 0;

    MatchMaker::MatchMaker()
        : myOnlinePlayersNum(0),
          myPlayerMap(1000000) ///!!???
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

        PlayerHashMap::iterator iter = myPlayerMap.find(aPlayerId);

        if(iter == myPlayerMap.end())
        {
            if(myPlayerMap.size() >= MAX_NUM_PLAYERS)
                return false;

            //add new
            PlayerData newPlayer;
            newPlayer.myIsAvailable = false;
            newPlayer.myOnlinePosition = -1;
            memcpy(newPlayer.myPreferences, aPreferenceVector, sizeof(newPlayer.myPreferences));

            myPlayerMap[aPlayerId] = newPlayer;

            return true;
        }

        //update existing player
        PlayerData& player = iter->second;
        // 1) update hashmap
        memcpy(player.myPreferences, aPreferenceVector, sizeof(player.myPreferences));

        // 2) for user which is 'online' we need to update some more data
        if(player.myIsAvailable)
        {
            memcpy(myOnlinePlayersPreferences[player.myOnlinePosition].data, aPreferenceVector, sizeof(player.myPreferences));
        }

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

        return true;
    }

    bool
    MatchMaker::SetPlayerAvailable(
        unsigned int    aPlayerId)
    {
        MutexLock lock(myLock); 

        //find player
        PlayerHashMap::iterator iter = myPlayerMap.find(aPlayerId);

        if(iter == myPlayerMap.end())
        {
            return false; //no such player
        }

        PlayerData& player = iter->second;

        //already marked as on-line
        if(player.myIsAvailable) 
            return true;

        //add player preferences to array of on-line players
        unsigned int newOnlinePosition = myOnlinePlayersNum;

        if(myOnlinePlayersNum > MAX_ONLINE_PLAYERS)
            return false;

        //copy preferences
        memcpy(myOnlinePlayersPreferences[newOnlinePosition].data, player.myPreferences, sizeof(player.myPreferences));

        ++myOnlinePlayersNum;
        player.myOnlinePosition = newOnlinePosition;

        myOnlinePlayersPrefencesToIdsMap[newOnlinePosition] = aPlayerId;

        player.myIsAvailable = true;

        return true;

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

      return true;
    }

    bool
    MatchMaker::SetPlayerUnavailable(
        unsigned int    aPlayerId)
    {
        MutexLock lock(myLock); 

        //find player
        PlayerHashMap::iterator iter = myPlayerMap.find(aPlayerId);

        if(iter == myPlayerMap.end())
        {
            return false; //no such player
        }

        PlayerData& player = iter->second;

        //already marked as off-line
        if(!player.myIsAvailable)
            return true;

        //remove player preferences from array of on-line players
        unsigned int onlinePosition = player.myOnlinePosition;

        //swap the last one with the one we are going to remove
        if(onlinePosition < myOnlinePlayersNum - 1)
        {
            Vector20f& lastPreferenceVector = myOnlinePlayersPreferences[myOnlinePlayersNum - 1];
            memcpy(myOnlinePlayersPreferences[onlinePosition].data, lastPreferenceVector.data, sizeof(lastPreferenceVector.data));

            PlayerId lastPlayerId = myOnlinePlayersPrefencesToIdsMap[myOnlinePlayersNum - 1];
            PlayerData& lastPlayer = myPlayerMap[lastPlayerId];
            lastPlayer.myOnlinePosition = onlinePosition;

            myOnlinePlayersPrefencesToIdsMap[myOnlinePlayersNum - 1] = -1;
            myOnlinePlayersPrefencesToIdsMap[onlinePosition] = lastPlayerId;
        }

        --myOnlinePlayersNum;

        player.myOnlinePosition = -1;
        player.myIsAvailable = false;

        return true;

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
    }

    float 
    Dist(
        float    aA[20], 
        float    aB[20])
    {
        float dist = 0.0f; 
        for(int i = 0; i < 20; i++)
            dist += (aA[i] - aB[i]) * (aA[i] - aB[i]); 
        return dist;
        //return sqrt(dist); 
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

    static inline float SquareDistance(__m128 vec1[5], __m128 vec2[5])
    {
        __m128 tempVec[5];

        //first subtract
        for(int i = 0; i < 5; ++i)
        {
            tempVec[i] = _mm_sub_ps(vec1[i], vec2[i]);
        }

        //squared
        for(int i = 0; i < 5; ++i)
        {
            tempVec[i] = _mm_mul_ps(tempVec[i], tempVec[i]);
        }

        //sum
        tempVec[0] = _mm_add_ps(tempVec[0], tempVec[1]);
        tempVec[1] = _mm_add_ps(tempVec[3], tempVec[4]);

        //3 m128s left
        tempVec[0] = _mm_add_ps(tempVec[0], tempVec[1]);
        
        //2 m128s left
        tempVec[0] = _mm_add_ps(tempVec[0], tempVec[2]);


        //1 left, just sum
        return tempVec[0].m128_f32[0] + tempVec[0].m128_f32[1] +
               tempVec[0].m128_f32[2] + tempVec[0].m128_f32[3];
    }

    bool
    MatchMaker::MatchMake(
        unsigned int    aPlayerId, 
        unsigned int    aPlayerIds[20], 
        int&            aOutNumPlayerIds)
    {
        MutexLock lock(myLock); 

        aOutNumPlayerIds = 0;

        //find player
        PlayerHashMap::iterator iter = myPlayerMap.find(aPlayerId);

        if(iter == myPlayerMap.end())
        {
            return false; //no such player
        }

        //if we don't have enough on-line players, just return available
        if(myOnlinePlayersNum <= 21)
        {
            if(myOnlinePlayersNum == 1)
                return false;

            for(unsigned int i = 0, j = 0; i < myOnlinePlayersNum; ++i)
            {
                if(myOnlinePlayersPrefencesToIdsMap[i] != aPlayerId)
                {
                    aPlayerIds[j++] = myOnlinePlayersPrefencesToIdsMap[i];
                }
            }

            aOutNumPlayerIds =  myOnlinePlayersNum - 1;
            return true;
        }

        PlayerData& player = iter->second;

        Vector20f vector;
        memcpy(vector.data, player.myPreferences, sizeof(player.myPreferences));

        //among the other points there will be ourselves that's why we have 21 entries
        float shortestDistances[21] = { 100000.0f };
        float maxDistance = 1000000.0f;
        unsigned int maxDistancePosition = 0;
        unsigned int closestPlayersIndexes[21] = { 0 };

        for(unsigned int i = 0; i < myOnlinePlayersNum; ++i)
        {
            float dist = SquareDistance(vector.data, myOnlinePlayersPreferences[i].data);
            
            if(dist < maxDistance)
            {
                shortestDistances[maxDistancePosition] = dist;
                closestPlayersIndexes[maxDistancePosition] = i;

                //find new max distance
                float* maxElementPtr = std::max_element(shortestDistances, shortestDistances + 20);
                maxDistance = *maxElementPtr;
                maxDistancePosition = maxElementPtr - shortestDistances;
            }

            //float diff = dist - Dist(vector.data->m128_f32, myOnlinePlayersPreferences[i].data->m128_f32);
            //assert(diff < 0.001f && diff > -0.001f);

        }

        //exclude ourselves and save result
        for(int i = 0, j = 0; i < 21; ++i)
        {
            if(myOnlinePlayersPrefencesToIdsMap[closestPlayersIndexes[i]] != aPlayerId)
            {
                aPlayerIds[j] = myOnlinePlayersPrefencesToIdsMap[closestPlayersIndexes[i]];

                ++j;
            }
        }

        return true;

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
