// squad stuff
// again, base code from tf2ebots
// you should really go check tf2ebots out.....
// this is from the NewVersion, but by looking at the code it actually looked unfinished considering a lot of the stuff is unused anywhere ( like the squadGoal or squadThreat )
// considering its a cool concept that would actually help deepen the bot's decisionmaking skill and coordination vs actual teams, this attempts to make it work

/*
	goals for making squads:
	1: in a 100 player server, every client should be able to join at least one other squad
	2: it should be easy to find the exact squad a specific client is in, EVEN IF THEY ARE NOT THE SQUAD LEADER
	3: it should be easy to request other clients to join the requester's squad
	4: it should be trivial to join and leave that squad whenever the bot chooses to
*/

int cbot_Squad[TF_MAXPLAYERS][10];

stock void StartSquad(int client) // becomes squad leader 
{
	cbot_Squad[client][0] = client;
}

stock void EndSquad(int client)
{
	for (int i = 0; i < 6; i++)
	{
		LeaveCurrentSquad(cbot_Squad[client][i]);
	}
}

stock bool JoinToSquad(int client, int index) // joins other squad
{
	for (int i = 0; i < 6; i++)
	{
		if(cbot_Squad[index][i] == -1)
		{
			cbot_Squad[index][i] = client;
			cbot_Squad[client][0] = index; // lets other bots + the plugin know which other squad the client's in
			return true;
		}
	}
	return false;
}

stock void LeaveCurrentSquad(int client)
{
	int current = FindClientSquad(client);
	
	if( current == -1 )
		return;

	for (int i = 0; i < 6; i++)
	{
		if(cbot_Squad[current][i] == client)
		{
			cbot_Squad[current][i] = -1;
			cbot_Squad[client][0] = -1;
			break;
		}
	}
}

stock bool RequestToJoin(int asker, int receiver)
{
	int count = GetSquadCount(receiver);
	if (count > 6)
		return false;
	else
	{
		JoinToSquad(receiver, asker);
		return true;
	}
}

stock bool IsSquadLeader(int client)
{
	return cbot_Squad[client][0] == client;
}

stock bool GetSquadLeader(int index)
{
	return cbot_Squad[index][0];
}

stock int FindClientSquad(int client)
{
	if (cbot_Squad[client][0] == client)
		return client;
	else if (cbot_Squad[client][0] != -1 && IsValidClient(cbot_Squad[client][0]))
		return cbot_Squad[client][0];
	
	return -1;
}

stock int GetSquadCount(int index)
{
	int count = 0;
	for (int i = 0; i < 6; i++)
	{
		if (cbot_Squad[index][i] != -1)
			count++;
	}
	
	return count;
}

stock bool RequestClientJoinSquad(int client, int edict) // asks team for anyone willing to join the bot's squad
{
	if(!IsValidClient(edict))
		return false;
	
	if(FindClientSquad(edict) != -1) // already in a squad TODO: let people leave squads and join requested ones
		return false;
	
	RequestToJoin(client, edict);
}