/*
	CREDITS:
	A LOT OF CREDIT TO TF2EBOTS! this variant of my bots is doing the same thing TF2Ebots was already doing, and a lot of the ideas implemented here was from it. go look at it brah!!!
	a bunch of other people on the forums
*/


#include <sourcemod>
#include <sdkhooks>
#include <sdktools_trace>
#include <tf2>
#include <tf2_stocks>
#include <navmesh>
#define TF_MAXPLAYERS MAXPLAYERS + 1	// change this if you need to. unless you somehow have more bots than your max players, it should be fine
#define POSITIVE_INFINITY		view_as<float>(0x7F800000)

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_VERSION "1.0"

ConVar 
	cavort_settings_enabled,
	cavort_settings_force_enable,
	cavort_settings_human_bots,
	cavort_settings_bots_ignore_humans,
	cavort_settings_bots_friendly_with_humans,
	cavort_settings_debug_enable_text,
	cavort_settings_debug_enable;

bool NextBotClientIsCavortBot;
bool DoesNavMeshExists;

Handle g_hLookupBone;
Handle g_hGetBonePosition;
bool BonePosAvaliable;

float maxStepSize;
int g_iResourceEntity;

char currentMap[PLATFORM_MAX_PATH];

#include <cavortbots/initialize>
#include <cavortbots/utilities>
#include <cavortbots/bot_utilities>
#include <cavortbots/base>
#include <cavortbots/base_lite>
#include <cavortbots/dev>
#include <cavortbots/navmesh>
#include <cavortbots/gamemodes>
#include <cavortbots/bonus_gamemodes>
#include <cavortbots/squad>

public Plugin myinfo =
{
	name = "Cavortbots",
	author = "weyouthey",
	description = "The third generation in my series of bots made for TF2. Based off of TF2Ebots.",
	version = PLUGIN_VERSION,
	url = "https://lovetaste.neocities.org/"
}

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
	if (GetEngineVersion() != Engine_TF2)
	{
		Format(error, err_max, "This plugin only works for Team Fortress 2.");
		return APLRes_Failure;
	}

	return APLRes_Success;
}

public void OnPluginStart()
{
	cavort_settings_enabled = CreateConVar("cbots_enabled", "1", "Toggles if Cavortbots is enabled.", _, true, 0.0, true, 1.0);
	cavort_settings_debug_enable = CreateConVar("cbots_debug_enabled", "0", "Enables debugging info.", FCVAR_CHEAT, true, 0.0, true, 1.0);
	cavort_settings_force_enable = CreateConVar("cbots_debug_force_bots_enabled", "0", "Forces all bots to be Cavortbots. Causes problems with TFBots.", FCVAR_CHEAT, true, 0.0, true, 1.0);
	// the reason why this isnt optimal is due to the TFBots having their own logic. Mainly for plugin debugging due to plugin refreshes turning off active Cavortbots
	cavort_settings_debug_enable_text = CreateConVar("cbots_debug_enabled_text", "0", "If this is set to one, then the bot will broadcast.", FCVAR_CHEAT, true, 0.0, true, 1.0);
	cavort_settings_human_bots = CreateConVar("cbots_humans_are_bots", "0", "Turns humans into bots.", _, true, 0.0, true, 1.0);
	cavort_settings_bots_ignore_humans = CreateConVar("cbots_bots_only_target_bots", "0", "Toggles if bots ignore humans.", _, true, 0.0, true, 1.0);
	cavort_settings_bots_friendly_with_humans = CreateConVar("cbots_bots_friendly_with_humans", "0", "Toggles if bots are always friendly with humans. If set to 2, the bot will treat humans as teammates. ", _, true, 0.0, true, 2.0);
	RegConsoleCmd("cbots_toggle_specific_bot_byName", command_toggleSpecificBotByName, "Toggles whether or not the plugin effect a specific client. Uses the name of said client.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_enable_all_bots", command_EnableAll, "Enables all bot brains.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_disable_all_bots", command_DisableAll, "Disables all bot brains.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_enable_all_team", command_EnableAllTeam, "Enables all bot brains on a certain team.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_disable_all_team", command_DisableAllTeam, "Disables all bot brains on a certain team.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_check_Status_bot_byName", command_CheckStatusOfBotByName, "Toggles whether or not the plugin effect a specific client. Uses the name of said client.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_debug_teleport_team", command_teleportTeam, "Teleports a select team to the server host client's viewing coordinates, or raw coordinates.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_debug_create_sniper_spot", command_CreateSniperSpot, "Makes a sniper spot for the bots.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_debug_draw_path", command_drawpath, "Draws a path.", FCVAR_CHEAT);
	RegConsoleCmd("cbots_addbot", AddCavortBot, "Adds and enables a Cavortbot.");
	
	CreateTimer(1.0, FindServerHost, _, TIMER_REPEAT);
	CreateTimer(1.0, Navmesh_Scan, _, TIMER_REPEAT);
	PrintToChatAll("\x04[lovelybots] \x01Reloaded/Started the plugin!");
	
	for(int i = 1; i < TF_MAXPLAYERS; i++)
	{
		g_BotData[i].Enabled = false;
		// this is important for the bot commands to work.
		// this HOPEFULLY allows normal TFBots to be spawned seperately from Cavortbots, while also allowing the puppet bots to stay
		// this should make the cbot commands the only command that enables the customizations
	}
	NextBotClientIsCavortBot = false;
}

public void OnMapStart()
{
	for(int i = 1; i < TF_MAXPLAYERS; i++)
	{
		g_BotData[i].Enabled = false;
	}
	NextBotClientIsCavortBot = false;

	g_iResourceEntity = GetPlayerResourceEntity();

	BonePosAvaliable = true;
	Handle hGameConf = LoadGameConfigFile("aimbot.games");
	if (hGameConf == INVALID_HANDLE)
	{
		BonePosAvaliable = false;
		LogMessage("Could not locate gamedata file, bot headshot aim wont be as accurate!");
	}
	
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hGameConf, SDKConf_Signature, "CBaseAnimating::LookupBone");
	PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
	PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
	if(!(g_hLookupBone=EndPrepSDKCall()))
	{
		BonePosAvaliable = false;
		LogMessage("Could not initialize SDK call CBaseAnimating::LookupBone");
	}
	
	StartPrepSDKCall(SDKCall_Entity);
	PrepSDKCall_SetFromConf(hGameConf, SDKConf_Signature, "CBaseAnimating::GetBonePosition");
	PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
	PrepSDKCall_AddParameter(SDKType_Vector, SDKPass_ByRef, _, VENCODE_FLAG_COPYBACK);
	PrepSDKCall_AddParameter(SDKType_QAngle, SDKPass_ByRef, _, VENCODE_FLAG_COPYBACK);
	if(!(g_hGetBonePosition=EndPrepSDKCall()))
	{
		BonePosAvaliable = false;
		LogMessage("Could not initialize SDK call CBaseAnimating::GetBonePosition");
	}
	
	ServerCommand("tf_bot_join_after_player %i", 0);
	// this allows bots to join before the player picks a team. I added this because having the bots join after the player makes it feel unrealistic. Normal people don't usually wait until someone else loads in, so why should bots?	
	
	ServerCommand("tf_bot_spawn_use_preset_roster %i", 0);
	// this command causes a bug which causes bots to not spawn in when there is 22 or more, so this disables it
	// bots change class dynamically now due to the plugin as well :)
	
	ServerCommand("mp_autoteambalance %i", 0);
	ServerCommand("mp_teams_unbalance_limit %i", 0);
	//disables autobalance and not being able to change teams when there is an imbalance. Mainly so bot spawning isn't annoying
	
	ServerCommand("tf_bot_quota_mode normal");
	//makes the bot quota be based on bots specifically
	
	ServerCommand("tf_bot_keep_class_after_death %i", 1);
	ServerCommand("tf_bot_reevaluate_class_in_spawnroom %i", 0);
	//prevents the bots from changing class by themselves
}

public Action AddCavortBot(int client, int args)
{
	char arg[12];
	GetCmdArg(1, arg, sizeof(arg));
	
	NextBotClientIsCavortBot = true;
	
	// TFTeam_Red = 2
	// TFTeam_Blue = 3
	
	char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "configs/CavortbotsNames.txt");
    File fp = OpenFile(filepath, "r");
	char ChosenName[MAX_NAME_LENGTH];

	ArrayList BotNames = new ArrayList(ByteCountToCells(65));
    if (fp != null)
    {
		char line[MAX_NAME_LENGTH];
        while (!fp.EndOfFile())
		{
			fp.ReadLine(line, sizeof(line));
			if (!line[0])
				continue;

			if (StrContains(line, "//") != -1)
				continue;

			TrimString(line);
			if (NameAlreadyTakenByPlayer(line))
				continue;

			BotNames.PushString(line);
		}

		fp.Close();
    }
	
	if (BotNames.Length > 0)
		BotNames.GetString(GetRandomInt(0, BotNames.Length - 1), ChosenName, sizeof(ChosenName));
	else
		FormatEx(ChosenName, sizeof(ChosenName), "%s", cavortnames[GetRandomInt(0,8)]);
		// fallback names that are hardcoded
	
	ConVar cheats = FindConVar("sv_cheats");
	if (cheats != null)
	{
		if (cheats.BoolValue)
		{
			if(GetTeamClientCount(TFTeam_Blue) >= GetTeamClientCount(TFTeam_Blue))
				ServerCommand("bot -name %s -team red -class %s", ChosenName, arg);
			else
				ServerCommand("bot -name %s -team blue -class %s", ChosenName, arg);
		}
		else
		{				
			int flags = cheats.Flags;
			flags &= ~FCVAR_NOTIFY;
			cheats.Flags = flags;
			cheats.SetBool(true, false, false);
			
			if(GetTeamClientCount(TFTeam_Blue) >= GetTeamClientCount(TFTeam_Blue))
				ServerCommand("bot -name %s -team red -class %s", ChosenName, arg);
			else
				ServerCommand("bot -name %s -team blue -class %s", ChosenName, arg);
				
			CreateTimer(GetGameFrameTime() * 2.0, SetCheats);
		}
	}
	return Plugin_Handled;
}

public void OnClientPutInServer(int client)
{
	if(NextBotClientIsCavortBot && IsFakeClient(client))
	{
		g_BotData[client].Enabled = true;
		LogMessage("%N has connected.", client);
	}
	
	NextBotClientIsCavortBot = false;
	
	Bot_Initialize(client);
}

public void OnClientDisconnect_Post(int client)
{
	g_BotData[client].Enabled = false;
	// prevents other bots that use the same index from keeping the status
	NextBotClientIsCavortBot = false;
	// prevents the next bot from becoming a Cavortbot if not spawned with the command
	// this may interfere if you spawn bots with quotas, buuuut it should be fine considering how tight the window is for this scenario to happen	
}

public void OnGameFrame()
{
	if(cavort_settings_debug_enable.IntValue == 1) // BoolValue is weird for me sometimes
	{
		if(DebugInfoRefreshTime <= GetGameTime() || DebugInfoRefreshTime > GetGameTime() + 10.0) // has a fallback for map changes due to a occasional bug, so it resets
		{
			DebugInfoRefreshTime = GetGameTime() + 0.1;
			DebugInfo();
		}
	}
}

public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon)
{
	if(IsFakeClient(client))
	{
		int activeWeapon = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
		char weaponName[128];
		weaponName = TF2_GetWeaponClassName(activeWeapon);
		if( StrContains(weaponName, "tf_weapon_lunchbox", false) != -1 && !TF2_IsPlayerInCondition(client, TFCond_Taunting) )
		{
			if(GetHealth(client) >= 290)
				SetEntityHealth(client, 290);
		}
	}
	
	if(isBot(client))
	{
		int decisionThink = 0;
		if(GetGameTime() >= (g_BotData[client].LastTimeReacted + g_BotData[client].BaseReactionTime))
		{
			decisionThink = 1;
		}
		
		Bot_Brain(client, decisionThink);
		
		if(g_BotData[client].WantedButtons != 0)
		{
			buttons |= g_BotData[client].WantedButtons; 
			g_BotData[client].WantedButtons = 0;
		}
		
		Bot_DirKeyCheck(vel, buttons);
		// bots DO NOT move with the movement buttons ( IN_LEFT etc )
		// ( not IN_JUMP or IN_DUCK, those work fine )
		// this is probably due to the fact that OnPlayerRunCmd is already based of the movement buttons
		// this stimulates the movements so it works
		// ( maybe use OnPlayerRunCmdPost ? that probably wont do anything though )
		// ( if only we had smth like OnPlayerRunCmdPre... )
	}
	
	ClampAngle(angles);
}

public Action SetCheats(Handle timer)
{
	ConVar cheats = FindConVar("sv_cheats");
	if (cheats != null)
	{
		cheats.SetBool(false, false, false);
		int flags = cheats.Flags;
		flags |= FCVAR_NOTIFY;
		cheats.Flags = flags;
	}
	return Plugin_Handled;
}