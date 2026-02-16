// primarily funcs that pertain to specifically bots, but dont necessarily get changed often enough to have them in base.inc

void Bot_FindTargets(int client)
{
	// this is different from usual bot target search functions in the last gens of bots; this directly edicts the bot targets rather than just returning one
	// previously, since this was usually triggered twice to find both enemies and teammates, that would also mess with the awareness calcs due to it being ran twice!
	// this doesnt have that flaw due to only needing to run once per cycle to find both enemies and teammates!
	
	g_BotData[client].validEnemies.Clear();
	g_BotData[client].validTeammates.Clear();
	
	// this looks for ALL good targets and puts them into the ArrayLists
	for(int edict = 1; edict < MAXPLAYERS + 1; edict++)
	{
		if(!IsValidClient(edict))
			continue;
			
		if((edict == client) || (cavort_settings_bots_ignore_humans.IntValue != 0 && !IsFakeClient(edict)))
			continue;
		
		if(!IsPlayerAlive(edict))
			continue;
		
		if(!ClientCanSeeClient(client, edict, _, true))
			continue;
		
		bool isEdictEnemy = (GetClientTeam(client) != GetClientTeam(edict));

		if( cavort_settings_bots_friendly_with_humans.IntValue == 2 && !IsFakeClient(edict) )
			isEdictEnemy = false;

		if(isSpyInvisible(edict))
			continue;
		
		switch (isEdictEnemy)
		{
			case true:
			{
				g_BotData[client].validEnemies.Push(edict);
			}
			case false:
			{
				g_BotData[client].validTeammates.Push(edict);
			}
		}
	}
}

int Bot_GetEnemy(int client)
{
	float clientOrigin[3], edictOrigin[3];
	GetClientEyePosition(client, clientOrigin);
	float distance = -1.0;
	int nearestEntity = -1;

	for( int x = 0; x < g_BotData[client].validEnemies.Length; x++ )
	{
		int edict = g_BotData[client].validEnemies.Get(x);
		
		if(!IsValidClient(edict))
			continue;
		
		float edict_distance = GetVectorDistance(clientOrigin, edictOrigin);
		
		if((edict_distance < distance) || (distance == -1.0))				
		{
			distance = edict_distance;
			nearestEntity = edict;
		}
	}
	return nearestEntity;
}

int Bot_GetTeammate(int client)
{
	float clientOrigin[3], edictOrigin[3];
	GetClientEyePosition(client, clientOrigin);
	float distance = -1.0;
	int nearestEntity = -1;

	for( int x = 0; x < g_BotData[client].validTeammates.Length; x++ )
	{
		int edict = g_BotData[client].validTeammates.Get(x);
		
		if(!IsValidClient(edict))
			continue;
		
		float edict_distance = GetVectorDistance(clientOrigin, edictOrigin);
		if((edict_distance < distance) || (distance == -1.0))				
		{
			distance = edict_distance;
			nearestEntity = x;
		}
	}
	return nearestEntity;
}

bool Bot_IsTargetStillValid(int client, int edict)
{
	if(!IsValidClient(client) || !IsValidClient(edict))
		return false;
		
	if((edict == client) || (cavort_settings_bots_ignore_humans.IntValue != 0 && !IsFakeClient(edict)))
		return false;
	
	if(!IsPlayerAlive(edict))
		return false;
	
	if(!ClientCanSeeClient(client, edict))
		return false;

	if(isSpyInvisible(edict))
		return false;
		
	return true;
}

// how the bot aims at a specific target
void Bot_AimAtTarget(int client, int target, bool head=false, float result[3])
{
	if(!IsValidClient(client) || !IsValidClient(target))
		return;
		
	float enemyPos[3];
	if( BonePosAvaliable )
	{
		/*
			good hitboxes
		
			bip_head: tied to the hitbox itself
			bip_neck: doesnt have its own hitbox but its juuuust close enough that most shots will hit the head. however, its much less guarenteed, considering some angles and some models have it closer to the body. plus, in some cases you can actually MISS!
			bip_spine_0 - bip_spine_3: some playermodels' chest
			bip_pelvis: pelvis / hip area
		*/
		
		float unused[3];
		int iBone = SDKCall(g_hLookupBone, target, (head) ? "bip_head" : "bip_pelvis");
		GetClientAbsOrigin(target, enemyPos);
		SDKCall(g_hGetBonePosition, target, iBone, enemyPos, unused);
		// proper way, this grabs the position based off of the actual bone position, rather than just using the eye pos and hoping the animations don't fuck over the bot
	}
	else
	{
		if(head)
			GetClientEyePosition(target, enemyPos);
		else
		{
			GetClientAbsOrigin(target, enemyPos);
			enemyPos[2] += 50.0;
		}
		// old way, now a fallback
		// this isnt really a problem most of the time, but the head position is not always the same as the eye pos due to playermodel animations, so this can sometimes cause the bot to aim incorrectly
	}
	
	float projvel = GetProjectileVelocity(client);
	if(projvel > 0.0)
	{
		int gravity = GetConVarInt(FindConVar("sv_gravity")) * -1;
		float distance, time, gfactor;
		float distanceToGround = GetClientDistanceToGround( target );
		float minDistance = 50.0;
		float targetVelocity[3], calc[3];
		
		GetClientVelocityArray(target, targetVelocity);
		
		distance = getClientDistanceFromClient(client, target);
		time += GetWarmup(client);
	
		time = distance / projvel;
		
		gfactor = 0.5 * gravity * time * time;
		if(distanceToGround < minDistance)
			gfactor = 0.0;
		
		calc[0] = enemyPos[0] + targetVelocity[0] * time; 
		calc[1] = enemyPos[1] + targetVelocity[1] * time;
		calc[2] = enemyPos[2] + targetVelocity[2] * time + gfactor;

		if(!isClientAirborne(target) && !head)
			calc[2] -= 50.0;
			
			
		enemyPos = calc;
	}
	
	result = enemyPos;
}

// how the plugin forces the bot to aim
// this is kept in the bot utilities bcuz i edit this a lot
stock void TF2_LookAtPos(int client, float flGoal[3], float flAimSpeed = 0.05)
{
	float flPos[3];
	GetClientEyePosition(client, flPos);

	float flAng[3];
	GetClientEyeAngles(client, flAng);
	
	// get normalised direction from target to client
	float desired_dir[3];
	MakeVectorFromPoints(flPos, flGoal, desired_dir);
	GetVectorAngles(desired_dir, desired_dir);
	
	// ease the current direction to the target direction
	flAng[0] += AngleNormalize(desired_dir[0] - flAng[0]) * flAimSpeed;
	flAng[1] += AngleNormalize(desired_dir[1] - flAng[1]) * flAimSpeed;

	TeleportEntity(client, NULL_VECTOR, flAng, NULL_VECTOR);
}

// normally with IN_ATTACK it would be easy to just put that there and have the bot fire
// however, considering how a LOT of weapons have special properties, this isnt really possible
// for example, the beggars bazooka would cause bots to just overload and self destruct, due to them just holding attack forever
// this function aims to fix all of this
void Bot_Fire(int client)
{
	int buttons = g_BotData[client].WantedButtons;
	
	int activeWeapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	char weaponName[128];
	weaponName = TF2_GetWeaponClassName(activeWeapon);
	// the reason it uses the classname of the weapon rather than the weapon slot is due to randomizer sometimes having weapons in different slots/classes then they usually are
	// ex: scouts using demoman weapons / double shotguns / etc
	int index = ((IsValidEntity(activeWeapon)) ? GetEntProp(activeWeapon, Prop_Send, "m_iItemDefinitionIndex") : -1);
	// TFClassType class = TF2_GetPlayerClass(client);

	buttons |= IN_ATTACK;

	if(index == 730) // beggars bazooka
	{
		int clipsize = -1;
		int AmmoTable = FindSendPropInfo("CTFWeaponBase", "m_iClip1");
		clipsize = GetEntData(activeWeapon, AmmoTable);
		// having this exposed instead of being in a function probably isnt needed, as iirc there IS one in utilities.inc
		// not gonna bother though, it doesnt really matter anyway
		if(clipsize == 3)
		{
			if(buttons & IN_ATTACK)
				buttons &= ~IN_ATTACK;
		}
		// prevents them from overloading
	}
	
	if(StrContains(weaponName, "tf_weapon_sniperrifle", false) != -1)
	{
		if(!TF2_IsPlayerInCondition(client, TFCond_Zoomed) && index != 1098)
			buttons |= IN_ATTACK2;

		float chargedDamage = GetEntPropFloat(activeWeapon, Prop_Send, "m_flChargedDamage");
		if(StrContains(weaponName, "tf_weapon_sniperrifle_classic", false) != -1)
		{
			//classic
			if( !(chargedDamage < 150.0) )
			{
				if(buttons & IN_ATTACK)
					buttons &= ~IN_ATTACK;
				// if charge wont headshot or kill the sniper's target, bot wont fire
			}
		}
		else
		{
			if(!(chargedDamage > 0.0))
			{
				if(buttons & IN_ATTACK)
					buttons &= ~IN_ATTACK;
				// if cant headshot, does not fire
			}
		}
	}
	
	if(StrContains(weaponName, "tf_weapon_knife", false) != -1)
	{
		// makes bots with knifes stab whenever ( autostab LOLZ )
		// the autostab isnt really a problem due to the spy bot kinda needing it in order for them to be good
		
		if(!CheckIfAbleToBackstab(client, g_BotData[client].Target))
		{
			if(buttons & IN_ATTACK)
				buttons &= ~IN_ATTACK;
			// if cant backstab, spy will not swing
		}
		else
		{
			if(GetItemIndexOfSlot(client, 4) == 59 && GetEntProp(client, Prop_Send, "m_bFeignDeathReady") == 1)
			{
				buttons |= IN_ATTACK2;
				// makes spies put away their dead ringer
				// IDEA: make spies NOT do this if there is a nearby rocket that will hit them OR their target is about to shoot at them
				// sometimes its not the best idea to stab, and since the backstab check isnt instant, then sometimes the bot might take it out when it was only able to stab briefly
			}
		}
	}
	
	if( StrContains(TF2_GetWeaponClassName(GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon")), "TF_WEAPON_CANNON", false) != -1 ) // loose cannon is the only weapon ingame with TF_WEAPON_CANNON
	{
		float loosecannon_Time = GetGameTime() - GetEntPropFloat(GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon"), Prop_Send, "m_flDetonateTime");
		if(!(loosecannon_Time > 0.0))
		{
			if(buttons & IN_ATTACK)
				buttons &= ~IN_ATTACK;
		}
	}
	// prevents bots from constantly holding down attack with the loose cannon causing them to self-explode
	
	g_BotData[client].WantedButtons |= buttons;
}

void Bot_State(int client)
{
	// this is for checking if the bot has an advantage in battle or not
	// check initialize.inc for list of states
	
	int enemyPower = 0;
	int teamPower = 0;
	// int individualPower = 0;
	int edictPower = 0;
	float clientOrigin[3], edictOrigin[3];
	GetClientAbsOrigin(client, clientOrigin);
	TFClassType class = TFClass_Unknown;
	
	for( int x = 0; x < g_BotData[client].validEnemies.Length; x++ )
	{
		int edict = g_BotData[client].validEnemies.Get(x);
		
		if(!IsValidClient(edict))
			continue;
	
		class = TF2_GetPlayerClass(edict);
		edictPower = 1;
		GetClientAbsOrigin(edict, edictOrigin);
		switch(class)
		{
			case TFClass_Heavy:
			{
				switch( TF2_IsPlayerInCondition(client, TFCond_Slowed) ) // revved up
				{
					case true:
					{
						edictPower += 4;
						
					}
					case false:
					{
						edictPower += 1;
						
					}
				}
				
			}
			case TFClass_Sniper:
			{
				int disPoint = RoundToCeil(Pow((GetFastDistance(clientOrigin, edictOrigin) / 1024.0), 3.0));
				if(disPoint > 8) disPoint = 8;
				if(GetClientHealth(client) <= 150 && disPoint != 0) disPoint *= 2; // if headshot leads to death, then sniper danger is doubled
				edictPower += disPoint; // distance based, close up = 0 pts, far away = up to 8 pts
			}
			case TFClass_Medic:
			{
				if( g_BotData[client].validEnemies.Length >= 2 )
					edictPower++; // if there are 2 or more enemies, and the medic is one of them, then the medic is probably healing someone, so danger is increased
				else
					edictPower = 0; // medics are ez kills
			}
			case TFClass_Spy:
			{
				if(isSpyInvisible(client))
					edictPower = 0;
			}
			default:
			{
				edictPower++;
			}
		}
		if(TF2_IsPlayerBuffedDangerous(edict))
		{
			edictPower++;
			edictPower *= 5;
		}
		if(TF2_IsPlayerUbercharged(client))
			edictPower *= 5;
	
		enemyPower += edictPower;
	}
	
	bool teamMed = false;
	for( int x = 0; x < g_BotData[client].validTeammates.Length; x++ )
	{
		int edict = g_BotData[client].validTeammates.Get(x);
		
		if(!IsValidClient(edict))
			continue;
	
		class = TF2_GetPlayerClass(edict);
		edictPower = 1;
		GetClientAbsOrigin(edict, edictOrigin);
		switch(class)
		{
			case TFClass_Heavy:
			{
				switch( TF2_IsPlayerInCondition(client, TFCond_Slowed) ) // revved up
				{
					case true:
					{
						edictPower += 4;
					}
					case false:
					{
						edictPower += 1;	
					}
				}
			}
			case TFClass_Medic:
			{
				teamMed = true;
			}
			default:
			{
				edictPower++;
			}
		}
		if(TF2_IsPlayerBuffedDangerous(edict))
		{
			edictPower++;
			edictPower *= 5;
		}
		if(TF2_IsPlayerUbercharged(client))
			edictPower *= 5;
	
		teamPower += edictPower;
	}
	
	if(teamMed) teamPower *= 1.5;
	
	g_BotData[client].state = STATE_NORMAL;
	
	if( enemyPower > teamPower )
	{
		if( enemyPower > teamPower * 5 )
			g_BotData[client].state = STATE_FATAL_DISADVANTAGE;
		else if( enemyPower > teamPower * 2 )
			g_BotData[client].state = STATE_BIG_DISADVANTAGE;
		else
			g_BotData[client].state = STATE_DISADVANTAGE;
	}
	else if( enemyPower < teamPower )
	{
		if( enemyPower * 5 < teamPower )
			g_BotData[client].state = STATE_LARGE_ADVANTAGE;
		else if( enemyPower * 2 < teamPower )
			g_BotData[client].state = STATE_BIG_ADVANTAGE;
		else
			g_BotData[client].state = STATE_ADVANTAGE;
	}
}