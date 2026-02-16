// lite mode, for performance
// activated when it is a singleplayer game and the bot is out of view from the client
// or when the debug command is active

/*
	GOALS OF LITE MODE:
	1. Allow bots to use navigation
	2. Maintain decent fps compared to normal
	3. Keep as much things as possible while not adding un-needed stuff ( stuff that looks cool but isnt needed for the bots )
	4. Keep things as interchangable as possible so the bots can switch between modes easily with no restrictions
*/

void Bot_Brain_Lite( int client )
{
	if(!IsValidClient(client) || !IsPlayerAlive(client))
		return;
		
	//int enemy = FindNearestTarget(client);
	
	//if(IsValidClient(enemy))
		//Bot_AimAtTarget
		//Bot_Fire
	
	
}
// https://people.mschfmag.com/