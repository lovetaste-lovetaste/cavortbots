/** Double-include prevention */
#if defined _GiveBotsLoadoutAttributes_included_
  #endinput
#endif
#define _GiveBotsLoadoutAttributes_included_
 
/**
 * Changes the weapon slot of a client with a new index.
 * 
 * @param client	The client who is being changed.
 * @param class		The class that is being changed.
 * @param slot		The weapon slot that is being changed.
 * @param newIndex		The new index.
 */
native void ChangeClientSlotIndex(int client, int class, int slot, int newIndex)

/**
 * Refreshes the client's weapons. Does not include the cosmetics.
 *
 * @param client	The client who is being changed.
 */
native void RefreshClientWeaponLoadout(int client)

public SharedPlugin __pl_GiveBotsLoadoutAttributes = 
{
	name = "GiveBotsLoadoutAttributes",
	file = "GiveBotsLoadoutAttributes.smx",
// #if defined REQUIRE_PLUGIN
	// required = 1,
// #else
	// required = 0,
// #endif
};