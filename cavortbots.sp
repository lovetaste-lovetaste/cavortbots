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

ConVar settings_enabled;

Handle g_hLookupBone;
Handle g_hGetBonePosition;
bool BonePosAvaliable;

enum struct BotData
{
}

BotData g_BotData[TF_MAXPLAYERS];

public Plugin myinfo =
{
	name = "Cavortbots",
	author = "weyouthey",
	description = "The third generation in my series of bots made for TF2.",
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
	settings_enabled = CreateConVar("cbots_enabled", "1", "Toggles if Cavortbots is enabled.", _, true, 0.0, true, 1.0);
}

public void OnMapStart()
{
	BonePosAvaliable = true;
	Handle hGameConf = LoadGameConfigFile("aimbot.games");
	if (hGameConf == INVALID_HANDLE)
	{
		BonePosAvaliable = false;
		LogMessage("Could not locate gamedata file, bot aim may be a bit weird!");
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
}