/** Double-include prevention */
#if defined _lovelybots_included_
  #endinput
#endif
#define _lovelybots_included_
 
/**
 * Returns the bots difficulty.
 *
 * @param client	The client who is being checked.
 * @param basedOnClass		Whether or not it grabs the class difficulty or the base difficulty.
 * @return        The client's difficulty.
 */
native int GetBotDifficulty(int client, bool basedOnClass)

/**
 * Returns the bot's most skilled class.
 *
 * @param client	The client who is being checked.
 * @return        The client's most skilled class.
 */
native int GetBotMostSkilledClass(int client)


public SharedPlugin __pl_lovelybots = 
{
	name = "lovelybots",
	file = "lovelybots.smx",
// #if defined REQUIRE_PLUGIN
	// required = 1,
// #else
	// required = 0,
// #endif
};