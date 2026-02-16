void Bot_Initialize(int client)
{
	g_BotData[client].mentality = MENTAL_AUTOASSIGN;
	g_BotData[client].BaseReactionTime = GetRandomFloat(0.175, 0.25);
	g_BotData[client].BaseReactionTime += ((GetRandomInt(0,1) == 0) ? 0.0152 : 0.0303);
	g_BotData[client].LastTimeReacted = GetGameTime();
}

void Bot_Brain(int client, int decisionThink=0)
{
	if(!IsValidClient(client))
		return;

	// this controls all the bot functions and the goal thinking
	// an important improvement over lovelybots ( last gen of my bots ) is the inclusion of decisionThink
	// since certain functions are required to run every cmd for the bots to work, while some others only need to be ran once in a while, this allows all the functions to be combined
	// this also helps with fps due to the massive functions happening less often
	
	// no decisionThink triggers the constant functions
	// 1 decisionThink triggers the functions that happen every bot update
	// 2 decisionThink triggers both the functions that happen every bot update AS WELL AS functions that don't happen as much
	// if ur gonna add more layers for this, then make sure to go to the sp file and add support for it in the OnPlayerRunCmd as well as below
	
	
	if(g_BotData[client].validEnemies == null)
		g_BotData[client].validEnemies = new ArrayList();
	if(g_BotData[client].validTeammates == null)
		g_BotData[client].validTeammates = new ArrayList();
	if(g_BotData[client].actions == null)
		g_BotData[client].actions = new ArrayList();

	if( decisionThink == 2 )
	{
		// run rare funcs here
		
		decisionThink = 1; // for the every bot update functions to occur in the switch below
	}
	
	if( decisionThink == 1 )
	{	
		// run every update funcs here
		g_BotData[client].LastTimeReacted = GetGameTime(); // KEEP THIS HERE OR ELSE THINGS BREAK
		CheckPlugins(client);
		Bot_FindTargets(client);
		Bot_Decisionmaking(client);
	}
	
	// run constant functions here
	if(IsPlayerAlive(client))
	{
		Bot_Actions(client);
	
		Bot_Aiming(client);
		
		Bot_Movement(client);
	}
	// if dead then do not do anything
	// might interfere with other stuff
}

void Bot_Movement(int client)
{
	if(!IsValidClient(client))
		return;
		
	if(GetClientDistanceToCeiling(client) < 107 || isClientAirborne(client))
	{
		g_BotData[client].WantedButtons |= IN_DUCK;
	}
	
	int fwdmove, sidemove;
	int target = g_BotData[client].Target; // just to make code cleaner
	
	if(ClientCanSeeClient(client, target, _, true))
	{
		TFClassType class = TF2_GetPlayerClass(client);
		TFClassType targetClass = TF2_GetPlayerClass(g_BotData[client].Target);

		float optimalDistance = 512.0; // this is the range for base dmg for most weapons
		float hpFactor = float( GetHealth(client) ) / float( GetHealth(g_BotData[client].Target) );

		if(targetClass == TFClass_Heavy)
		{
			bool revved = TF2_IsPlayerInCondition(g_BotData[client].Target, TFCond_Slowed);
			if( ( !revved && StrContains(TF2_GetWeaponClassName(GetEntPropEnt(g_BotData[client].Target, Prop_Send, "m_hActiveWeapon")), "tf_weapon_minigun", false) != -1 ) || ( revved && GetNextFiringTime(g_BotData[client].Target) >= g_BotData[client].BaseReactionTime ) || angleToFaceCoords(g_BotData[client].Target, GetOrigin(client), true, true) <= 90.0 )
				optimalDistance *= 0.75;	// if heavy has the minigun and is not revved OR the heavy is just starting to rev up OR the target is not facing the client, then the bot has more leeway against them
		}

		switch( g_BotData[client].state )
		{
			case STATE_BIG_ADVANTAGE, STATE_LARGE_ADVANTAGE, STATE_AGGRO_ADVANTAGE, STATE_FATAL_SACRIFICE:
				optimalDistance *= hpFactor;
			case STATE_BIG_DISADVANTAGE, STATE_FATAL_DISADVANTAGE:
				optimalDistance /= hpFactor;
		}

		if(StrContains(TF2_GetWeaponClassName(GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon")), "tf_weapon_rocketlauncher", false) != -1 || StrContains(TF2_GetWeaponClassName(GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon")), "tf_weapon_grenadelauncher", false) != -1 || StrContains(TF2_GetWeaponClassName(GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon")), "tf_weapon_cannon", false) != -1)
		{
			// if the bot is holding a rocket launcher, grenade launcher, or the loose cannon, the bot will try to stay more than 170 units away from its target
			// iirc max splash distance is 140-169 units, so this ensures bots will at least try to not self-destruct
			if(optimalDistance < 170.0)
				optimalDistance = 170.0;
		}

		if(class == TFClass_Sniper)
		{
			if(StrContains(TF2_GetWeaponClassName(GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon")), "TF_WEAPON_SNIPERRIFLE", false) != -1)
				optimalDistance = 2048.0; // max distance for other people's damage falloff times 2
			// if holding a sniper rifle, then the bot wants to stay as far away as possible
		}

		if(IsWeaponSlotActive(client, TFWeaponSlot_Melee) && class != TFClass_Spy)
			optimalDistance = 10.0;
		// non-spies will want to get up close and personal

		if(class == TFClass_Spy && isSpyInvisible(client))
			optimalDistance = 2048.0;

		if(optimalDistance < 100.0)
			optimalDistance = 100.0;
		// personal space bubble

		optimalDistance *= GetRandomFloat(0.5, 1.5);
	
		switch( GetFastDistance(GetOrigin(client), GetOrigin(target)) < optimalDistance )
		{
			case true:
			{
				sidemove = GetRandomInt(0,1) == (client % 2) ? -1 : 1;
			}
			case false:
			{
				fwdmove = 1;
			}
		}
	}
	else
	{
		// import navigation shit here brah!!!!
	}
	
	// rocket surfing
	int nearRocket = GetNearestRocket(client, 286.0, g_BotData[client].Target);
	if(nearRocket != -1)
	{
		float rocketOrigin[3];
		GetEntPropVector(nearRocket, Prop_Send, "m_vecOrigin", rocketOrigin);
		
		if(GetVectorDistance(rocketOrigin, GetOrigin(client)) <= 286.0)
		{
			if(!isClientAirborne(client))
			{
				g_BotData[client].WantedButtons |= IN_JUMP;
			}
				
			int dmg = PredictRocketDamage(client, nearRocket, 90, GetOwnerEntity(nearRocket));
			
			if(GetClientHealth(client) - dmg <= 0) // rocket is predicted to kill bot, in grave danger!!!!!!
			{
				switch(TF2_GetPlayerClass(client))
				{
					case TFClass_Medic:
					{
						if(isUberAbovePercent(client))
							g_BotData[client].WantedButtons |= IN_ATTACK2;
						// if medic then ubers in preparation for rocket, depending on which medigun they have it may save them
					}
					case TFClass_Spy:
					{
						if( GetItemIndexOfSlot(client, 4) == 59 && GetEntProp(client, Prop_Send, "m_bFeignDeathReady") == 0 )
							g_BotData[client].WantedButtons |= IN_ATTACK2;
						// if spy then tries to dead ringer if possible
					}
				}
			}
		}
	}
	
	Bot_MoveDirection(client, fwdmove, sidemove);
}

void Bot_Aiming(int client)
{
	if( g_BotData[client].actions.FindValue(Action_FightTarget) != -1 )
	{
		int enemy = g_BotData[client].Target;
	
		if(IsValidClient(enemy))
		{
			int activeWeapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
			char weaponName[128];
			weaponName = TF2_GetWeaponClassName(activeWeapon);
			// the reason it uses the classname of the weapon rather than the weapon slot is due to randomizer sometimes having weapons in different slots/classes then they usually are
			// ex: scouts using demoman weapons / double shotguns / etc
			int index = ((IsValidEntity(activeWeapon)) ? GetEntProp(activeWeapon, Prop_Send, "m_iItemDefinitionIndex") : -1);
		
			float edictOrigin[3];
		
			bool aimForHead = false;
			if( StrContains(weaponName, "tf_weapon_sniperrifle", false) != -1 || StrContains(weaponName, "tf_weapon_compound_bow", false) != -1 || index == 61 || index == 1006 )
				aimForHead = true;	
			
			Bot_AimAtTarget(client, enemy, aimForHead, edictOrigin);
			TF2_LookAtPos(client, edictOrigin, 0.25);
			
			bool playAggresive = false;
			if( g_BotData[client].actions.FindValue(Action_Push) != -1 )
				playAggresive = true;
			switch( g_BotData[client].state )
			{
				case STATE_BIG_ADVANTAGE, STATE_LARGE_ADVANTAGE, STATE_AGGRO_ADVANTAGE, STATE_FATAL_SACRIFICE:
					playAggresive = true;
				case STATE_BIG_DISADVANTAGE, STATE_FATAL_DISADVANTAGE: // big disad, better stick to close-ish targets
					playAggresive = false;
			}
			switch( g_BotData[client].mentality )
			{
				case MENTAL_BEAST: // KILL EVERYONE
					playAggresive = true;
				case MENTAL_WIN_SAFELY:
					playAggresive = false;
			}
			
			playAggresive = true;
			if( playAggresive || GetFastDistance(GetOrigin(client), GetOrigin(enemy)) < 1024.0 || TF2_IsPlayerBuffedDangerous(client) || TF2_GetPlayerClass(client) == TFClass_Sniper ) // at this point its useless to shoot someone, unless u have crits OR are a sniper
			{
				Bot_Fire(client);
			}
		}
	}
}

void Bot_Decisionmaking(int client)
{
	if(cavort_settings_debug_enable_text.IntValue == 1)
		PrintToServer("%N is making decisions...", client);
	
	// this is for deciding on which goals to pursue
	// check initialize.inc for list of goals
	
	if(g_BotData[client].mentality == MENTAL_AUTOASSIGN)
	{
		GetCurrentMap(currentMap, sizeof(currentMap));
		
		int rand = GetRandomInt(0, view_as<int>(MENTAL_LAST) -3);
		switch( (StrContains(currentMap, "dm_", false) != -1 || StrContains(currentMap, "duel_", false) != -1 || StrContains(currentMap, "mge_", false) != -1 || StrContains(currentMap, "trade_", false) != -1) ? -1 : rand )
		{
			case -1:
			{
				g_BotData[client].mentality = MENTAL_DEATHMATCH;
			}
			default:
			{
				g_BotData[client].mentality = view_as<Mentality>(rand);
			}
		}
	}
	
	bool areThereEnemies = g_BotData[client].validEnemies.Length > 0;
	bool areThereTeammates = g_BotData[client].validTeammates.Length > 0;
	
	Bot_State(client);
	
	if( g_BotData[client].state == STATE_FATAL_DISADVANTAGE )
	{
		if(cavort_settings_debug_enable_text.IntValue == 1)
			PrintToServer("%N thinks it's about to die!", client);
		
		// IT'S FUCKING OVER!!!!!
		// TOO LATE TO SURVIVE!!!
		// GO OUT LIKE A G!!!!
		if( g_BotData[client].mentality == MENTAL_LOSER )
		{
			g_BotData[client].goal = GOAL_SURVIVE_IMPORTANT;
			
			if(cavort_settings_debug_enable_text.IntValue == 1)
				PrintToServer("%N is going to try to run away!", client);
		}
		else if(GetClientHealth(client) <= 25)
		{
			FakeClientCommandThrottled(client, "taunt");
			
			if(cavort_settings_debug_enable_text.IntValue == 1)
				PrintToServer("%N decides to make fun of the enemy before death!", client);
			// FINAL FUCK YOU TO ENEMY
		}
		else
		{
			g_BotData[client].goal = GOAL_KILL;
			
			if(cavort_settings_debug_enable_text.IntValue == 1)
				PrintToServer("%N isn't going out without a fight!", client);
			// KEEP TRYING TO TAKE AS MUCH PEOPLE OUT AS POSSIBLE
		}
			
	}
	
	if( g_BotData[client].goal == GOAL_SURVIVE_IMPORTANT )
	{
		bool goodEnough = false;
		
		if( GetClientHealth(client) >= (GetMaxPlayerHealth(client) * 0.75) || (!areThereEnemies && GetClientHealth(client) >= (GetMaxPlayerHealth(client) * 0.5)) )
		{
			goodEnough = true;
		}
		
		if(goodEnough)
		{
			g_BotData[client].goal = GOAL_SURVIVE; // its fine, no longer need to panic
				
			if(cavort_settings_debug_enable_text.IntValue == 1)
				PrintToServer("%N doesn't need to worry as much, and calms down from it's panicked state.", client);
		}
		else
		{
			if(cavort_settings_debug_enable_text.IntValue == 1)
				PrintToServer("%N is trying to survive as hard as it can!", client);
			return; // bot doesnt care abt anything else if all its doing is trying to retreat ASAP
		}
	}
	
	TFClassType class = TF2_GetPlayerClass(client);
	
	if(areThereEnemies)
	{
		if( g_BotData[client].goal == GOAL_KILL_SPECIFIC ) // implies that the bot is already trying to kill someone in particular
		{
			if(cavort_settings_debug_enable_text.IntValue == 1)
				PrintToServer("%N is trying to kill a specific person.", client);
				
			if( !(IsValidClient(g_BotData[client].Target) && IsPlayerAlive(g_BotData[client].Target) && g_BotData[client].validEnemies.FindValue(g_BotData[client].Target) != -1) )
			{
				// if target is no longer a valid enemy to focus, then reset target and goal
				g_BotData[client].Target = -1;
				g_BotData[client].goal = GOAL_KILL;
				
				if(cavort_settings_debug_enable_text.IntValue == 1)
					PrintToServer("%N can't target that person anymore, it wants to find someone else.", client);
			}
			else
				return; // since the bot is targetting a specific client already, then the rest of the checks doesnt matter
		}
		
		if( g_BotData[client].goal != GOAL_KILL_SPECIFIC ) // this is specifically after the goal specific check so if the target is invalid ( see above ) it still runs this stuff
		{
			int enemy = Bot_GetEnemy(client);
			g_BotData[client].Target = enemy; // if the bot enemy returns an invalid set, then it won't bother to kill em
			
			if(IsValidClient(enemy))
			{
				g_BotData[client].goal = GOAL_KILL;
				
				if(cavort_settings_debug_enable_text.IntValue == 1)
					PrintToServer("%N wants to kill %N", client, enemy);
				
				TFClassType enemyClass = TF2_GetPlayerClass(enemy);
				
				if(enemyClass == TFClass_Medic || ( class == TFClass_Sniper && enemyClass == TFClass_Sniper ) )
				{
					if(cavort_settings_debug_enable_text.IntValue == 1)
						PrintToServer("%N REALLY wants to kill %N...", client, enemy);
				
					g_BotData[client].goal = GOAL_KILL_SPECIFIC;
					// focus medics!
					// if sniper, focus other snipers
					// if enemy weak, focus ( kinda cheats, will remove )
				}
			}
			else
			{
				if(cavort_settings_debug_enable_text.IntValue == 1)
					PrintToServer("%N found no enemy to kill!", client);
			}
		}
	}
	
	if(areThereTeammates)
	{
		bool assist = false;
		int teammate = Bot_GetTeammate(client);
		
		if(IsValidClient(teammate))
		{
			if(class == TFClass_Medic)
			{
				// if there is a possible teammate to help out, then always do it
				// theres no real reason not to
				// in the case that it would be better to fight a closer enemy than it would be to heal a far away enemy, the action will handle getting the medic away from the enemy while trying to survive
				// Medic has special Action logic for assists anyway
				assist = true;
			}
			
			if(!areThereEnemies)
			{
				switch(class)
				{
					case TFClass_Pyro:
					{
						if(TF2_IsPlayerInCondition(teammate, TFCond_OnFire))
							assist = true;
						
					}
				}
				// add more shit here eventually
			}
			
			if(assist)
			{
				g_BotData[client].goal = GOAL_ASSIST;
				g_BotData[client].Target = teammate;
			}
		}
	}
}

void Bot_Actions(int client)
{
	// this is for deciding which actions to do according to the goal
	// check initialize.inc for list of actions
	
	g_BotData[client].actions.Clear();
	
	if( g_BotData[client].goal == GOAL_KILL || g_BotData[client].goal == GOAL_KILL_SPECIFIC )
	{
		if(Bot_IsTargetStillValid(client, g_BotData[client].Target))
		{
			g_BotData[client].actions.Push(Action_FightTarget);
		}
		else
		{
			g_BotData[client].Target = -1;
			g_BotData[client].goal = GOAL_LAST;
				
			if(cavort_settings_debug_enable_text.IntValue == 1)
				PrintToServer("%N's target is dead/invalid! No point in shooting, so goal is cleared.", client);
		}
			
	}
}