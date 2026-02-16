// mge support
void CheckMGEArena(int client)
{
	if(!IsValidClient(client))
		return;
			
	// as of right now this is bugged, causing crashes sometimes
	// i do not know why :(
	
	TFTeam team = TF2_GetClientTeam(client);

	if( team == TFTeam_Spectator || team == TFTeam_Unassigned )
	{
		switch( (client % 4) + 1 )
		{
			case 1:
			{
				if(cavort_settings_debug_enable_text.IntValue == 1)
					PrintToServer("%N is not in a MGE arena! Choosing Endif.", client);
					
				FakeClientCommandThrottled(client, "add endif");
			}
			case 2:
			{
				if(cavort_settings_debug_enable_text.IntValue == 1)
					PrintToServer("%N is not in a MGE arena! Choosing Ammomod.", client);
					
				FakeClientCommandThrottled(client, "add ammomod");
			}
			case 3:
			{
				if(cavort_settings_debug_enable_text.IntValue == 1)
					PrintToServer("%N is not in a MGE arena! Choosing Ammomod MGE.", client);
					
				FakeClientCommandThrottled(client, "add 15");
			}
			case 4:
			{
				if(cavort_settings_debug_enable_text.IntValue == 1)
					PrintToServer("%N is not in a MGE arena! Choosing No Splash.", client);
					
				FakeClientCommandThrottled(client, "add 16");
			}
		}
	}
}
