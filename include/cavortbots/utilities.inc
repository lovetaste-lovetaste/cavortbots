// base stuff that is used in a lot of stuff
// this is NOT stuff that controls the bots
// this is used in OTHER stuff that THEN controls the bots
// check base.inc for the actual base bot brain

int ServerHostClient;

stock void moveForward(float vel[3],float MaxSpeed)
{
	vel[0] = MaxSpeed;
}

stock void moveBackwards(float vel[3],float MaxSpeed)
{
	vel[0] = -MaxSpeed;
}

stock void moveRight(float vel[3],float MaxSpeed, bool random=false, int chance=3)
{
	vel[1] = MaxSpeed * (( random && GetRandomInt(0,10) < chance ) ? -1 : 1);
}

stock void moveLeft(float vel[3],float MaxSpeed, bool random=false, int chance=3)
{
	vel[1] = -MaxSpeed * (( random && GetRandomInt(0,10) < chance ) ? -1 : 1);
}

stock void Bot_MoveDirection(int client, int forwardmove, int sidemove)
{
	// just for bots bro

	if(!IsValidClient(client))
		return;
		
	switch(sidemove)
	{
		case 1:
			g_BotData[client].WantedButtons |= IN_MOVERIGHT;
		case -1:
			g_BotData[client].WantedButtons |= IN_MOVELEFT;
	}
	switch(forwardmove)
	{
		case 1:
			g_BotData[client].WantedButtons |= IN_FORWARD;
		case -1:
			g_BotData[client].WantedButtons |= IN_BACK;
	}
}

stock bool IsValidClient(int client) 
{
	if(!(1 <= client <= MaxClients) || !IsClientInGame(client) || client < 0) 
		return false; 
	return true; 
}

bool isBot(int client)
{	
	if(!IsValidClient(client))
		return false;
	
	if(cavort_settings_enabled.IntValue == 0)
		return false;
	
	if(cavort_settings_force_enable.IntValue != 0)
		return true;
		
	if(!g_BotData[client].Enabled)
		return false;
	
	// if(human_bots.IntValue == 1)
		// return true;
	
	if(IsFakeClient(client))
		return true; // obviously
		
	return false;
}


stock bool ClientCanSeeCoords(int client, float endPos[3], float distance = 0.0, any force=false) // wall detection
{
	if(!IsValidClient(client))
		return false;
	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);
	if (distance == 0.0 || GetVectorDistance(clientOrigin, endPos, false) < distance)
	{
		Handle trace;
		trace = TR_TraceRayFilterEx(clientOrigin, endPos, MASK_ALL, RayType_EndPoint, Base_TraceFilter, force);
		if(TR_DidHit(trace))
		{
			CloseHandle(trace);
			return false;
		}
		CloseHandle(trace);
		return true;
	}
	else
		return false;
}

stock bool CoordsCanSeeCoords(float startPos[3], float endPos[3], float distance = 0.0, any force=false) // wall detection
{
	if (distance <= 0.0 || GetVectorDistance(startPos, endPos, false) < distance)
	{
		Handle trace;
		trace = TR_TraceRayFilterEx(startPos, endPos, MASK_ALL, RayType_EndPoint, Base_TraceFilter, force);
		if(TR_DidHit(trace))
		{
			CloseHandle(trace);
			return false;
		}
		CloseHandle(trace);
		return true;
	}
	else
		return false;
}

stock bool CoordsCanSeeCoordsHull(float startPos[3], float startMins[3], float startMaxs[3], float endPos[3], float distance = 0.0, bool force=false) // wall detection
{
	if (distance == 0.0 || GetVectorDistance(startPos, endPos, false) < distance)
	{
		Handle trace;
		trace = TR_TraceHullFilterEx(startPos, endPos, startMins, startMaxs, MASK_ALL, Base_TraceFilter, force);
		if(TR_DidHit(trace))
		{
			CloseHandle(trace);
			return false;
		}
		CloseHandle(trace);
		return true;
	}
	else
		return false;
}

stock bool ClientCanSeeCoordsHull(int client, float endPos[3], float distance = 0.0, bool force=false) // wall detection for client hulls
{
	if(!IsValidClient(client))
		return false;
	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);

	if (distance == 0.0 || GetVectorDistance(clientOrigin, endPos, false) < distance)
	{
		clientOrigin[2] += 10.0; // just in case, for floors
		float clientMaxs[3], clientMins[3];
		GetEntPropVector(client, Prop_Send, "m_vecMaxs", clientMaxs);
		GetEntPropVector(client, Prop_Send, "m_vecMins", clientMins);
	
		Handle trace;
		trace = TR_TraceHullFilterEx(clientOrigin, endPos, clientMins, clientMaxs, MASK_ALL, Base_TraceFilter, force);
		if(TR_DidHit(trace))
		{
			CloseHandle(trace);
			return false;
		}
		CloseHandle(trace);
		return true;
	}
	else
		return false;
}

stock bool ClientCanSeeClient(int client, int target, float dis = 0.0, bool fovcheck=false) // to make the code cleaner, does both wall detection and fov check
{	
	if(!IsValidClient(client) || !IsValidClient(target) || !IsPlayerAlive(target))
		return false;

	float clientOrigin[3];
	GetClientEyePosition(client, clientOrigin);
	
	float targetOrigin[3];
	GetClientAbsOrigin(target, targetOrigin);
	
	float targetMaxs[3], targetMins[3];
	GetEntPropVector(target, Prop_Send, "m_vecMaxs", targetMaxs);
	GetEntPropVector(target, Prop_Send, "m_vecMins", targetMins);
	
	AddVectors(targetMins, targetOrigin, targetMins);
	AddVectors(targetMaxs, targetOrigin, targetMaxs);
	
	bool wallcheck = ( CoordsCanSeeCoords( clientOrigin, targetMaxs, dis, client) || CoordsCanSeeCoords( clientOrigin, targetMins, dis, client) || CoordsCanSeeCoords( clientOrigin, targetOrigin, dis, client) );
	
	if(fovcheck)
	{
		if( angleToFaceCoords(client, targetOrigin, true) > 70.0 )
			wallcheck = false;
		/*
			the reason why the angle check is 70 and not 90 is due to tf2's ""fov"" being weird.
			on a simple 1366x768 moniter, the point in which people get cut off almost fully is at 70-ish fov.
			90 is pretty much overkill. I GUESS you can make the argument that since higher moniters can get higher fovs, this should be increased. however, most people dont have this! plus, its restricted in windowed mode due to sv_restrict_aspect_ratio
		*/
	}
	
	return wallcheck;
}

stock float GetClientDistanceToGround( int client ) 
{
	// Player is already standing on the ground?
	if( GetEntPropEnt( client, Prop_Send, "m_hGroundEntity" ) == 0 || (GetEntityFlags(client) & FL_ONGROUND) )
		return 0.0;
	
	float fOrigin[ 3 ], fGround[ 3 ];
	GetClientAbsOrigin( client, fOrigin );
	
	fOrigin[ 2 ] += 10.0;
	
	TR_TraceRayFilter( fOrigin, view_as<float>( { 90.0, 0.0, 0.0 } ), MASK_ALL, RayType_Infinite, Base_TraceFilter, client);
	if ( TR_DidHit() ) 
	{
		TR_GetEndPosition( fGround );
		fOrigin[ 2 ] -= 10.0;
		return GetVectorDistance( fOrigin, fGround );
	}
	return 0.0;
}

stock float GetClientDistanceToCeiling(int client ) 
{   
	float fOrigin[ 3 ], fGround[ 3 ];
	GetClientAbsOrigin( client, fOrigin );
	
	fOrigin[ 2 ] += 10.0;
	
	TR_TraceRayFilter( fOrigin, view_as<float>( { -90.0, 0.0, 0.0 } ), MASK_ALL, RayType_Infinite, Base_TraceFilter, client);
	if ( TR_DidHit() ) 
	{
		TR_GetEndPosition( fGround );
		fOrigin[ 2 ] -= 10.0;
		return GetVectorDistance( fOrigin, fGround );
	}
	return 0.0;
}

stock bool TF2_IsNextToWall(int client) // Stock By Pelipokia
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	float flMaxs[3], flMins[3];
	GetEntPropVector(client, Prop_Send, "m_vecMaxs", flMaxs);
	GetEntPropVector(client, Prop_Send, "m_vecMins", flMins);
	
	flMaxs[0] += 2.5;
	flMaxs[1] += 2.5;
	flMins[0] -= 2.5;
	flMins[1] -= 2.5;
	
	flPos[2] += 18.0;
	
	//Perform a wall check to see if we are near any obstacles we should try jump over
	Handle TraceRay = TR_TraceHullFilterEx(flPos, flPos, flMins, flMaxs, MASK_ALL, Base_TraceFilter, client);
	bool bHit = TR_DidHit(TraceRay);
	CloseHandle(TraceRay);
	
	return bHit;
}

public bool Base_TraceFilter(int entity, int contentsMask, any data)
{
	// return false = do not hit this entity, keep ray going
	// return true = hit the ray
	
	if(data==true)
		return false;

	static char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	
	if (StrEqual(class, "entity_medigun_shield"))
		return false;
	else if (StrEqual(class, "func_respawnroomvisualizer"))
		return false;
	else if (StrContains(class, "tf_projectile_", false) != -1)
		return false;
	else if (StrContains(class, "obj_", false) != -1)
		return false;
	else if (StrContains(class, "tf_zombie", false) != -1)
		return false;
	else if (StrContains(class, "tank_boss", false) != -1)
		return false;
	else if (StrContains(class, "merasmus", false) != -1)
		return false;
	else if (StrContains(class, "eyeball_boss", false) != -1)
		return false;
	else if (StrContains(class, "headless_hatman", false) != -1)
		return false;
	else if (StrContains(class, "tf_robot_destruction_robot", false) != -1)
		return false;
	else if (StrContains(class, "item_ammopack_", false) != -1)
		return false;
	else if (StrContains(class, "team_control_point", false) != -1)
		return false;
	else if (StrEqual(class, "entity_revive_marker"))
		return false;
	else if (StrEqual(class, "mapobj_cart_dispenser"))
		return false;
	else if (StrEqual(class, "tf_dropped_weapon"))
		return false;
	else if (StrEqual(class, "tf_ammo_pack"))
		return false;
	else if (StrEqual(class, "func_door"))
		return true;
	// else if (StrEqual(class, "func_moniter"))
		// return true;
		//PrintToServer("%s", class);
	else if (StrEqual(class, "player"))
	{
		if ( IsValidClient(data) && IsValidClient(entity) && GetClientTeam(entity) == GetClientTeam(data) && IsPlayerAlive(entity) )
			return false;
	}
	return entity > MaxClients;
}

stock void GetCoordsOfClientView(int client, float buffer[3])
{
	// this gets the coordinates of where the client specified is looking at
	float clientOrigin[3], clientAngles[3];
	GetClientEyePosition(client, clientOrigin);
	GetClientEyeAngles(client, clientAngles);
	Handle trace;
	trace = TR_TraceRayFilterEx(clientOrigin, clientAngles, MASK_ALL, RayType_Infinite, Base_TraceFilter);
	TR_GetEndPosition(buffer, trace);
	CloseHandle(trace);
}

stock float AngleNormalize(float angle)
{
	angle = fmodf(angle, 360.0);
	if (angle > 180.0) 
	{
		angle -= 360.0;
	}
	else if (angle < -180.0)
	{
		angle += 360.0;
	}
	
	return angle;
}

stock float fmodf(const float number, const float denom)
{
	return number - RoundToFloor(number / denom) * denom;
}

stock void ClampAngle(float fAngles[3])
{
	if(fAngles[0] > 89.0)  fAngles[0]-=360.0;
	if(fAngles[0] < -89.0) fAngles[0]+=360.0;
	if(fAngles[1] > 180.0) fAngles[1]-=360.0;
	if(fAngles[1] <-180.0) fAngles[1]+=360.0;
}

stock float AngleDifference(float angle1, float angle2)
{
	float diff = (angle2 - angle1 + 180) % 360 - 180;
	return diff < -180 ? diff + 360 : diff;
}

stock void TF2_MoveTo(int client, float flGoal[3], float fVel[3], float fAng[3]) // Stock By Pelipokia
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	newmove[1] = -newmove[1];
	
	float sin = Sine(fAng[1] * FLOAT_PI / 180.0);
	float cos = Cosine(fAng[1] * FLOAT_PI / 180.0);						
	
	fVel[0] = cos * newmove[0] - sin * newmove[1];
	fVel[1] = sin * newmove[0] + cos * newmove[1];
	
	NormalizeVector(fVel, fVel);
	ScaleVector(fVel, 450.0);
}

stock void TF2_MoveAwayFrom(int client, float flGoal[3], float fVel[3], float fAng[3]) // Stock By Pelipokia
{
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	float newmove[3];
	SubtractVectors(flGoal, flPos, newmove);
	
	newmove[1] = -newmove[1];
	
	float sin = Sine(fAng[1] * FLOAT_PI / 180.0);
	float cos = Cosine(fAng[1] * FLOAT_PI / 180.0);						
	
	fVel[0] = cos * newmove[0] - sin * newmove[1];
	fVel[1] = sin * newmove[0] + cos * newmove[1];
	
	NormalizeVector(fVel, fVel);
	ScaleVector(fVel, -450.0);
}

stock bool IsWeaponSlotActive(int iClient, int iSlot)
{
	if(!IsValidClient(iClient))
		return false;
	return GetPlayerWeaponSlot(iClient, iSlot) == GetEntPropEnt(iClient, Prop_Send, "m_hActiveWeapon");
}

stock int GetActiveWeaponSlot(int client)
{
	if(!IsValidClient(client))
		return 0;

	for(int i = 0; i < 6; i++)
	{
		if(IsWeaponSlotActive(client, i))
			return i;
	}
   return 0;
}

stock int GetTeamNumber(const int entity)
{
	return GetEntProp(entity, Prop_Send, "m_iTeamNum");
}

stock int GetOwnerEntity(const int entity)
{
	return GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity");
}

stock int GetEntityThrower(const int entity)
{
	return GetEntPropEnt(entity, Prop_Send, "m_hThrower");
}

stock bool IsReloading(const int client)
{
	int weapon = GetEntPropEnt(client, Prop_Data, "m_hActiveWeapon");
	if (weapon != -1)
		return GetEntProp(weapon, Prop_Data, "m_bInReload") != 0;
	return false;
}

public int GetItemIndexOfSlot(int client, int slot)
{
	int item = GetPlayerWeaponSlot(client, slot);
	if(IsValidEntity(item))
		return GetEntProp(item, Prop_Send, "m_iItemDefinitionIndex");
	else
		return -1;
}

stock int GetHealth(int client)
{
	if(!IsValidClient(client) || !HasEntProp(client, Prop_Send, "m_iHealth"))
		return -1;
	
	if(TF2_IsPlayerUbercharged(client))
		return 99999;
		
	return GetEntProp(client, Prop_Send, "m_iHealth");
}

stock int GetMaxHealth(int client)
{
	return GetEntProp(client, Prop_Send, "m_iMaxHealth");
}

stock int GetMaxPlayerHealth(const int client)
{
	if (!IsValidClient(client) || g_iResourceEntity == -1)
		return -1;
		
	return GetEntProp(g_iResourceEntity, Prop_Send, "m_iMaxHealth", _, client);
}

stock int GetDuckState(int client)
{
	// from class movement
	int ducked = GetEntityFlags(client) & FL_DUCKING;
	int ducking = GetEntProp(client, Prop_Send, "m_bDucking");

	if (!ducking && !ducked) // Standing
		return 0;
	if (ducking && !ducked)  // Ducking in progress
		return 1;
	if (!ducking && ducked)  // Fully crouched
		return 2;
	return 3; // Standing up after being fully crouched
}

stock int TF2_GetHealingTarget(const int client)
{
	if(!IsValidClient(client))
		return -1;
	int index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if(!IsValidEntity(index))
		return -1;
	if (!HasEntProp(index, Prop_Send, "m_hHealingTarget"))
		return -1;
	return GetEntPropEnt(index, Prop_Send, "m_hHealingTarget");
}

stock float GetMaxSpeed(const int entity)
{
	return GetEntPropFloat(entity, Prop_Send, "m_flMaxspeed");
}

stock int GetClientScore(const int client)
{
	return GetEntProp(client, Prop_Data, "m_iFrags");
}

stock int GetClientDeath(const int client)
{
	return GetEntProp(client, Prop_Data, "m_iDeaths");
}

stock float GetKDR(const int client)
{
	return float(GetClientScore(client) + 1) / float(GetClientDeath(client) + 1);
}

stock int GetMetal(const int client)
{
	return GetEntProp(client, Prop_Data, "m_iAmmo", 4, 3);
}

stock float GetNextFiringTime(int client)
{
	float nextprimatt = 0.0;
	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon" );
	if(IsValidEntity(weapon) && IsValidEdict(weapon) && weapon != -1)
	{
		nextprimatt = GetEntPropFloat(weapon, Prop_Send, "m_flNextPrimaryAttack");
		nextprimatt -= GetGameTime();
		if(nextprimatt < 0.0)
			nextprimatt = 0.0;
	}
	return nextprimatt;
}
stock float GetNextIdleTime(int client)
{
	float nextprimatt = 0.0;
	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon" );
	if(IsValidEntity(weapon) && IsValidEdict(weapon) && weapon != -1)
	{
		nextprimatt = GetEntPropFloat(weapon, Prop_Send, "m_flTimeWeaponIdle");
		nextprimatt -= GetGameTime();
		if(nextprimatt < 0.0)
			nextprimatt = 0.0;
	}
	return nextprimatt;
}
stock float GetNextFiringTimeOfSlot(int client, int slot)
{
	float nextprimatt = 0.0;
	int weapon = GetPlayerWeaponSlot(client, slot);
	if(IsValidEntity(weapon) && IsValidEdict(weapon) && weapon != -1)
	{
		nextprimatt = GetEntPropFloat(weapon, Prop_Send, "m_flNextPrimaryAttack");
		nextprimatt -= GetGameTime();
		if(nextprimatt < 0.0)
			nextprimatt = 0.0;
	}
	return nextprimatt;
}

stock float GetStickybombLauncherChargeTime(int client)
{
	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if( !HasEntProp(weapon, Prop_Send, "m_flChargeBeginTime") )
		return -1.0;
	float begin_charge = GetEntPropFloat( weapon , Prop_Send, "m_flChargeBeginTime");
	float charge = (begin_charge == 0.0) ? 0.0 : GetGameTime() - begin_charge;
	return charge;
}

stock int GetWeaponClipSize(int client, int slot, bool returnReserveAtZeroClip=false)
{
	if(!IsValidClient(client))
		return -1;
		
	int weapon = GetPlayerWeaponSlot(client, slot);
	if(!IsValidEntity(weapon))
		return -1;
	
	int clipsize = -1;
	int AmmoTable = FindSendPropInfo("CTFWeaponBase", "m_iClip1");
	clipsize = GetEntData(weapon, AmmoTable);
	
	if( returnReserveAtZeroClip ? ( clipsize <= 0 ) : ( clipsize == -1 ) )
	{
		int iOffsetValidate = GetEntProp(weapon, Prop_Data, "m_iPrimaryAmmoType");
		if (iOffsetValidate != -1)
		{
			return GetEntProp(client, Prop_Send, "m_iAmmo", _, iOffsetValidate);
		}
	}
	return clipsize;
}

stock float getClientDistanceFromClient(int client, int target, int removeZ=0, bool closest=false)
{
	// removeZ = 0 = nothing
	// removeZ = 1 = removes height differences so the distance is flat / equal ground
	// removeZ = 2 = ONLY returns height difference
	// closest tries to get the closest z coord
	
	if(IsValidClient(client) && IsValidClient(target) && IsPlayerAlive(client) && IsPlayerAlive(target))
	{
		float clientOrigin[3];
		float enemyPosition[3];
		GetClientAbsOrigin(client, clientOrigin);
		GetClientAbsOrigin(target, enemyPosition);
		
		if(closest)
		{
			if(enemyPosition[2] - clientOrigin[2] > 84.0)
				clientOrigin[2] += 84;
			else if( enemyPosition[2] - clientOrigin[2] < 0.0 )
			{
				if( enemyPosition[2] - clientOrigin[2] < -84.0 )
					enemyPosition[2] += 84.0;
				else
					enemyPosition[2] += enemyPosition[2] - clientOrigin[2];
			}
			else
				clientOrigin[2] += enemyPosition[2] - clientOrigin[2];
		}
		
		if(removeZ == 1)
		{
			clientOrigin[2] = enemyPosition[2] = 0.0;
		}
		else if(removeZ == 2)
		{
			clientOrigin[0] = enemyPosition[0] = 0.0;
			clientOrigin[1] = enemyPosition[1] = 0.0;
			return	enemyPosition[2] - clientOrigin[2];
		}
		
		return GetVectorDistance(clientOrigin, enemyPosition);
	}
	return 0.0;
}
stock bool checkInvisStatusEffects(int client)
{
	// if return false = spy isnt truly invisible

	if (TF2_IsPlayerInCondition(client, TFCond_OnFire))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Jarated))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Bleeding))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Milked))
		return false;

	// if (TF2_IsPlayerInCondition(client, TFCond_TeleportedGlow))
		// return false;
	// cloaked spies don't have this effect on them
	// spies that arent cloaked will return as false anyway

	if (TF2_IsPlayerInCondition(client, TFCond_Sapped))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Gas))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_Bonked))
		return false;

	if (TF2_IsPlayerInCondition(client, TFCond_CloakFlicker))
		return false;
		
	return true;
}

stock bool isSpyInvisible(const int client) // from ebot utilities
{
	//if (TF2_IsPlayerInCondition(client, TFCond_Disguising))
		//return false;
	//if (TF2_IsPlayerInCondition(client, TFCond_Disguised))
		//return true;
	// disguises are not considered due to bots having logic for dealing with disguised spies

	// standard mode uses the custom logic in the plugin 
	// only noticable difference is that my plugin has spychecking and other stuff that normal players look for (slow scouts, non-healing medics, people looking directly at someone's back, unscoped snipers / unrevved heavies, etc), while the normal tfbot logic only looks for bumps and if you were just undisguised (they don't even care for dead ringers (not literally, stock tfbots DO fire at dead ringers, but once some time passes then they don't care) )
	
	if(!checkInvisStatusEffects(client))
		return false;
	
	if (TF2_IsPlayerInCondition(client, TFCond_DeadRingered))
		return true;
		
	// (TF2_IsPlayerInCondition(client, TFCond_Cloaked))
	if (GetPercentInvisible(client) >= 0.5)							// the cloaked condition immediately triggers when the spy starts cloaking. this check allows for the bot to still properly target spies in the progress of cloaking
		return true;

	if (TF2_IsPlayerInCondition(client, TFCond_Stealthed))
		return true;

	return false;
}

stock float Squaredf(const float number)
{
	return number * number;
}

stock bool CheckIfAbleToBackstab(int client, int target, bool distanceCheck=true)
{
	// the entprop SUCKS ASS at checking for backstab, so this manually does it
	// the entprop doesnt even stab when inspecting a weapon and is REALLY BAD
	// instead, this uses the same equations the actual game does, taken and converted directly from tf_weapon_knife.cpp in the source code

	if(!IsValidClient(client) || !IsValidClient(target))
		return false;

	float clientOrigin[3];
	float targetPosition[3];
	GetClientAbsOrigin(client, clientOrigin);
	GetClientAbsOrigin(target, targetPosition);
	
	float targetAng[3];
	GetClientEyeAngles(target, targetAng);
	float clientAng[3];
	GetClientEyeAngles(client, clientAng);

	float vecToTarget[3];
	SubtractVectors(targetPosition, clientOrigin, vecToTarget);
	vecToTarget[2] = 0.0;
	NormalizeVector(vecToTarget, vecToTarget);

	float vecOwnerForward[3];
	GetAngleVectors(clientAng, vecOwnerForward, NULL_VECTOR, NULL_VECTOR);
	vecOwnerForward[2] = 0.0;
	NormalizeVector(vecOwnerForward, vecOwnerForward);

	float vecTargetForward[3];
	GetAngleVectors(targetAng, vecTargetForward, NULL_VECTOR, NULL_VECTOR);
	vecTargetForward[2] = 0.0;
	NormalizeVector(vecTargetForward, vecTargetForward);

	// Make sure owner is behind, facing and aiming at target's back
	float flPosVsTargetViewDot = GetVectorDotProduct( vecToTarget, vecTargetForward );	// Behind?
	float flPosVsOwnerViewDot = GetVectorDotProduct( vecToTarget, vecOwnerForward );		// Facing?
	float flViewAnglesDot = GetVectorDotProduct( vecTargetForward, vecOwnerForward );	// Facestab?

	if(GetHealth(target) <= 45)
	{
		flViewAnglesDot = 1.0;
		flPosVsTargetViewDot = 1.0;
		flPosVsOwnerViewDot = 1.0;
		// this makes spies butterknife weak targets
		// i know butterknife dmg is 40, but realistically if the target is 45 hp its probably safe to just butterknife them to death
	}

	return ( flPosVsTargetViewDot >= 0.0 && flPosVsOwnerViewDot >= 0.5 && flViewAnglesDot >= -0.3 && ( getClientDistanceFromClient(client, target) <= 100.0 && distanceCheck ) );
}

stock int Find_Payload_Cart_Coords(float buffer[3], bool &bufferResult)
{
	// credit to MasterOfTheXP on the forums
	int watcherEnt = FindEntityByClassname(-1, "team_train_watcher");
	int payload = -1;
	if (IsValidEntity(watcherEnt))
	{
		//PrintToServer("found team_train_watcher");
		char trainName[64];
		GetEntPropString(watcherEnt, Prop_Data, "m_iszTrain", trainName, sizeof(trainName));
		int i = MaxClients+1;
		int trainEnt;
		while ((i = FindEntityByClassname(i, "*")) != -1)
		{
			char iName[64];
			GetEntPropString(i, Prop_Data, "m_iName", iName, sizeof(iName));
			if (!StrEqual(iName, trainName)) continue; // Next one
			trainEnt = i;
			//PrintToServer("found payload");
			break; // Stahp this whole loop, we found it.
		}
		if (trainEnt != 0) // not zero, so we found it.
		{
			payload = trainEnt;
			GetEntPropVector(trainEnt, Prop_Send, "m_vecOrigin", buffer);
			//PrintToServer("%f %f %f payload coords", buffer[0], buffer[1], buffer[2]);
			bufferResult = true;
		}
		else
			bufferResult = false;
	}
	else
	{
		bufferResult = false;
	}
	return payload;
}

stock void GetClientVelocityArray(int client, float buffer[3])
{
	if(IsValidClient(client) && IsPlayerAlive(client))
		GetEntPropVector(client, Prop_Data, "m_vecVelocity", buffer);
}

stock float GetClientVelocityFloat(int client, bool removeZ=true)
{
	if(!(IsValidClient(client) && IsPlayerAlive(client)))
		return 0.0;

	float buffer[3];
	GetEntPropVector(client, Prop_Data, "m_vecVelocity", buffer);
	if(removeZ)
		buffer[2] = 0.0;
	return GetVectorLength(buffer);
}

stock int GetNearestRocket(int client, float distanceLimit=0.0, int OwnedBy=0, bool enemyTeamOnly=true)
{
	if(!IsValidClient(client))
		return false;
		
	char ClassName[32];	
	float clientOrigin[3];	
	float entityOrigin[3];	
	float distance = -1.0;	
	int nearestEntity = -1;	
	int edict = -1;
	int clientTeam = GetClientTeam(client);
	while(IsValidEntity((edict = FindEntityByClassname(edict, "tf_projectile_rocket"))))
	{
		// todo: add cow mangler support (entity name is tf_projectile_energy_ball)
		GetEdictClassname(edict, ClassName, 32);								
		if(StrContains(ClassName, "tf_projectile_rocket", false) != -1)			
		{
			if(IsValidClient(OwnedBy))
			{
				if(GetOwnerEntity(edict) != OwnedBy)
					continue;
			}
			
			int rocketTeam = GetEntProp(edict, Prop_Send, "m_iTeamNum");
		
			if(rocketTeam == clientTeam && enemyTeamOnly)
				continue;
			
			GetEntPropVector(edict, Prop_Data, "m_vecOrigin", entityOrigin);				
			GetClientAbsOrigin(client, clientOrigin);								
			float edict_distance = GetVectorDistance(clientOrigin, entityOrigin);
			
			if(distanceLimit < edict_distance && distanceLimit > 0.0)
				continue;
				
			if((edict_distance < distance) || (distance == -1.0))				
			{					
				distance = edict_distance;					
				nearestEntity = edict;
			}			
		}	
	}
	
	return nearestEntity;
}

stock void PredictCoordsOfRocketLanding(int rocket, float buffer[3], bool debug=false, int client=-1)
{
	if(!IsValidEntity(rocket))
		return;
	char rocketName[32];
	GetEntityClassname(rocket, rocketName, sizeof(rocketName));
	if(StrContains(rocketName, "tf_projectile_rocket", false) == -1)
		return;
	float rocketOrigin[3], rocketAngles[3];
	GetEntPropVector(rocket, Prop_Send, "m_vecOrigin", rocketOrigin);
	GetEntPropVector(rocket, Prop_Send, "m_angRotation", rocketAngles);
	Handle trace;
	trace = TR_TraceRayFilterEx(rocketOrigin, rocketAngles, MASK_ALL, RayType_Infinite, Base_TraceFilter, true);
	TR_GetEndPosition(buffer, trace);
	CloseHandle(trace);
	if(debug == true && IsValidClient(client))
	{
		float square1[3], square2[3], square3[3], square4[3];
		
		square1[0] = buffer[0] - 10.0;
		square3[0] = buffer[0] - 10.0;
		square2[0] = buffer[0] + 10.0;
		square4[0] = buffer[0] + 10.0;
		
		square1[1] = buffer[1] + 10.0;
		square3[1] = buffer[1] - 10.0;
		square2[1] = buffer[1] + 10.0;
		square4[1] = buffer[1] - 10.0;
		
		square1[2] = buffer[2] + 1.0;
		square3[2] = buffer[2] + 1.0;
		square2[2] = buffer[2] + 1.0;
		square4[2] = buffer[2] + 1.0;
		
		Laser(client, square1, square2);
		Laser(client, square2, square4);
		Laser(client, square4, square3);
		Laser(client, square3, square1);
	}
}

int PredictRocketDamage(int client, int rocket, int base=90, int owner=-1, int crits=0)
{
	if(!IsValidEntity(rocket) || !IsValidClient(client))
		return -1;
	if(owner != -1 && !IsValidClient(owner))
		return -1;
	float clientOrigin[3], rocketOrigin[3];
	GetClientAbsOrigin(client, clientOrigin);
	GetEntPropVector(rocket, Prop_Send, "m_vecOrigin", rocketOrigin);
	float rocketDistance = GetVectorDistance(rocketOrigin, clientOrigin);
	if(owner != -1)
	{
		float ownerOrigin[3];
		GetClientAbsOrigin(owner, ownerOrigin);
		rocketDistance = GetVectorDistance(rocketOrigin, ownerOrigin);
	}
	
	if(rocketDistance > 1024.0)
		rocketDistance = 1024.0;
	if(crits > 0 && rocketDistance > 512.0)
		rocketDistance = 512.0; // both minicrits and full crits ignore damage falloff. this doesnt account for rampup due to minicrits having it
	if(crits == 2)
		rocketDistance = 512.0; // full crits always act as if its base dmg no matter what, ignoring rampup, so the distance gets set to 512. minicrits do not do this as they actually DO have damage rampup
		
	float multiplier = ((0.0 - rocketDistance) / 1024.0) + 1.5;
	if(crits == 1) multiplier += 0.35;
	if(crits == 2) multiplier += 2.0;
	
	return RoundFloat(multiplier * float(base));
}

bool IsEngineerBuilding(int entity)
{
	if (!IsValidEntity(entity))
		return false;
	
	char classname[64];
	GetEntityClassname(entity, classname, sizeof(classname));
	
	// Check for all engineer building variants
	return (StrContains(classname, "obj_sentrygun") == 0 ||
			StrContains(classname, "obj_dispenser") == 0 ||
			StrContains(classname, "obj_teleporter") == 0);
}

stock bool DoesPlayerOwnAEngieBuilding(int client)
{
	if(!IsValidClient(client))
		return false;
		
	for(int x = 0; x <= GetMaxEntities(); x++)	
	{
		if(!IsValidEntity(x))
			continue;
		char classname[64];
		GetEntityClassname(x, classname, sizeof(classname));
		if(StrContains(classname, "obj_sentrygun") == 0 || StrContains(classname, "obj_dispenser") == 0)
		{
			if(GetOwnerEntity(x) == client)
				return true;
		}
	}	
	return false;
}

stock int Find_Nearest_Building(int client, int mode=1, bool visibleOnly=true, bool damagedOnly=false, bool ignoreSapped=true, bool ignoreDisabled=true, float range=0.0, float percentageCompleted=100.0, int maxLevel=3)
{
	// mode 0 = all
	// mode 1 = enemyOnly
	// mode 2 = teammatesOnly
	float clientOrigin[3];
	GetClientAbsOrigin(client, clientOrigin);	
	float entityOrigin[3];	
	float distance = -1.0;	
	int nearestEntity = -1;	
	int clientTeam = GetClientTeam(client);
	
	for(int x = 0; x <= GetMaxEntities(); x++)	
	{
		if(IsEngineerBuilding(x))			
		{
			GetEntPropVector(x, Prop_Data, "m_vecOrigin", entityOrigin);
			if(!CoordsCanSeeCoords(clientOrigin, entityOrigin, range) && visibleOnly) // the range can be 0.0 here due to the CoordsCanSeeCoords automatically filtering out 0.0 distances
				continue;
				
			int sentryTeam = GetEntProp(x, Prop_Send, "m_iTeamNum");
			if(mode == 1 && clientTeam == sentryTeam)
				continue;
			if(mode == 2 && clientTeam != sentryTeam)
				continue;

				
			if(GetEntProp(x, Prop_Send, "m_bHasSapper") == 1 && (ignoreSapped || ignoreDisabled))
				continue;
			if(GetEntProp(x, Prop_Send, "m_bPlacing") == 1 && ignoreDisabled)
				continue;
			if(GetEntProp(x, Prop_Send, "m_bDisabled") == 1 && ignoreDisabled)
				continue;
			if(GetEntPropFloat(x, Prop_Send, "m_flPercentageConstructed") <= percentageCompleted && ( ignoreDisabled || percentageCompleted > 0.0)) // if the building's build progress is less than percentageCompleted and either ignoreDisabled is on or percentageCompleted is bigger than 0.0
				continue;	
			if(damagedOnly && !(GetHealth(x) < GetMaxHealth(x)))
				continue;
			if(maxLevel <= GetEntProp(x, Prop_Send, "m_iUpgradeLevel") && maxLevel > 0)
				continue;
				
			float edict_distance = GetVectorDistance(clientOrigin, entityOrigin);				
			if((edict_distance < distance) || (distance == -1.0))				
			{					
				distance = edict_distance;					
				nearestEntity = x;				
			}			
		}	
	}	
	
	return nearestEntity;
}

stock float angleToFaceCoords(int client, float inputted_coords[3], bool absoluteAngle=false, bool clientBasedOrigin=true)
{	
	//IMPORTANT: clientBasedOrigin is used for PLAYER ENTITIES ONLY. NON PLAYERS DISABLE THIS
	float clientpos[3];
	float targetpos[3];
	targetpos = inputted_coords;
	float anglevector[3];
	float targetvector[3];
	float resultangle = 0.0;
	if(absoluteAngle)
	{
		if(clientBasedOrigin)
		{
			GetClientAbsOrigin(client, clientpos);
			GetClientEyeAngles(client, anglevector);
		}
		else
		{
			GetEntPropVector(client, Prop_Send, "m_vecOrigin", clientpos);
			GetEntPropVector(client, Prop_Send, "m_angRotation", anglevector);
		}
		anglevector[0] = anglevector[2] = 0.0;
		GetAngleVectors(anglevector, anglevector, NULL_VECTOR, NULL_VECTOR);
		NormalizeVector(anglevector, anglevector);
		clientpos[2] = targetpos[2] = 0.0;
		MakeVectorFromPoints(clientpos, targetpos, targetvector);
		NormalizeVector(targetvector, targetvector);
		resultangle = RadToDeg(ArcCosine(GetVectorDotProduct(targetvector, anglevector)));
	}
	else
	{
		float clientAngles[3];
		if(clientBasedOrigin)
		{
			GetClientAbsOrigin(client, clientpos);
			GetClientEyeAngles(client, clientAngles);
			//GetClientAbsAngles is based off the models rotation, which doesnt always turn 100% where the player is actually looking
		}
		else
		{
			GetEntPropVector(client, Prop_Send, "m_vecOrigin", clientpos);
			GetEntPropVector(client, Prop_Send, "m_angRotation", clientAngles);
			//angRotation may or may not exist for all entities, so make sure to double check the netprops for your entity
		}
		MakeVectorFromPoints(clientpos, targetpos, targetvector);
		GetVectorAngles(targetvector, anglevector);
		resultangle = clientAngles[1] - anglevector[1];
		if (resultangle < -180)
		{
			resultangle = 360 + resultangle;
		}

		if (resultangle > 180)
		{
			resultangle = 360 - resultangle;
		}
	}
	return resultangle;
}

stock float angleCoordsToFaceCoords(float angles[3], float pos[3], float inputted_coords[3], bool absoluteAngle=false)
{
	float clientpos[3];
	clientpos = pos;
	float targetpos[3];
	targetpos = inputted_coords;
	float anglevector[3];
	float targetvector[3];
	float resultangle = 0.0;
	if(absoluteAngle)
	{
		anglevector[0] = anglevector[2] = 0.0;
		GetAngleVectors(anglevector, anglevector, NULL_VECTOR, NULL_VECTOR);
		NormalizeVector(anglevector, anglevector);
		clientpos[2] = targetpos[2] = 0.0;
		MakeVectorFromPoints(clientpos, targetpos, targetvector);
		NormalizeVector(targetvector, targetvector);
		resultangle = RadToDeg(ArcCosine(GetVectorDotProduct(targetvector, anglevector)));
	}
	else
	{
		MakeVectorFromPoints(clientpos, targetpos, targetvector);
		GetVectorAngles(targetvector, anglevector);
		resultangle = angles[1] - anglevector[1];
		if (resultangle < -180)
		{
			resultangle = 360 + resultangle;
		}

		if (resultangle > 180)
		{
			resultangle = 360 - resultangle;
		}
	}
	return resultangle;
}

stock void TF2_GetTFClassTypeToString(TFClassType classType, char[] buffer, int maxlen)
{
	switch (classType)
	{
		case TFClass_Scout:
			strcopy(buffer, maxlen, "scout");
		case TFClass_Sniper:
			strcopy(buffer, maxlen, "sniper");
		case TFClass_Soldier:
			strcopy(buffer, maxlen, "soldier");
		case TFClass_DemoMan:
			strcopy(buffer, maxlen, "demoman");
		case TFClass_Medic:
			strcopy(buffer, maxlen, "medic");
		case TFClass_Heavy:
			strcopy(buffer, maxlen, "heavyweapons");
		case TFClass_Pyro:
			strcopy(buffer, maxlen, "pyro");
		case TFClass_Spy:
			strcopy(buffer, maxlen, "spy");
		case TFClass_Engineer:
			strcopy(buffer, maxlen, "engineer");
		default:
			strcopy(buffer, maxlen, "unknown");
	}
}

stock void TF2_GetIntToTFClassString(int index, char[] buffer, int maxlen)
{
	switch (view_as<TFClassType>(index))
	{
		case TFClass_Scout:
			strcopy(buffer, maxlen, "scout");
		case TFClass_Sniper:
			strcopy(buffer, maxlen, "sniper");
		case TFClass_Soldier:
			strcopy(buffer, maxlen, "soldier");
		case TFClass_DemoMan:
			strcopy(buffer, maxlen, "demoman");
		case TFClass_Medic:
			strcopy(buffer, maxlen, "medic");
		case TFClass_Heavy:
			strcopy(buffer, maxlen, "heavyweapons");
		case TFClass_Pyro:
			strcopy(buffer, maxlen, "pyro");
		case TFClass_Spy:
			strcopy(buffer, maxlen, "spy");
		case TFClass_Engineer:
			strcopy(buffer, maxlen, "engineer");
		default:
			strcopy(buffer, maxlen, "unknown");
	}
}

stock bool isClientAirborne(int client, int groundDistanceLimit=21)
{
	// !(GetEntityFlags(client) & FL_ONGROUND)
	if( GetClientDistanceToGround(client) > groundDistanceLimit )
		return true;
	
	// PrintToServer("%N %f distance to ground", client, GetClientDistanceToGround(client));
	//if(GetEntityMoveType(client) == MOVETYPE_NOCLIP)
		//return true; //noclip, mainly so bots fly directly at people with melee
	
	return false;
}

stock bool IsClientMoving(int client, float wantedVel=0.0, bool verticalVel=false)
{
	float buffer[3];
	GetEntPropVector(client, Prop_Data, "m_vecVelocity", buffer);
	if(!verticalVel)
		buffer[2] = 0.0;
	return (GetVectorLength(buffer) > wantedVel);
}

stock void Laser(int client, float start[3], float end[3], float life=0.1, int r=255, int g=255, int b=255, bool halo=false, bool lift2eye=false)
{
	// must be in debug mode for this to be enabled.
	if(!IsValidClient(client) || life < 0.0 || cavort_settings_debug_enable.IntValue != 1)
		return;
	int laserEnt = PrecacheModel("materials/sprites/laserbeam.vmt");
	int coloring[4];
	coloring[0] = r;
	coloring[1] = g;
	coloring[2] = b;
	coloring[3] = 255;
	
	if( lift2eye )
	{
		float eye[3];
		GetClientEyePosition(client, eye);
		if( start[2] < eye[2] )
			start[2] = eye[2];
		if( end[2] < eye[2] )
			end[2] = eye[2];
	}
	
	TE_SetupBeamPoints(start, end, laserEnt, laserEnt, 0, 30, life, 1.0, 1.0, 1, 0.0, coloring, 20);
	TE_SendToClient(client);
	
	if( halo )
	{
		int g_HaloSprite = PrecacheModel("materials/sprites/glow.vmt");  
		TE_SetupBeamRingPoint(end, 100.0, 99.0, laserEnt, g_HaloSprite, 0, 15, life, 5.0, 0.0, coloring, 10, 0);
		TE_SendToClient(client);
	}
}

stock void Halo(int client, float start[3], float life=0.1, int r=255, int g=255, int b=255)
{
	// must be in debug mode for this to be enabled.
	if(!IsValidClient(client) || life < 0.0 || cavort_settings_debug_enable.IntValue != 1)
		return;
	int laserEnt = PrecacheModel("materials/sprites/laserbeam.vmt");
	int g_HaloSprite = PrecacheModel("materials/sprites/glow.vmt");  
	int coloring[4];
	coloring[0] = r;
	coloring[1] = g;
	coloring[2] = b;
	coloring[3] = 255;
	TE_SetupBeamRingPoint(start, 100.0, 99.0, laserEnt, g_HaloSprite, 0, 15, life, 5.0, 0.0, coloring, 10, 0);
	TE_SendToClient(client);
}

stock bool isUberAbovePercent(int client, float minPercent=100.0, int slot=-1)
{
	if(!IsValidClient(client))
		return false;
	int active = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
	if(slot != -1)
		active = GetPlayerWeaponSlot(client, slot);
	char weaponName[64];
	if(active != -1)
		GetEntityClassname(active, weaponName, sizeof(weaponName));
	if( StrContains(weaponName, "tf_weapon_medigun", false) == -1 )
		return false;
	
	float charge = GetEntPropFloat(active, Prop_Send, "m_flChargeLevel");
	if( (minPercent / 100.0) > charge )
		return false;
		
	return true;
}

stock bool TF2_IsPlayerBuffed(int client)
{
	// buffs
	if(!IsValidClient(client) || !IsPlayerAlive(client))
		return false;
		
	return (TF2_IsPlayerInCondition(client, TFCond_Buffed) || TF2_IsPlayerInCondition(client, TFCond_Charging) || TF2_IsPlayerInCondition(client, TFCond_CritCola) || TF2_IsPlayerInCondition(client, TFCond_CritDemoCharge) || TF2_IsPlayerInCondition(client, TFCond_CritMmmph) || TF2_IsPlayerInCondition(client, TFCond_CritOnKill) || TF2_IsPlayerInCondition(client, TFCond_DeadRingered) || TF2_IsPlayerInCondition(client, TFCond_DefenseBuffed) || TF2_IsPlayerInCondition(client, TFCond_Kritzkrieged) || TF2_IsPlayerInCondition(client, TFCond_MegaHeal) || TF2_IsPlayerUbercharged(client));
}

bool TF2_IsPlayerUbercharged(int client)
{
	// ubers
	if(!IsValidClient(client) || !IsPlayerAlive(client))
		return false;
	
	return (TF2_IsPlayerInCondition(client, TFCond_Ubercharged) || TF2_IsPlayerInCondition(client, TFCond_UberchargedCanteen) || TF2_IsPlayerInCondition(client, TFCond_UberchargedHidden) || TF2_IsPlayerInCondition(client, TFCond_UberchargedOnTakeDamage));
}

bool TF2_IsPlayerBuffedDangerous(int client)
{
	// conds that buff player but can still die while using them
	if(!IsValidClient(client) || !IsPlayerAlive(client))
		return false;
		
	return (TF2_IsPlayerInCondition(client, TFCond_Buffed) || TF2_IsPlayerInCondition(client, TFCond_Charging) || TF2_IsPlayerInCondition(client, TFCond_CritCola) || TF2_IsPlayerInCondition(client, TFCond_CritDemoCharge) || TF2_IsPlayerInCondition(client, TFCond_CritMmmph) || TF2_IsPlayerInCondition(client, TFCond_CritOnKill) || TF2_IsPlayerInCondition(client, TFCond_DefenseBuffed) || TF2_IsPlayerInCondition(client, TFCond_Kritzkrieged) || TF2_IsPlayerInCondition(client, TFCond_MegaHeal));
}

stock float GetProjectileVelocity(int client) 
{
	//this is taken from athairus's airshot trainer
	
	float pv = 0.0; // if not changed, current weapon is a hitscan/melee weapon

	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon" );
	
	if(!IsValidEntity(weapon))
		return 0.0;

	// http://wiki.teamfortress.com/wiki/Projectiles
	// http://wiki.alliedmods.net/Team_Fortress_2_Item_Definition_Indexes

	//if(!IsFakeClient(client))
		//PrintToServer("%N wep index %i", client, GetEntProp( weapon, Prop_Send, "m_iItemDefinitionIndex" ));

	switch( GetEntProp( weapon, Prop_Send, "m_iItemDefinitionIndex" ) ) 
	{
		case 56, 1005, 1092: // huntsman/fortified compound
		{
			float begin_charge = GetEntPropFloat( weapon , Prop_Send, "m_flChargeBeginTime");
			float charge = (begin_charge == 0.0) ? 0.0 : GetGameTime() - begin_charge;
			pv = RemapValClamped(charge, 0.0, 1.0, 1800.0, 2600.0);
			//PrintToServer("%f huntsman charge", pv);
		}
		case 305: // crusader's crossbow
			pv = 2400.0;

		case 44, 648:  // sandman and wrap assasin
			pv = 3000.0;

		case 812, 833: // flying guillotine
			pv = 3000.0;

		case 441: // cow mangler 5000
			pv = 1100.0;

		case 442: // righteous bison
			pv = 1200.0;
		case 588: // pomson 6000
			pv = 1200.0;

		case 39: // flare gun 
			pv = 2000.0;
		case 740: // scorch shot 
			pv = 2000.0;
		case 351: // detonator
			pv = 2000.0;

		case 595: // manmelter
			pv = 3000.0;

		case 19:  // grenade laucher
			pv = 1220.0;
		case 206: // renamed gl
			pv = 1220.0;
		case 1151: // iron bomber
			pv = 1220.0;

		case 308: // loch-n-load
			pv = 1575.0;

		case 996: // loose cannon
			pv = 1440.0;

		case 58: // jarate
			pv = 935.0;
		case 222: // mad milk
			pv = 935.0;

		case 18, 889, 800, 809, 898, 907, 916, 965, 974, 15006, 15014, 15028, 15043, 15052, 15057, 15081, 15104, 15105, 15129, 15130, 15150:  // rocket launcher
			pv = 1100.0;
			
		case 1104:  // air strike
			pv = 1100.0;
			
		case 658:  // rocket launcher
			pv = 1100.0;
			
		case 205: // renamed rl
			pv = 1100.0;
			
		case 228: // black box
			pv = 1100.0;
		case 237: // rocket jumper
			pv = 1100.0;
		case 513: // the orginal
			pv = 1100.0;
		case 730: // beggar's bazooka
			pv = 1100.0;

		case 127: // direct hit
			pv = 1540.0;

		case 414: // liberty launcher
			pv = 1540.0;

		case 581: // MONOCULUS
			pv = 1100.0;

		case 140: // wrangler (level 3 rockets)
			pv = 1100.0;

		case 20, 207, 130, 265, 661, 1150:	// stickybomb launcher, scottish resistance, sticky jumper, stickies
		{
			float begin_charge = GetEntPropFloat( weapon , Prop_Send, "m_flChargeBeginTime");
			float charge = (begin_charge == 0.0) ? 0.0 : GetGameTime() - begin_charge;
			pv = RemapValClamped((charge), 0.0, 4.0, 900.0, 2400.0);
		}
		
		case 17: // syringe gun
			pv = 1000.0;
		case 204: // syringe gun (renamed/strange)
			pv = 1000.0;
		case 36: // blutsauger
			pv = 1000.0;
		case 412: // overdose
			pv = 1000.0;

		case 997: // rescue ranger
			pv = 2400.0;

	}
	return pv;
}

stock float RemapValClamped(float val, float A, float B, float C, float D)
{
	// grabbed from haven_tf2
	if (A == B)
		return val >= B ? D : C;
	float cVal = (val - A) / (B - A);
	if(cVal < 0.0)
		cVal = 0.0;
	if(cVal > 1.0)
		cVal = 1.0;
	return C + (D - C) * cVal;
}

stock float GetProjectileGravity(int client) 
{
	//this is taken from athairus's airshot trainer

	int weapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon" );
	
	if(!IsValidEntity(weapon))
		return 0.0;

	float weapon_gravity = 0.0;

	switch( GetEntProp( weapon, Prop_Send, "m_iItemDefinitionIndex" ) ) 
	{
		case 997, 305, 1079:
			weapon_gravity = 0.2;
		case 17, 204, 36, 412, 1081, 39, 740:
			weapon_gravity = 0.3;
		case 56, 1092:
		{
			float begin_charge = GetEntPropFloat( weapon , Prop_Send, "m_flChargeBeginTime");
			float charge = (begin_charge == 0.0) ? 0.0 : GetGameTime() - begin_charge;
			weapon_gravity = RemapValClamped(charge, 0.0, 1.0, 0.5, 0.1);
		}
		case 19, 206, 1007, 996, 308, 1151, 1150, 20:
			weapon_gravity = 1.5;
		case 44:
			weapon_gravity = 0.5;
		default:
			weapon_gravity = 0.0;
	}
	return weapon_gravity;
}

stock float GetWarmup( int player ) 
{
	int weapon = GetEntPropEnt( player, Prop_Send, "m_hActiveWeapon" );

	switch( GetEntProp( weapon, Prop_Send, "m_iItemDefinitionIndex" ) ) {

		case 44:  // sandman
			return 0.25;
		case 648: // the wrap assassin
			return 0.25;

		case 812: // flying guillotine
			return 0.25;
		case 833: // flying guillotine (genuine)
			return 0.25;
	}
	return 0.0;
}

stock void GetPositionAroundPlayer(int client, float distance = 64.0, int mode=0, float buffer[3])
{
	// mode 0 = behind
	// mode 1 = front
	// mode 2 = left
	// mode 3 = right
	
	if (!IsValidClient(client))
		return;
	
	float clientPos[3], clientAngles[3];
	float forw[3], right[3], up[3];
	
	// Get player's current position and angles
	GetClientAbsOrigin(client, clientPos);
	GetClientEyeAngles(client, clientAngles);
	
	// Get the direction vectors based on player's facing direction
	GetAngleVectors(clientAngles, forw, right, up);
	
	if(mode == 0)
	{
		buffer[0] = clientPos[0] - (forw[0] * distance);
		buffer[1] = clientPos[1] - (forw[1] * distance);
	}
	else if(mode == 1)
	{
		buffer[0] = clientPos[0] + (forw[0] * distance);
		buffer[1] = clientPos[1] + (forw[1] * distance);
	}
	else if(mode == 2)
	{
		buffer[0] = clientPos[0] - (right[0] * distance);
		buffer[1] = clientPos[1] - (right[1] * distance);
	}
	else if(mode == 3)
	{
		buffer[0] = clientPos[0] + (right[0] * distance);
		buffer[1] = clientPos[1] + (right[1] * distance);
	}
	
	
	buffer[2] = clientPos[2]; // Keep same height
	
	buffer[2] += 10.0;
}

stock void GetPositionAroundCoords(float startPos[3], float ang[3], float distance = 64.0, int mode=0, float buffer[3])
{
	// mode 0 = behind
	// mode 1 = front
	// mode 2 = left
	// mode 3 = right
	
	float forw[3], right[3], up[3];
	// Get the direction vectors based on player's facing direction
	GetAngleVectors(ang, forw, right, up);
	
	float pos[3];
	AddVectors(startPos, pos, pos);
	
	if(mode == 0)
	{
		buffer[0] = pos[0] - (forw[0] * distance);
		buffer[1] = pos[1] - (forw[1] * distance);
	}
	else if(mode == 1)
	{
		buffer[0] = pos[0] + (forw[0] * distance);
		buffer[1] = pos[1] + (forw[1] * distance);
	}
	else if(mode == 2)
	{
		buffer[0] = pos[0] - (right[0] * distance);
		buffer[1] = pos[1] - (right[1] * distance);
	}
	else if(mode == 3)
	{
		buffer[0] = pos[0] + (right[0] * distance);
		buffer[1] = pos[1] + (right[1] * distance);
	}
	
	buffer[2] = pos[2]; // Keep same height
	
	buffer[2] += 10.0;
}

stock void GetPositionAroundPlayerRelativeToViewer(int viewer, int target, float distance, float result[3])
{
	// if distance is negative then it will be on the left
	// else it is on right

	// Get target's position
	float targetPos[3];
	GetClientAbsOrigin(target, targetPos);
	
	// Get viewer's eye position and angles
	float viewerPos[3], viewerAngles[3];
	GetClientEyePosition(viewer, viewerPos);
	GetClientEyeAngles(viewer, viewerAngles);
	
	// Calculate direction from viewer to target
	float directionToTarget[3];
	SubtractVectors(targetPos, viewerPos, directionToTarget);
	NormalizeVector(directionToTarget, directionToTarget);
	
	// Get the right vector from viewer's angles
	float rightVector[3];
	GetAngleVectors(viewerAngles, NULL_VECTOR, rightVector, NULL_VECTOR);
	
	// Calculate the offset position
	result[0] = targetPos[0] + (rightVector[0] * distance);
	result[1] = targetPos[1] + (rightVector[1] * distance);
	result[2] = targetPos[2] + (rightVector[2] * distance);
}

stock void GetPositionAroundCoordsRelativeToViewer(float coords[3], int viewer, float distance, float result[3])
{
	// if distance is negative then it will be on the left
	// else it is on right
	
	// Get viewer's eye position and angles
	float viewerPos[3], viewerAngles[3];
	GetClientEyePosition(viewer, viewerPos);
	GetClientEyeAngles(viewer, viewerAngles);
	
	// Calculate direction from viewer to target
	float directionToTarget[3];
	SubtractVectors(coords, viewerPos, directionToTarget);
	NormalizeVector(directionToTarget, directionToTarget);
	
	// Get the right vector from viewer's angles
	float rightVector[3];
	GetAngleVectors(viewerAngles, NULL_VECTOR, rightVector, NULL_VECTOR);
	
	// Calculate the offset position
	result[0] = coords[0] + (rightVector[0] * distance);
	result[1] = coords[1] + (rightVector[1] * distance);
	result[2] = coords[2] + (rightVector[2] * distance);
}

stock char[] TF2_GetWeaponClassName(const int weapon)
{
	char className[64] = "UNKNOWN";
	if(!IsValidEntity(weapon))
		return className;
	GetEntityClassname(weapon, className, sizeof(className));
	return className;
}

public Action OnWeaponSwitch(int client, int weapon) 
{
	if(IsValidClient(client) && isBot(client) && IsPlayerAlive(client))
	{
		// Make sure we calculate ammo so we can switch if we're out!
		int ammoOffset = FindSendPropInfo("CTFPlayer", "m_iAmmo");
		int size = GetEntData(client, ammoOffset + 4, 4);
		
		if(g_BotData[client].ban_primary && IsWeaponSlotActive(client, 0) && size > 0)
		{
			//PrintToServer("%N ban primary switch OnWeaponSwitch", client);
			return Plugin_Handled;
		}
		else if(g_BotData[client].ban_secondary && IsWeaponSlotActive(client, 1) && size > 0)
		{
			//PrintToServer("%N ban secondary switch OnWeaponSwitch", client);
			return Plugin_Handled;
		}
		else if(g_BotData[client].ban_melee && IsWeaponSlotActive(client, 2))
		{
			//PrintToServer("%N ban melee switch OnWeaponSwitch", client);
			return Plugin_Handled;
		}
	}
	else
	{
		// Let's ensure these are reset if these conditions aren't met.
		g_BotData[client].ban_primary = false;
		g_BotData[client].ban_secondary = false;
		g_BotData[client].ban_melee = false;
	}
	return Plugin_Continue;
}

public Action OnWeaponCanSwitchTo(int client, int weapon) 
{
	if(IsValidClient(client) && IsFakeClient(client) && IsPlayerAlive(client))
	{
		// Make sure we calculate ammo so we can switch if we're out!
		int ammoOffset = FindSendPropInfo("CTFPlayer", "m_iAmmo");
		int size = GetEntData(client, ammoOffset + 4, 4);
		
		if(g_BotData[client].ban_primary && IsWeaponSlotActive(client, 0) && size > 0)
		{
			//PrintToServer("%N ban primary switch OnWeaponCanSwitchTo", client);
			return Plugin_Handled;
		}
		else if(g_BotData[client].ban_secondary && IsWeaponSlotActive(client, 1) && size > 0)
		{
		//	PrintToServer("%N ban secondary switch OnWeaponCanSwitchTo", client);
			return Plugin_Handled;
		}
		else if(g_BotData[client].ban_melee && IsWeaponSlotActive(client, 2))
		{
			//PrintToServer("%N ban melee switch OnWeaponCanSwitchTo", client);
			// vprof_generate_report
			return Plugin_Handled;
		}
	}
	else
	{
		// Let's ensure these are reset if these conditions aren't met.
		g_BotData[client].ban_primary = false;
		g_BotData[client].ban_secondary = false;
		g_BotData[client].ban_melee = false;
	}
	return Plugin_Continue;
}

stock void TF2_SwitchtoSlot(int client, int slot)
{
//	float switchTimeLimit = ( 0 - ( g_BotData[client].MechanicalSkill / ( 100 / g_BotData[client].BaseReactionTime ) ) ) + g_BotData[client].BaseReactionTime;
	//if( switchTimeLimit <= 0.0 )
		//switchTimeLimit = 0.0;
	//if( switchTimeLimit > g_BotData[client].BaseReactionTime )
		//switchTimeLimit = g_BotData[client].BaseReactionTime;
	//
	//if(GetGameTime() <= (g_BotData[client].LastSwitchedWeapon + switchTimeLimit))
		//return;
	// if the bot switches too fast based on their MechanicalSkill, then the bot cannot switch again, and has to wait for longer
	
	// g_BotData[client].LastSwitchedWeapon = GetGameTime();
	
	if(slot >= 0 && slot <= 5 && IsValidClient(client) && IsPlayerAlive(client) && !IsWeaponSlotActive(client, slot) && IsFakeClient(client))
	{
		char playername[32];
		GetClientName(client, playername, sizeof(playername));
		int wep = GetPlayerWeaponSlot(client, slot);
		if(wep > MaxClients && IsValidEntity(wep))
		{
			// from Showin 
			int flags = GetCommandFlags( "cc_bot_selectweapon" );
			SetCommandFlags( "cc_bot_selectweapon", flags & ~FCVAR_CHEAT ); 
			FakeClientCommand(client, "cc_bot_selectweapon \"%s\" %i", playername, slot);
			SetCommandFlags( "cc_bot_selectweapon", flags|FCVAR_CHEAT); 
			
			// backup, this just forces it
			// this is behind a debug command due to it being a bit TOO direct
			// sometimes this can break things too, so just keep that in mind
			//if(bot_debug_weapon_force_switch.IntValue == 1)
		//		SetEntPropEnt(client, Prop_Send, "m_hActiveWeapon", wep);
		}
	}
}

stock void Bot_SwitchToSlot(int client, int slot)
{
	// this is mainly for ease of access, to prevent conflicting weapon switch bans for the bots.
	// remember, the weapon switch stuff DOES NOT WORK for human clients. I'll figure a universal one eventually, but for now just keep that in mind
	
	if(slot == 0)
	{
		g_BotData[client].ban_primary = true;
		g_BotData[client].ban_secondary = false;
		g_BotData[client].ban_melee = false;
	}
	if(slot == 1)
	{
		g_BotData[client].ban_primary = false;
		g_BotData[client].ban_secondary = true;
		g_BotData[client].ban_melee = false;
	}
	if(slot == 2)
	{
		g_BotData[client].ban_primary = false;
		g_BotData[client].ban_secondary = false;
		g_BotData[client].ban_melee = true;
	}
	if(!IsWeaponSlotActive(client, slot))
		TF2_SwitchtoSlot(client, slot);
}

stock int FindNearestHealth(int client, float maxDistance=-1.0, bool visible=true, float maxHeightDifference=0.0)
{
	char ClassName[32];	
	float clientOrigin[3];	
	GetClientAbsOrigin(client, clientOrigin);		
	float entityOrigin[3];	
	float distance = -1.0;	
	int nearestEntity = -1;	
	
	for(int x = 0; x <= GetMaxEntities(); x++)	
	{		
		if(IsValidEdict(x) && IsValidEntity(x))		
		{			
			GetEdictClassname(x, ClassName, 32);						
			if(StrContains(ClassName, "prop_dynamic", false) == -1 && !HasEntProp(x, Prop_Send, "m_fEffects"))				
				continue;						
			if(StrContains(ClassName, "prop_dynamic", false) == -1 && GetEntProp(x, Prop_Send, "m_fEffects") != 0)				
				continue;						
			if(StrContains(ClassName, "item_health", false) != -1 || StrContains(ClassName, "obj_dispenser", false) != -1 || StrContains(ClassName, "func_regen", false) != -1 || StrContains(ClassName, "rd_robot_dispenser", false) != -1 || StrContains(ClassName, "pd_dispenser", false) != -1)			
			{				
				GetEntPropVector(x, Prop_Data, "m_vecOrigin", entityOrigin);
				
				if( visible && !CoordsCanSeeCoords(entityOrigin, clientOrigin) )
					continue;
				if( maxHeightDifference > 0.0 && ( entityOrigin[2] - clientOrigin[2] ) > maxHeightDifference )
					continue;
				float edict_distance = GetVectorDistance(clientOrigin, entityOrigin);
				if(maxDistance > 0.0 && edict_distance > maxDistance) continue;
				if((edict_distance < distance) || (distance == -1.0))				
				{
					distance = edict_distance;					
					nearestEntity = x;				
				}			
			}		
		}	
	}	
	
	return nearestEntity;
}

public int FindNearestAmmo(int client)
{	
	char ClassName[32];	
	float clientOrigin[3];	
	float entityOrigin[3];	
	float distance = -1.0;	
	int nearestEntity = -1;	
	
	for(int x = 0; x <= GetMaxEntities(); x++)	
	{		
		if(IsValidEdict(x) && IsValidEntity(x))		
		{			
			GetEdictClassname(x, ClassName, 32);						
			if(StrContains(ClassName, "prop_dynamic", false) == -1 && !HasEntProp(x, Prop_Send, "m_fEffects"))				
				continue;						
			if(StrContains(ClassName, "prop_dynamic", false) == -1 && GetEntProp(x, Prop_Send, "m_fEffects") != 0)				
				continue;						
			if(StrContains(ClassName, "item_ammopack", false) != -1 || StrContains(ClassName, "obj_dispenser", false) != -1 || StrContains(ClassName, "func_regen", false) != -1 || StrContains(ClassName, "rd_robot_dispenser", false) != -1 || StrContains(ClassName, "pd_dispenser", false) != -1)			
			{				
				GetEntPropVector(x, Prop_Data, "m_vecOrigin", entityOrigin);				
				GetClientEyePosition(client, clientOrigin);								
				float edict_distance = GetVectorDistance(clientOrigin, entityOrigin);				
				if((edict_distance < distance) || (distance == -1.0))				
				{					
					distance = edict_distance;					
					nearestEntity = x;				
				}			
			}		
		}	
	}	
	
	return nearestEntity;
}

stock int bodyGroup(int client, int target=-1)
{
	/*
		Head = 1,
		Neck = 2,
		Pelvis = 4,
		Body = 5,
		Chest = 7
		from some lmaobox lua
		idk if this is accurate for tf2 except for the head bodygroup, that is accurate
	*/
	// not hitting anything = -1
	
	if(!IsValidClient(client))
		return -1;
	
	int selected = GetClientAimTarget(client);
	
	if(IsValidClient(target))
		selected = target;
		
	float botOrigin[3], clientAngles[3];
	GetClientEyePosition(client, botOrigin);
	GetClientEyeAngles(client, clientAngles);
	
	int hitgroup = -1;
	
	if(IsValidClient(selected))
	{
		Handle headshotTrace;
		headshotTrace = TR_ClipRayToEntityEx(botOrigin, clientAngles, MASK_ALL, RayType_Infinite, selected);
		hitgroup = TR_GetHitGroup(headshotTrace);
		CloseHandle(headshotTrace);
	}
	
	return hitgroup;
}

public Action Timer_BotTaunt(Handle hTimer, Handle tauntData)
{
	ResetPack( tauntData );
	int client = ReadPackCell(tauntData);
	if( !IsValidClient(client) )
		return Plugin_Handled;
	
	FakeClientCommand(client, "taunt");
	return Plugin_Handled;
}

stock float isClientFacingAwayFromClient(int client, int target, bool rawOutput=false)
{
	if(!IsValidClient(client) || !IsValidClient(target))
		return -1.0;
	// math converted from tf2 source code
	float targetOrigin[3];
	GetClientAbsOrigin(target, targetOrigin);
	float clientOrigin[3];
	GetClientAbsOrigin(target, clientOrigin);
	
	float clientAng[3];
	GetClientEyeAngles(client, clientAng);
	float targetAng[3];
	GetClientEyeAngles(target, targetAng);

	float vecToTarget[3];
	SubtractVectors(clientOrigin, targetOrigin, vecToTarget);
	vecToTarget[2] = 0.0;
	NormalizeVector(vecToTarget, vecToTarget);

	float vecOwnerForward[3];
	GetAngleVectors(clientAng, vecOwnerForward, NULL_VECTOR, NULL_VECTOR);
	vecOwnerForward[2] = 0.0;
	NormalizeVector(vecOwnerForward, vecOwnerForward);
	
	float vecTargetForward[3];
	GetAngleVectors(targetAng, vecTargetForward, NULL_VECTOR, NULL_VECTOR);
	vecTargetForward[2] = 0.0;
	NormalizeVector(vecTargetForward, vecTargetForward);
	
	float flViewAnglesDot = GetVectorDotProduct( vecTargetForward, vecOwnerForward );	// Facestab?
	
	//PrintToServer("%f", flViewAnglesDot);
	
	if( rawOutput )
		return flViewAnglesDot;
	
	return  ( (flViewAnglesDot > -0.3) ? 1.0 : 0.0 );
}

stock bool isThereGroundAboveClient(int client, float wantedHeight=200.0, float wantedDistance=400.0, int target=-1)
{
	// mainly for rocket jumping bots
	// not perfect
	
	if(!IsValidClient(client))
		return false;
		
	float clientPos[3], clientAngles[3];
	GetClientAbsOrigin(client, clientPos);
	GetClientEyeAngles(client, clientAngles);
	
	if( IsValidEntity(target) )
	{
		float eyePos[3];
		GetClientEyePosition(client, eyePos);
		float targetPos[3];
		
		if( IsValidClient(target) )
			GetClientAbsOrigin(target, targetPos);
		else
			GetEntPropVector(target, Prop_Send, "m_vecOrigin", targetPos);
			
		float desired_dir[3];
		MakeVectorFromPoints(eyePos, targetPos, desired_dir);
		GetVectorAngles(desired_dir, desired_dir);
		clientAngles[0] += AngleNormalize(desired_dir[0] - clientAngles[0]);
		clientAngles[1] += AngleNormalize(desired_dir[1] - clientAngles[1]);
		
		// float angang[3];
		// AddVectors(angang, clientAngles, angang);
		
		// angang[0] += 85.0;
		// angang[1] -= 180.0;
		
		// Handle boner = TR_TraceRayFilterEx(eyePos, angang, MASK_SOLID, RayType_Infinite, Base_TraceFilter);
		// float tracetrace[3];
		// TR_GetEndPosition(tracetrace, boner);
		// CloseHandle(boner);
		
		// Laser(ServerHostClient, eyePos, tracetrace);
	}
	
	// Trace forward and upward to detect elevated positions
	float endPos[3];
	endPos[0] = clientPos[0] + Cosine(DegToRad(clientAngles[1])) * wantedDistance;
	endPos[1] = clientPos[1] + Sine(DegToRad(clientAngles[1])) * wantedDistance;
	endPos[2] = clientPos[2] + wantedHeight;
	
	float resultEndPos[3];
	resultEndPos = endPos;
	/* Handle trace1 = TR_TraceRayFilterEx(clientPos, endPos, MASK_SOLID, RayType_EndPoint, Base_TraceFilter);
	TR_GetEndPosition(resultEndPos, trace1);
	// resultEndPos[2] = endPos[2];
	CloseHandle(trace1); */
	
	Handle trace2 = TR_TraceRayFilterEx(resultEndPos, view_as<float>( { 90.0, 0.0, 0.0 } ), MASK_SOLID, RayType_Infinite, Base_TraceFilter);
	bool hitSomething = TR_DidHit(trace2);
	float hitPos[3];
	TR_GetEndPosition(hitPos, trace2);
	CloseHandle(trace2);
	
	//Laser(ServerHostClient, resultEndPos, hitPos);
	
	// If there's a platform above us within range
	if (hitSomething && hitPos[2] > clientPos[2] + 82.0)
	{
		//PrintToServer("hitSomething %b", hitSomething);
		return !CoordsCanSeeCoords(clientPos, hitPos);
	}
	
	return false;
}

stock bool GetShieldPosition(int ally, int threat, float distance, float result[3])
{
	if(!IsValidClient(ally) || !IsValidClient(threat))
		return false;
	
	float allyPos[3], threatPos[3];
	GetClientAbsOrigin(ally, allyPos);
	GetClientAbsOrigin(threat, threatPos);
	
	float direction[3];
	SubtractVectors(allyPos, threatPos, direction);
	NormalizeVector(direction, direction);
	
	result[0] = allyPos[0] + (direction[0] * distance);
	result[1] = allyPos[1] + (direction[1] * distance);
	result[2] = allyPos[2];
	
	// Check if position is valid (not in wall, etc)
	return (ClientCanSeeCoords(threat, result));
}

stock int checkSticky(int client)
{
	// for bots only
	if(!isBot(client))
		return -1;
		
	int x;
	char ClassName[32];
	for (x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidEntity(x))
			continue;

		if (!HasEntProp(x, Prop_Send, "m_hThrower"))
			continue;

		if (GetEntityThrower(x) != client)
			continue;

		GetEdictClassname(x, ClassName, 32);
		//if (!ClassName)
			//continue;

		if (StrContains(ClassName, "tf_projectile_pipe_remote", false) != -1)
		{
			int edict = -1;
			float vecOrigin_center[3], x_origin[3], enemyOrigin[3];
			GetClientEyePosition(client, vecOrigin_center);
			GetEntPropVector(x, Prop_Send, "m_vecOrigin", x_origin);
			if(g_BotData[client].validEnemies != null && g_BotData[client].validEnemies.Length > 0)
			{
				for( int set = 0; set < g_BotData[client].validEnemies.Length; set++ )
				{
					edict = g_BotData[client].validEnemies.Get(set);
					if(!IsValidClient(edict) || !IsPlayerAlive(edict))
						continue;
					GetClientAbsOrigin(edict, enemyOrigin);
					float dis = GetVectorDistance(x_origin, enemyOrigin);
					if(dis <= 146.0)
						return x;
				}
			}
		}
	}
	return -1;
}

stock int GetClientHealer(int client)
{
	if(!IsValidClient(client))
		return -1;
	for(int i = 1; i <= MaxClients; i++)
	{
		if (!IsValidClient(i) || !IsPlayerAlive(i))
			continue;
		if(TF2_GetHealingTarget(i) == client)
		{
			return i;
		}
	}
	return -1;
}

stock bool WillInterceptPosition(int client, float targetPos[3], float maxDistance = 100.0, bool override=false, float overrideVel[3]={-1.0,-1.0,-1.0})
{
	float playerPos[3], velocity[3];
	GetClientAbsOrigin(client, playerPos);
	
	if( override )
		velocity = overrideVel;
	else
		GetClientVelocityArray(client, velocity);
	
	float speed = GetVectorLength(velocity);
	if (speed < 1.0)
		return false;
	
	float toTarget[3];
	SubtractVectors(targetPos, playerPos, toTarget);
	
	float dotProduct = GetVectorDotProduct(toTarget, velocity);
	float velocitySquared = speed * speed;
	float t = dotProduct / velocitySquared;
	
	// If t < 0, player is moving away from target
	if (t < 0.0)
		return false;
	
	// Find the closest point on the trajectory
	float closestPoint[3];
	closestPoint[0] = playerPos[0] + velocity[0] * t;
	closestPoint[1] = playerPos[1] + velocity[1] * t;
	closestPoint[2] = playerPos[2] + velocity[2] * t;
	
	// Calculate distance from closest point to target
	float distance = GetVectorDistance(closestPoint, targetPos);
	
	return distance <= maxDistance;
}

stock void GetAverageTeamPosition(int team, float buffer[3])
{
	float average[3], edictOrigin[3];
	int debug = 1;
	for(int i = 1; i <= MaxClients; i++)
	{
		if ( !IsValidClient(i) || !IsPlayerAlive(i) )
			continue;
		if( GetClientTeam(i) != team )
			continue;
		GetClientAbsOrigin(i, edictOrigin);
		average[0] += edictOrigin[0];
		average[1] += edictOrigin[1];
		average[2] += edictOrigin[2];
		debug++;
	}
	average[0] /= debug;
	average[1] /= debug;
	average[2] /= debug;
	buffer = average;
}

// from tf2ebots
stock int GetNearestEnemyControlPoint(int client)
{
	int NearestControlPoint = -1;

	float distance = -1.0;
	float nearestDistance = 9999999999999999.0;

	int iControlPoint;
	while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	{
		if (!IsValidEntity(iControlPoint))
			continue;

		if (GetClientTeam(client) == GetTeamNumber(iControlPoint))
			continue;

		distance = GetFastDistance(GetOrigin(client), GetOrigin(iControlPoint));
		if (distance < nearestDistance)
		{
			NearestControlPoint = iControlPoint;
			nearestDistance = distance;
		}
	}

	return NearestControlPoint;
	// old version
	
	// int NearestControlPoint = -1;
	
	// float distance = -1.0; 
	// float nearestDistance = -1.0;
	
	// int iControlPoint = -1;
	
	// while ((iControlPoint = FindEntityByClassname(iControlPoint, "team_control_point")) != -1)
	// {
		// if(!IsValidEntity(iControlPoint))
			// continue;
		
		// if(!IsValidEdict(iControlPoint))
			// continue;
		
		// if(GetEntProp(iControlPoint, Prop_Data, "m_bLocked") == 1)
			// continue;
		
		// if(GetClientTeam(client) == GetTeamNumber(iControlPoint))
			// continue;
		
		// distance = GetVectorDistance(GetOrigin(client), GetOrigin(iControlPoint), true);
        
		// if (distance < nearestDistance || nearestDistance == -1.0)
		// {
			// NearestControlPoint = iControlPoint;
			// nearestDistance = distance;
		// }
	// }
	
	// return NearestControlPoint;
}

stock float GetFastDistance(const float vec1[3], const float vec[3])
{
	static float result;
	result = (vec1[0] - vec[0]) + (vec1[1] - vec[1]) + (vec1[2] - vec[2]);
	if (result < 0)
		return -result;

	return result;
}

stock float[] GetOrigin(int entity)
{
	if(!IsValidEntity(entity))
		return {0.0,0.0,0.0};
	float vector[3];
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vector);
	return vector;
}

stock float GetPercentInvisible(const int client)
{
	int offset = FindSendPropInfo("CTFPlayer", "m_flInvisChangeCompleteTime") - 8;
	return GetEntDataFloat(client, offset);
}

stock int GetUsableSniperHintsCount(const int team)
{
	int SniperHintsCount = 0;
	int iEntity;
	while ((iEntity = FindEntityByClassname(iEntity, "func_tfbot_hint")) != -1)
	{
		if (!IsValidEntity(iEntity))
			continue;

		if (GetEntProp(iEntity, Prop_Data, "m_hint") != 0)
			continue;

		if (GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;

		if (GetTeamNumber(iEntity) != 0 && GetTeamNumber(iEntity) != team) // don't use enemy hints. but still can use UNDEFINED TEAM hints. (team != GetTeamNumber(iEntity) is team only hints.)
			continue;

		SniperHintsCount++;
	}

	return SniperHintsCount;
}

stock int GetNearestSniperSpot(const int team, int client)
{
	int iEntity;
	int iSelectedHint = -1;
	float distance = -1.0;

	float clientOrigin[3];
	GetClientAbsOrigin(client, clientOrigin);
	
	while ((iEntity = FindEntityByClassname(iEntity, "func_tfbot_hint")) != -1)
	{
		if (!IsValidEntity(iEntity))
			continue;

		if (GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;

		if (GetTeamNumber(iEntity) != 0 && GetTeamNumber(iEntity) != team)
			continue;

		if( distance == -1.0 || GetVectorDistance( clientOrigin, GetOrigin( iEntity ), true ) < distance )
		{
			distance = GetVectorDistance( clientOrigin, GetOrigin( iEntity ), true );
			iSelectedHint = iEntity;
		}
	}
	return iSelectedHint;
}

stock int GetRandomSniperSpot(const int team)
{
	int iEntity;
	int iSelectedHint = -1;
	ArrayList SniperHints = new ArrayList();

	while ((iEntity = FindEntityByClassname(iEntity, "func_tfbot_hint")) != -1)
	{
		if (!IsValidEntity(iEntity))
			continue;

		//if (GetEntProp(iEntity, Prop_Data, "m_hint") != 0)
			//continue;

		if (GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;

		if (GetTeamNumber(iEntity) != 0 && GetTeamNumber(iEntity) != team)
			continue;

		SniperHints.Push(iEntity);
	}

	if (SniperHints.Length > 0)
		iSelectedHint = SniperHints.Get(GetRandomInt(0, SniperHints.Length - 1));

	delete SniperHints;
	return iSelectedHint;
}

//  code is converted from nextbot source code
//----------------------------------------------------------------------------------------------------------
/**
 * Return true if this locomotor could potentially move along the line given.
 * If false is returned, fraction of walkable ray is returned in 'fraction'
 */
stock bool IsPotentiallyTraversable( const float from[3], const float to[3], float &buffer )
{
	float fraction = 0.0;
	
	// if 'to' is high above us, it's not directly traversable
	// Adding a bit of fudge room to allow for floating point roundoff errors
	if ( ( to[2] - from[2] ) > 50.0 )
	{
		float along[3];
		SubtractVectors(to, from, along);
		NormalizeVector(along, along);
		if ( along[2] > 0.6 )
		{
			fraction = 0.0;
			return false;
		}
	}

	// use a small hull since we cannot simulate collision resolution and avoidance along the way
	float probeSize = 0.25 * 50.0; // Cant be TOO small, or open stairwells/grates/etc will cause problems
	float probeZ = maxStepSize;

	float hullMin[3];
	hullMin[0] = -probeSize;
	hullMin[1] = -probeSize;
	hullMin[2] = probeZ;
	float hullMax[3];
	hullMax[0] = probeSize;
	hullMax[1] = probeSize;
	hullMax[2] = 32.0;
	
	Handle trace;
	trace = TR_TraceHullFilterEx( from, to, hullMin, hullMax, MASK_NPCSOLID, Base_TraceFilter );
	fraction = TR_GetFraction( trace );
	bool startsolid = TR_StartSolid( trace );
	buffer = fraction;
	CloseHandle( trace );
	
	return ( fraction >= 1.0 ) && ( !startsolid );
}


//----------------------------------------------------------------------------------------------------------
/**
 * Return true if there is a possible "gap" that will need to be jumped over
 * If true is returned, fraction of ray before gap is returned in 'fraction'
 */
stock bool HasPotentialGap( const float from[3], const float desiredTo[3], float &fraction )
{
	// find section of this ray that is actually traversable
	float traversableFraction;
	IsPotentiallyTraversable( from, desiredTo, traversableFraction );

	// compute end of traversable ray
	float to[3], add[3];
	SubtractVectors( desiredTo, from, add );
	ScaleVector( add, traversableFraction );
	AddVectors( from, add, to );
	
	float forwardvector[3], normalized[3];
	SubtractVectors(to, from, forwardvector);
	NormalizeVector(forwardvector, normalized);
	float length = GetVectorLength(normalized);
	float step = 13.0;

	// scan along the line checking for gaps
	float pos[3];
	pos = from;
	float delta[3], forv[3];
	forv = forwardvector;
	ScaleVector(forv, step);
	delta = forv;
	for( float t = 0.0; t < (length + step); t += step )
	{
		if ( IsGap( pos ) )
		{
			fraction = ( t - step ) / ( length + step );
			return true;
		}

		AddVectors(pos, delta, pos);
		// pos += delta;		
	}
	fraction = 1.0;

	return false;
}


//----------------------------------------------------------------------------------------------------------
/**
 * Return true if there is a "gap" here when moving in the given direction.
 * A "gap" is a vertical dropoff that is too high to jump back up to.
 */
stock bool IsGap( const float pos[3] )
{
	//float halfWidth = ( body ) ? body->GetHullWidth()/2.0f : 1.0f;

	// can't really jump effectively when crouched anyhow
	//float hullHeight = ( body ) ? body->GetStandHullHeight() : 1.0f;

	// use a small hull since we cannot simulate collision resolution and avoidance along the way
	const float halfWidth = 13.0;
	const float hullHeight = 68.0;

	Handle ground;

	// NextBotTraceFilterIgnoreActors filter( GetBot()->GetEntity(), COLLISION_GROUP_NONE );

	float maxstep[3], fifty[3];
	maxstep[0] = 0.0;
	maxstep[1] = 0.0;
	maxstep[2] = maxStepSize;
	fifty[0] = 0.0;
	fifty[1] = 0.0;
	fifty[2] = -50.0;
	AddVectors(pos, maxstep, maxstep);
	AddVectors(pos, fifty, fifty);
	float halfWidthVec[3], halfWidthHullHeight[3];
	halfWidthVec[0] = -halfWidth;
	halfWidthVec[1] = -halfWidth;
	halfWidthVec[2] = 0.0;
	halfWidthHullHeight[0] = halfWidth;
	halfWidthHullHeight[1] = halfWidth;
	halfWidthHullHeight[2] = hullHeight;
	ground = TR_TraceHullFilterEx( maxstep, fifty, halfWidthVec, halfWidthHullHeight, MASK_NPCSOLID, Base_TraceFilter);
	float fraction = TR_GetFraction(ground);
	bool startsolid = TR_StartSolid(ground);
	CloseHandle(ground);
	// if trace hit nothing, there's a gap ahead of us
	return ( fraction >= 1.0 && !startsolid );
}

// tfebots thingy
stock bool FakeClientCommandThrottled(const int client, const char[] command)
{
	if(g_BotData[client].lastCommand > GetGameTime())
		return false;
		
	FakeClientCommand(client, command);
	g_BotData[client].lastCommand = GetGameTime() + 2.0;
	return true;
}

public Action FindServerHost(Handle timer)
{
	ServerHostClient = -1;
	for(int i = 1; i <= MaxClients; i++)
	{
		if (IsValidClient(i) && !IsFakeClient(i))
		{
			ServerHostClient = i;
			break;
		}
	}
	return Plugin_Continue;
}

public Action command_toggleSpecificBotByName(int client, int args)
{
	if(GetCmdArgs() < 1)
	{
		ReplyToCommand(client, "ltbots_toggle_specific_bot_byName <client name>");
		return Plugin_Handled;
	}
	char arg1[32];
	GetCmdArg(1, arg1, sizeof(arg1));
	int bot = FindTarget(client, arg1, false, false);
	
	if(!IsValidClient(bot))
	{
		ReplyToCommand(client, "Invalid client.");
		return Plugin_Handled;
	}
	g_BotData[bot].Enabled = !g_BotData[bot].Enabled;
	if(g_BotData[bot].Enabled)
		ReplyToCommand(client, "%N's lovelybot improvements have been enabled.", bot);
	else
		ReplyToCommand(client, "%N's lovelybot improvements have been disabled.", bot);
	
	return Plugin_Handled;
}

public Action command_CheckStatusOfBotByName(int client, int args)
{
	if(GetCmdArgs() < 1)
	{
		ReplyToCommand(client, "ltbots_check_Status_bot_byName <client name>");
		return Plugin_Handled;
	}
	char arg1[32];
	GetCmdArg(1, arg1, sizeof(arg1));
	int bot = FindTarget(client, arg1, false, false);
	
	if(!IsValidClient(bot))
	{
		ReplyToCommand(client, "Invalid client.");
		return Plugin_Handled;
	}
	
	if(g_BotData[bot].Enabled)
		ReplyToCommand(client, "%N's lovelybot improvements are enabled.", bot);
	else
		ReplyToCommand(client, "%N's lovelybot improvements are disabled.", bot);
	return Plugin_Handled;
}

public Action command_EnableAll(int client, int args)
{
	int amount = 0;
	for(int i = 1; i < TF_MAXPLAYERS; i++)
	{
		if(!IsValidClient(i))
			continue;
		amount += 1;
		g_BotData[i].Enabled = true;
	}
	
	if(amount > 0)
		ReplyToCommand(client, "Enabled %i bots.", amount);
	else
		ReplyToCommand(client, "No bots have been enabled.");
	
	return Plugin_Handled;
}

public Action command_DisableAll(int client, int args)
{
	int amount = 0;
	for(int i = 1; i < MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		amount += 1;
		g_BotData[i].Enabled = false;
	}
	if(amount > 0)
		ReplyToCommand(client, "Disabled %i bots.", amount);
	else
		ReplyToCommand(client, "No bots have been disabled.");
	
	return Plugin_Handled;
}

public Action command_EnableAllTeam(int client, int args)
{
	char cmdArg[10];
	GetCmdArg(1, cmdArg, sizeof(cmdArg));
	int team = StringToInt(cmdArg);
	
	int amount = 0;
	for(int i = 1; i < TF_MAXPLAYERS; i++)
	{
		if(!IsValidClient(i))
			continue;
		if(team == GetClientTeam(i))
		{	
			amount += 1;
			g_BotData[i].Enabled = true;
		}
	}
	
	if(amount > 0)
		ReplyToCommand(client, "Enabled %i bots.", amount);
	else
		ReplyToCommand(client, "No bots have been enabled.");
	
	return Plugin_Handled;
}

public Action command_DisableAllTeam(int client, int args)
{
	char cmdArg[10];
	GetCmdArg(1, cmdArg, sizeof(cmdArg));
	int team = StringToInt(cmdArg);
	
	int amount = 0;
	for(int i = 1; i < MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		if(team == GetClientTeam(i))
		{
			amount += 1;
			g_BotData[i].Enabled = false;
		}
	}
	if(amount > 0)
		ReplyToCommand(client, "Disabled %i bots.", amount);
	else
		ReplyToCommand(client, "No bots have been disabled.");
	
	return Plugin_Handled;
}

public Action command_CreateSniperSpot(int client, int args)
{
	if(GetCmdArgs() < 1)
	{
		ReplyToCommand(client, "ltbots_debug_create_sniper_spot <team>");
		return Plugin_Handled;
	}

	if(!IsValidClient(ServerHostClient))
		return Plugin_Handled;
	
	char arg[12];
	GetCmdArg(1, arg, sizeof(arg));
	
	float viewCoords[3];
	GetCoordsOfClientView(ServerHostClient, viewCoords);
	
	int sniperEntity = CreateEntityByName("func_tfbot_hint");
	if( StringToInt(arg) == 2 )
	{
		DispatchKeyValue(sniperEntity, "teamnum", "2");
		Halo(ServerHostClient, viewCoords, 10.0, 255, 0, 0);
	}
	else if( StringToInt(arg) == 3 )
	{
		DispatchKeyValue(sniperEntity, "teamnum", "3");
		Halo(ServerHostClient, viewCoords, 10.0, 0, 0, 255);
	}
	else
	{
		DispatchKeyValue(sniperEntity, "teamnum", "0");
		Halo(ServerHostClient, viewCoords, 10.0, 255, 255, 255);
	}
	
	TeleportEntity(sniperEntity, viewCoords, NULL_VECTOR, NULL_VECTOR);
	
	return Plugin_Handled;
}

public Action command_teleportTeam(int client, int args)
{
	if(GetCmdArgs() < 2)
	{
		ReplyToCommand(client, "ltbots_debug_teleport_team <team> <view or abs coords>");
		return Plugin_Handled;
	}
	
	char arg[12];
	GetCmdArg(1, arg, sizeof(arg));
	
	if(!IsValidClient(ServerHostClient))
		return Plugin_Handled;
		
	float viewCoords[3];
	if( StringToInt(arg) != 1 )
		GetCoordsOfClientView(ServerHostClient, viewCoords);
	else
	{
		GetClientEyePosition(ServerHostClient, viewCoords);
		viewCoords[2] -= 100.0;
	}
	viewCoords[2] += 100.0;
	
	for(int i=1;i<=MaxClients;i++)
	{
		if(!IsValidClient(i) || !IsPlayerAlive(i))
			continue;
		if( GetClientTeam(i) != StringToInt(arg) )
			continue;
			
		TeleportEntity(i, viewCoords, NULL_VECTOR, NULL_VECTOR);
	}
	return Plugin_Handled;
}

int TF2_GetClientCount(int team=-1, bool aliveOnly=false)
{
	int result = 0;
	for(int i = 1; i <= MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		if(IsClientObserver(i) || IsClientSourceTV(i) || IsClientReplay(i))
			continue;
		if(aliveOnly && !IsPlayerAlive(i))
			continue;
		if(team != -1)
		{
			if(team != GetClientTeam(i))
				continue;
		}
		result++;
	}
	return result;
}
// tf2ebots thingy
stock bool NameAlreadyTakenByPlayer(const char[] name)
{
	char buffer[MAX_NAME_LENGTH];
	for(int i = 1; i <= MaxClients; i++)
	{
		if(!IsValidClient(i))
			continue;
		
		GetClientName(i, buffer, sizeof(buffer));
		
		if(StrEqual(name, buffer))
			return true;
	}
	
	return false;
}

void Bot_DirKeyCheck(float vel[3], int &buttons)
{
	if(buttons & IN_MOVELEFT)
		moveLeft(vel, 400.0);
	if(buttons & IN_MOVERIGHT)
		moveRight(vel, 400.0);
	if(buttons & IN_FORWARD)
		moveForward(vel, 400.0);
	if(buttons & IN_BACK)
		moveBackwards(vel, 400.0);
}

stock int GetBluControlPointCount()
{
	int x;
	int count = 0;
	char ClassName[32];
	for (x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidEntity(x))
			continue;

		if (!HasEntProp(x, Prop_Send, "m_iTeamNum"))
			continue;

		if (GetTeamNumber(x) != 3)
			continue

		bool class = GetEdictClassname(x, ClassName, 32);
		if (!class)
			continue;

		if (StrEqual(ClassName, "team_control_point", false))
			count++;
	}

	return count;
}