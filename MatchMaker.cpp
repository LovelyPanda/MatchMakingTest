#include "stdafx.h"

#include "MatchMaker.h"

#include <algorithm>
#include <cfloat>
#include <mmintrin.h>

#define MATCHMAKE_PLAYERS_NUM 20

    MatchMaker* MatchMaker::ourInstance = 0;

    MatchMaker::MatchMaker()
        : myOnlinePlayersNum(0),
          myPlayerMap(MAX_NUM_PLAYERS) // I can't think about any other (better) number of buckets
    {
		myReadersNum = myWritersNum = 0;
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
		WriterCondition(&myConditionData, &myReadersNum, &myWritersNum);

        PlayerHashMap::iterator iter = myPlayerMap.find(aPlayerId);

        if(iter == myPlayerMap.end())
        {
            if(myPlayerMap.size() >= MAX_NUM_PLAYERS)
			{
                return false;
			}

            //add new
            PlayerData newPlayer;
            newPlayer.myIsAvailable = false;
            newPlayer.myOnlinePosition = -1;
            memcpy(newPlayer.myPreferences, aPreferenceVector, sizeof(newPlayer.myPreferences));

            myPlayerMap[aPlayerId] = newPlayer;

			if(myPlayerMap.size() % 100 == 0)
				printf("num players in system %u\n", myPlayerMap.size()); 

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

        return true;
    }

    bool
    MatchMaker::SetPlayerAvailable(
        unsigned int    aPlayerId)
    {
		WriterCondition(&myConditionData, &myReadersNum, &myWritersNum);

        //find player
        PlayerHashMap::iterator iter = myPlayerMap.find(aPlayerId);

        if(iter == myPlayerMap.end())
        {
            return false; //no such player
        }

        PlayerData& player = iter->second;

        //already marked as on-line
        if(player.myIsAvailable)
		{
            return true;
		}

        //add player preferences to array of on-line players
        unsigned int newOnlinePosition = myOnlinePlayersNum;

        if(myOnlinePlayersNum >= MAX_ONLINE_PLAYERS)
		{
            return false;
		}

        //copy preferences
        memcpy(myOnlinePlayersPreferences[newOnlinePosition].data, player.myPreferences, sizeof(player.myPreferences));

        ++myOnlinePlayersNum;
        player.myOnlinePosition = newOnlinePosition;

        myOnlinePlayersPrefencesToIdsMap[newOnlinePosition] = aPlayerId;

        player.myIsAvailable = true;

        return true;
    }

    bool
    MatchMaker::SetPlayerUnavailable(
        unsigned int    aPlayerId)
    {
		WriterCondition(&myConditionData, &myReadersNum, &myWritersNum);

        //find player
        PlayerHashMap::iterator iter = myPlayerMap.find(aPlayerId);

        if(iter == myPlayerMap.end())
        {
            return false; //no such player
        }

        PlayerData& player = iter->second;

        //already marked as off-line
        if(!player.myIsAvailable)
		{
            return true;
		}

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
    }

	//SSE-accelerated function to compute square distance between two 20-D points
	//contains Microsoft-specific code
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

	struct Neighbor
	{
		bool operator<(const Neighbor& rhv) const
		{
			return (distance < rhv.distance);
		}

		float distance;
		unsigned int arrayIndex;
	};

    bool
    MatchMaker::MatchMake(
        unsigned int    aPlayerId, 
        unsigned int    aPlayerIds[20], 
        int&            aOutNumPlayerIds)
    {
	    ReaderCondition(&myConditionData, &myReadersNum, &myWritersNum);

        aOutNumPlayerIds = 0;

        //find player
        PlayerHashMap::iterator iter = myPlayerMap.find(aPlayerId);

        if(iter == myPlayerMap.end())
        {
            return false; //no such player
        }

		//the number of on-line users except for our user
        unsigned int otherOnlineUsersNum = (iter->second.myIsAvailable) ? myOnlinePlayersNum - 1 : myOnlinePlayersNum;

	    if(otherOnlineUsersNum == 0)
		{
			return false;
		}

        //if we don't have enough on-line players, just return available, no need to calculate anything
        if(otherOnlineUsersNum <= MATCHMAKE_PLAYERS_NUM)
        {
			//exclude our player and copy result
            for(unsigned int i = 0, j = 0; i < myOnlinePlayersNum; ++i)
            {
                if(myOnlinePlayersPrefencesToIdsMap[i] != aPlayerId)
                {
                    aPlayerIds[j++] = myOnlinePlayersPrefencesToIdsMap[i];
                }
            }

            aOutNumPlayerIds = otherOnlineUsersNum;

            return true;
        }

        PlayerData& player = iter->second;

        Vector20f vector;
        memcpy(vector.data, player.myPreferences, sizeof(player.myPreferences));

        //to decrease the number of branches, I am going to search for 21 nearest neighbors
		//one of them would be our player, we will exclude him later

		Neighbor neighbors[MATCHMAKE_PLAYERS_NUM + 1];
		        
		//fill array with large distances
		for(unsigned int i = 0; i < MATCHMAKE_PLAYERS_NUM + 1; ++i)
        {
            neighbors[i].distance = FLT_MAX;
        }

        float maxDistance = FLT_MAX;

        for(unsigned int i = 0; i < myOnlinePlayersNum; ++i)
        {
            float dist = SquareDistance(vector.data, myOnlinePlayersPreferences[i].data);
            
            if(dist < maxDistance)
            {
				//last one is the worst neighbor, owerwrite him
                neighbors[MATCHMAKE_PLAYERS_NUM].distance = dist;
				neighbors[MATCHMAKE_PLAYERS_NUM].arrayIndex = i;

				//resort the array
				std::sort(neighbors, neighbors + MATCHMAKE_PLAYERS_NUM + 1);
            }
        }

        //exclude ourselves and save result
        for(int i = 0, j = 0; i < MATCHMAKE_PLAYERS_NUM + 1; ++i)
        {
			PlayerId neighborId = myOnlinePlayersPrefencesToIdsMap[neighbors[i].arrayIndex];

            //skip ourselves
			if(neighborId != aPlayerId)
            {
                aPlayerIds[j] = neighborId;
                ++j;
            }

			//we don't need the 21-st one in case our player is off-line
			if(!iter->second.myIsAvailable && i == MATCHMAKE_PLAYERS_NUM - 1)
            {
                break;
            }
        }

        aOutNumPlayerIds = MATCHMAKE_PLAYERS_NUM;

        return true;
    }
