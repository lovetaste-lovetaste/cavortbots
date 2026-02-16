// this initialize bot specific variables
// for general variable, look in the .sp file

enum Mentality {
	MENTAL_WIN = 0,						// play normally, prioritize objective and general success
	MENTAL_WIN_SAFELY,					// play passively, based off objective
	MENTAL_WIN_AGGRESIVE,			// play aggresively but still pay attention to objective
	MENTAL_DEATHMATCH,				// deathmatch, ignore objective; this makes the mentality NOT AFFECT DECISION MAKING, equivalent to not having one at all
	MENTAL_LOSER,							// do not die under any means necessary; permanently disadvantaged state
	MENTAL_BEAST, 							// kill under any means necessary; permanently advantaged state; goal is always GOAL_HUNT_FOR_KILLS or GOAL_KILL
	MENTAL_FRIENDLY,						// friendly, no killing
	
	// MENTAL_,			// comment
	/*
		Add any new mentalities above!
		I don't know why you would want to though....
		Too much of a hassle
	*/
	
	// DONT PUT ANYTHING UNDER THESE MENTALITIES; THESE ARE DEBUG
	MENTAL_AUTOASSIGN,				// randomly picks a Mentality 
	MENTAL_IDIOT,								// goals are completely random, debuug, doesnt give a shit, a stimulator of The Average European Engineer Player
	
	MENTAL_LAST
}

enum State {
	STATE_NORMAL = 0,						// normal
	STATE_DISADVANTAGE,					// low health, low ammo, alone vs many, at a disadvantage
	STATE_BIG_DISADVANTAGE,			// massive disadvantage, most likely going to die soon however hope is not lost
	STATE_FATAL_DISADVANTAGE,		// death is guarenteed, obscenely large disadvantage, cannot win
	STATE_FATAL_SACRIFICE,					// reserved for last stands and needed sacrifices ( round time abt to end, enemy is abt to win, trying to kill specific enemy that is so important to kill that your death is worth it )
	
	STATE_ADVANTAGE,							// high health, many vs one, at a advantage
	STATE_BIG_ADVANTAGE,					// massive advantage, however risk is still there ( dry kritzkrieg pushes with no support, can still get nuked )
	STATE_LARGE_ADVANTAGE,			// incredibly large advantage, most likely will win any 1v1s ( massive numbers advantage, massive class-based advantage, buffs  )
	STATE_AGGRO_ADVANTAGE,			// reserved for specific conditions where any risk is completely ignored  ( ubers and other stuff ). most likely tied with MENTAL_WIN_AGGRESIVE or MENTAL_BEAST
	
	// STATE_,			// comment
	/*
		Add any new states above!
		I don't know why you would want to though....
	*/
	STATE_LAST
}

enum Goal {
	GOAL_SURVIVE = 0,											// do not die
	GOAL_SURVIVE_IMPORTANT,							// do not die!!!! no risks!!! no confrontations/kills if its avoidable!!! just survive!!!
	GOAL_KILL,														// generic kill
	GOAL_KILL_SPECIFIC,										// priority based kills ( important people )
	GOAL_HUNT_FOR_KILLS,									// find targets to kill
	GOAL_HUNT_FOR_SPIES,									// spycheck
	GOAL_HUNT_FOR_FLANKERS,							// look for anyone behind the team
	GOAL_WIN,														// do objective
	GOAL_ASSIST,													// help others
	
	// GOAL_,							// comment
	/*
		Add any new goals above!
	*/
	GOAL_LAST
}

enum Actions {
	Action_Null = 0,									// this is here to prevent some bugs; dont mess with this
	Action_FightTarget,								// fight target; this is seperate from the shoot target due to not needing to fight
	Action_Special_Action,							// do special action ( ubers, banners, etc )
	Action_Retreat,										// get to a safe spot
	Action_Push,											// move into the enemy
	
	// Action_,			// comment
	/*
		Add any new actions above!
	*/
	Action_Last
}

enum struct BotData
{
	bool Enabled; 						// this allows the toggling of specific bots, for better control over them

	ArrayList validEnemies;		// all the enemies that the bot is aware of
	ArrayList validTeammates;	// all the teammates that the bot is aware of
	int Target;

	bool ban_primary;
	bool ban_secondary;
	bool ban_melee;
	// for weapon switching; specifically in case that any TFBots get enabled as a Cavortbot
	
	float lastCommand;
	// to prevent bots from spamming commands
	
	int WantedButtons;
	// makes the bot press buttons
	
	Mentality mentality;
	State state;
	Goal goal;
	ArrayList actions;
	// bot decision making things

	float LastTimeReacted;
	float BaseReactionTime;
	// reaction time things
}

BotData g_BotData[TF_MAXPLAYERS];

char cavortnames[9][] = {"cavort", "cavorter", "cavortmaxxing", "CavortCavort", "Wow,isThatCavort???", "cavortest cavort", "beep boop", "cavortrovac", "CcAaVvOoRrTt"};