// this is where all the bonus/custom gamemode support goes in
// make sure to put allat stuff here to make the code easier

/*

	CURRENTLY SUPPORTED PLUGINS:
	MGE ( kinda )
	maybe more in the future...
*/

#include <cavortbots/mge>

void CheckPlugins(int client)
{
	if(!IsValidClient(client))
		return;
		
	// on mge map??
	GetCurrentMap(currentMap, sizeof(currentMap));
	if( StrContains(currentMap, "mge_", false) != -1 )
	{
		CheckMGEArena(client);
	}
}