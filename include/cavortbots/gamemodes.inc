// for normal gamemodes
// special ones go in bonus_gamemodes.inc

// all this logic is stolen from tf2ebots anyway
// sorry

bool isCTF;
bool isPL;
bool isPLR;
bool isCP;
bool isAD;
bool isKOTH;
bool isArena;
bool isMVM;
bool isDM;
bool isVSH;

stock void CheckGamemodes()
{
	GetCurrentMap(currentMap, sizeof(currentMap));

	isCTF = false;
	isPL = false;
	isPLR = false;
	isCP = false;
	isAD = false;
	isKOTH = false;
	isArena = false;
	isMVM = false;
	isDM = false;
	isVSH = false;

	if (GetConVarInt(FindConVar("tf_gamemode_mvm")) == 1)
		isMVM = true;
	else if (GetConVarInt(FindConVar("tf_gamemode_arena")) == 1)
	{
		if (StrContains(currentMap, "vsh_" , false) != -1)
			isVSH = true;
		else
		{
			isArena = true;
			isKOTH = true;
		}
	}
	else if (GetConVarInt(FindConVar("tf_gamemode_ctf")) == 1)
		isCTF = true;
	else if (GetConVarInt(FindConVar("tf_gamemode_payload")) == 1)
	{
		if (StrContains(currentMap, "plr_" , false) != -1)
			isPLR = true;
		else
			isPL = true;
	}
	else if (GetConVarInt(FindConVar("tf_gamemode_cp")) == 1 || GetConVarInt(FindConVar("tf_gamemode_tc")) == 1)
	{
		if (StrContains(currentMap, "koth_" , false) != -1)
			isKOTH = true;
		else if (GetBluControlPointCount() > 0)
			isCP = true;
		else
			isAD = true;
	}
	else
		isDM = true;
}

stock bool IsOnDefense(const int client)
{
	if(!IsValidClient(client))
		return false;

	if (GetClientTeam(client) == 2)
	{
		if (isPL)
			return true;

		if (isMVM)
			return true;

		if (isAD)
			return true;
	}

	return false;
}

stock bool IsOnOffense(const int client)
{
	if(!IsValidClient(client))
		return false;
	
	if (GetClientTeam(client) == 3)
	{
		if (isPL)
			return true;

		if (isMVM)
			return true;

		if (isAD)
			return true;
	}

	return false;
}
