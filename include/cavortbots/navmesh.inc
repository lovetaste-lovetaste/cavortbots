// navmesh stuff

public Action Navmesh_Scan(Handle timer)
{
	DoesNavMeshExists = NavMesh_Exists();
	return Plugin_Handled;
}

public Action command_drawpath(int client, int args)
{
	if(GetCmdArgs() < 3)
	{
		ReplyToCommand(client, "cbots_debug_draw_path <start nav area index> <destination nav area index> <time the path drawing lasts>");
		return Plugin_Handled;
	}
	
	char arg[12];
	GetCmdArg(1, arg, sizeof(arg));
	char arg2[12];
	GetCmdArg(2, arg2, sizeof(arg2));
	char arg3[12];
	GetCmdArg(3, arg3, sizeof(arg3));
	
	int startArea = StringToInt(arg);
	if(startArea < 1)
	{
		ReplyToCommand(0, "Invalid area.");
		return Plugin_Handled;
	}
	CNavArea startNav = NavMesh_FindAreaByID(startArea);
	if(startNav == INVALID_NAV_AREA)
	{
		ReplyToCommand(0, "Invalid chosen navmesh.");
		return Plugin_Handled;
	}
	
	int wantedArea = StringToInt(arg2);
	if(wantedArea < 1)
	{
		ReplyToCommand(0, "Invalid area.");
		return Plugin_Handled;
	}
	CNavArea wantedNav = NavMesh_FindAreaByID(wantedArea);
	if(wantedNav == INVALID_NAV_AREA)
	{
		ReplyToCommand(0, "Invalid chosen navmesh.");
		return Plugin_Handled;
	}
	
	float time = StringToFloat(arg3);
	
	Bot_Pathfind( -1, wantedNav, startNav, false, _, false, true, time);
}

bool Bot_Pathfind( int client=-1, CNavArea goalArea, CNavArea startArea, bool reversed=false, ArrayList &buffer=null, bool clearBuffer=true, bool drawHalo=true, float haloTime=0.5 )
{
	if( client != -1 && !IsValidClient(client) )
		return false;
	
	if( clearBuffer )
	{	buffer.Clear();	}
	
	if( goalArea == INVALID_NAV_AREA || startArea == INVALID_NAV_AREA )
		return false;
	
	float endOrigin[3];
	NavMeshArea_GetCenter(startArea, endOrigin);
	
	float startOrigin[3];
	NavMeshArea_GetCenter(goalArea, startOrigin);
	
	// NavMesh_BuildPath(CNavArea startArea, CNavArea goalArea, const float goalPos[3], NavPathCostFunctor costFunctor, any costData=0, CNavArea &closestArea=INVALID_NAV_AREA, float maxPathLength=0.0, float flMaxStepSize=0.0);
	
	bool pathed = true;
	
	if( reversed )
		pathed = NavMesh_BuildPath(goalArea, startArea, endOrigin, NavMeshShortestPathCostWall, client, _, 0.0, 50.0);
	else
		pathed = NavMesh_BuildPath(startArea, goalArea, startOrigin, NavMeshShortestPathCostWall, client, _, 0.0, 50.0);
	
	if( pathed )
	{
		int iTempAreaIndex = -1;
		
		if( reversed )
			iTempAreaIndex = startArea.Index;
		else
			iTempAreaIndex = goalArea.Index;
		
		int iParentAreaIndex = NavMeshArea_GetParent(iTempAreaIndex);
		int iNavDirection;
		float flHalfWidth;
		float flCenterPortal[3];
		float flClosestPoint[3], lastClosest[3];
		NavMeshArea_GetCenter(iTempAreaIndex, lastClosest);
		while(iParentAreaIndex != -1)
		{
			// todo: if indexes are visible in a line, then dont add them
			// removes the filler areas in corriders by only keeping the important ones for turns n shit
			
			float flTempAreaCenter[3];
			float flParentAreaCenter[3];
			NavMeshArea_GetCenter(iTempAreaIndex, flTempAreaCenter);
			NavMeshArea_GetCenter(iParentAreaIndex, flParentAreaCenter);
			iNavDirection = NavMeshArea_ComputeDirection(iTempAreaIndex, flParentAreaCenter);
			NavMeshArea_ComputePortal(iTempAreaIndex, iParentAreaIndex, iNavDirection, flCenterPortal, flHalfWidth);
			NavMeshArea_ComputeClosestPointInPortal(iTempAreaIndex, iParentAreaIndex, iNavDirection, flCenterPortal, flClosestPoint);
			flClosestPoint[2] = NavMeshArea_GetZ(iTempAreaIndex, flClosestPoint);
			iTempAreaIndex = iParentAreaIndex;
			iParentAreaIndex = NavMeshArea_GetParent(iTempAreaIndex);
			//Laser(ServerHostClient, lastClosest, flClosestPoint, 10.0, 255, 100, 50, true, false);
			if( drawHalo )
			{	Halo(ServerHostClient, flClosestPoint, haloTime, 155, 60, 255);	}
			lastClosest = flClosestPoint;
			buffer.Push(iTempAreaIndex);
		}
		
		// if( reversed )
			// buffer.Push(goalArea.Index);
		// else
			// buffer.Push(startArea.Index);
	}
	else
		return false;
	
	return true;
}

// Shortest-path cost function for NavMesh_BuildPath
// edited to have wall collision so the NavMesh_BuildPath properly avoids them
// this also has enemy detection for the bots, which would allow the pathfinding to avoid places with large amounts of enemies ( the more enemies there, the higher the cost, meaning that the pathfinding will want to take another path with lower costs )
public int NavMeshShortestPathCostWall(CNavArea area, CNavArea from, CNavLadder ladder, any data)
{
	// PrintToServer("NavMeshShortestPathCostWall Ran");
	if (from == INVALID_NAV_AREA)
	{
		return 0;
	}
	else
	{
		int iDist = 0;
		int iCost = from.CostSoFar;
		if (ladder != INVALID_NAV_LADDER)
		{
			iDist = RoundFloat(ladder.Length);
		}
		else
		{
			// float ex[3], exLow[3];
			// NavMeshArea_GetExtentLow(area, exLow);
			// NavMeshArea_GetExtentHigh(area, ex);
			
			float flAreaCenter[3]; float flFromAreaCenter[3];
			area.GetCenter(flAreaCenter);
			from.GetCenter(flFromAreaCenter);
			iDist = RoundFloat(GetVectorDistance(flAreaCenter, flFromAreaCenter));
		}
		iCost += iDist;
		
		int iAreaFlags = area.Attributes;
		if (iAreaFlags & NAV_MESH_CROUCH) iCost += 20;
		if (iAreaFlags & NAV_MESH_JUMP) iCost += (5 * iDist);
		return iCost;
	}
}