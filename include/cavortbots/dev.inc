// debug functions brah!!!

float DebugInfoRefreshTime;

void DebugInfo()
{
	if(ServerHostClient != -1)
	{
		if(!IsValidClient(ServerHostClient))
			return; // just in case
			
		int edict = GetEntPropEnt(ServerHostClient, Prop_Send, "m_hObserverTarget");
		if(!IsClientObserver(ServerHostClient))
		{
			if(cavort_settings_human_bots.IntValue == 1 && IsPlayerAlive(ServerHostClient))
				edict = ServerHostClient;
			else	
				edict = -1; // fixes a bug with hud text not going away when you switch off spectator without switching off your spec target
		}
		if (isBot(edict))
		{
			SetHudTextParams(0.0, 0.25, 0.105, 255, 255, 255, 255, 1, 0.0, 0.0, 0.0);
			if(IsPlayerAlive(edict))
			{
				char targetName[MAX_NAME_LENGTH] = "None";
				if(IsValidClient(g_BotData[edict].Target))
				{
					GetClientName(g_BotData[edict].Target, targetName, sizeof(targetName));
				}
				char botState[32] = "Normal";
				switch( g_BotData[edict].state )
				{
					case STATE_DISADVANTAGE:
						botState = "Disadvantage";
					case STATE_BIG_DISADVANTAGE:
						botState = "Big disadvantage";
					case STATE_FATAL_DISADVANTAGE:
						botState = "Fatal disadvantage";
					case STATE_FATAL_SACRIFICE:
						botState = "Sacrificing itself!";
						
					case STATE_ADVANTAGE:
						botState = "Advantage";
					case STATE_BIG_ADVANTAGE:
						botState = "Big advantage";
					case STATE_LARGE_ADVANTAGE:
						botState = "Massive advantage";
					case STATE_AGGRO_ADVANTAGE:
						botState = "Rip and tear!";	
				}
				
				ShowHudText( ServerHostClient, -1, "Bot Name: %N \nTarget: %s \n State: %s \n", edict, targetName, botState);
			}
			else
				ShowHudText( ServerHostClient, -1, "Bot Name: %N \nDead!", edict);
		}
	}
}