#include <sourcemod>
#include <sdktools>
#include <clientprefs>
#include <tf2_stocks>
#include <morecolors>
Database KSYS_Database;

//Registry of all playable classes
char ClassDefs[][32] = {
  "scout",
  "sniper",
  "soldier",
  "demoman",
  "medic",
  "heavy",
  "pyro",
  "spy",
  "engineer",
  ""
};

enum struct PLAYER_SOUNDS {
    int killcount;
    char sound[256];
    char sound_old[256];
}
PLAYER_SOUNDS PD[MAXPLAYERS+1];

enum struct TEMP_SOUNDS {
  char sound[256];
}
TEMP_SOUNDS TEMPSNDS[MAXPLAYERS+1][10];
stock bool IsValidClient(int client) {
  return (0 < client <= MaxClients && IsClientInGame(client) && !IsFakeClient(client));
}

char PLUGIN_VERSION[8] = "1.0.0";

public Plugin myinfo = {
  name = "Fartsy's Kill Streak System",
  author = "Fartsy",
  description = "Allows users to listen to music on kill streaks",
  version = PLUGIN_VERSION,
  url = "https://forums.firehostredux.com"
};

public void OnPluginStart(){
    HookEvent("player_death", EventDeath);
    HookEvent("player_spawn", EventSpawn);
    RegConsoleCmd("sm_ksys", Command_SetKSYS, "Set your killstreak sounds");
}

//Connect to database
public void OnConfigsExecuted() {
  if (!KSYS_Database) Database.Connect(Database_OnConnect, "ksys");
}

//Format database if needed
public void Database_OnConnect(Database db, char[] error, any data) {
  if (!db) {
    LogError(error);
    return;
  }
  char buffer[64];
  db.Driver.GetIdentifier(buffer, sizeof(buffer));
  if (!StrEqual(buffer, "mysql", false)) {
    delete db;
    LogError("Could not connect to the database: expected mysql database.");
    return;
  }
  KSYS_Database = db;
  KSYS_Database.Query(Database_FastQuery, "CREATE TABLE IF NOT EXISTS killstreak_sounds(steamid INT UNSIGNED, scout TEXT DEFAULT 'null', soldier TEXT DEFAULT 'null', pyro TEXT DEFAULT 'null', demoman TEXT DEFAULT 'null', heavy TEXT DEFAULT 'null', engineer TEXT DEFAULT 'null', medic TEXT DEFAULT 'null', sniper TEXT DEFAULT 'null', spy TEXT DEFAULT 'null', streak INT UNSIGNED DEFAULT '5', multiFile INT UNSIGNED DEFAULT '0', PRIMARY KEY (steamid, streak));");
}

//Database Fastquery Manager - Writing only
void Database_FastQuery(Database db, DBResultSet results, const char[] error, any data) {
  if (!results) LogError("Failed to query database: %s", error);
}
public void Database_MergeDataError(Database db, any data, int numQueries,
  const char[] error, int failIndex, any[] queryData) {
  LogError("Failed to query database (transaction): %s", error);
}

//Set up thing with thing
public Action Command_SetKSYS(int client, int args){
    char class[16]; char sound[256]; char streak[8];
    GetCmdArg(1, class, sizeof(class));
    GetCmdArg(2, sound, sizeof(sound));
    GetCmdArg(3, streak, sizeof(streak));
    int steamID = GetSteamAccountID(client);
    if (!steamID || steamID <= 10000) return Plugin_Handled;
    else {
      if (!KSYS_Database) {
          LogError("NO DATABASE");
          return Plugin_Handled;
      }
      char query[1024];
      Format(query, sizeof(query), "INSERT INTO killstreak_sounds (steamid, %s, streak) VALUES ('%d', '%s', '%i') ON DUPLICATE KEY UPDATE %s = '%s';", class, steamID, sound, StringToInt(streak), class, sound);
      KSYS_Database.Query(Database_FastQuery, query);
      CPrintToChat(client, "{orange}[{white}CORE{orange}] {white} You set {limegreen}%s{white} to play upon obtaining a {limegreen}%s{white} kill streak as {limegreen} %s{white} .", sound, streak, class);
    }
    return Plugin_Handled;
}

//Do Killstreak Sounds
public Action EventDeath(Event Spawn_Event, const char[] Spawn_Name, bool Spawn_Broadcast) {
  int client = GetClientOfUserId(Spawn_Event.GetInt("userid"));
  int attacker = GetClientOfUserId(Spawn_Event.GetInt("attacker"));
  int steamid = GetSteamAccountID(attacker);
  if (IsValidClient(client)){
    ResetSound(client, true);
    PD[client].killcount = 0;
  }
  if (steamid && client != attacker && IsValidClient(attacker)){
    PD[attacker].killcount++;
    for (int i = 5; i <= 50; i = i+5){
      if(i == PD[attacker].killcount){
        CPrintToChatAll("{limegreen}%N{white} has a {darkred}%i{white} killstreak!", attacker, PD[attacker].killcount);
        char query[1024];
        Format(query, sizeof(query), "SELECT %s, streak FROM killstreak_sounds WHERE steamID = '%d' AND streak = '%i';", ClassDefs[GetEntProp(attacker, Prop_Send, "m_iClass") - 1], steamid, i);
        KSYS_Database.Query(KSYS_Query, query, attacker);
      }
    }
  }
  return Plugin_Continue;
}

public Action EventSpawn(Event Spawn_Event, const char[] Spawn_Name, bool Spawn_Broadcast) {
  int client = GetClientOfUserId(Spawn_Event.GetInt("userid"));
  if (IsValidClient(client)) PD[client].killcount = 0;
  return Plugin_Continue;
}

void KSYS_Query(Database db, DBResultSet results, const char[] error, int client) {
  if (!results) return;
  if (results.FetchRow()) {
    results.FetchString(0, PD[client].sound, 256);
    if (strcmp(PD[client].sound, "null") == 0 || strcmp(PD[client].sound, "") == 0){
      PrintToConsole(client, "[Fartsy's Killstreak System] SOUND %s WAS NULL. You may add your own music using sm_ksys class path/file.mp3 killstreakNumber.", PD[client].sound);
      return;
    }
    //It's multi file, load ALL the things!
    if (StrContains(PD[client].sound, "_multi") != -1){
      char query[512];
      Format(query, sizeof(query), "SELECT %s, streak FROM killstreak_sounds WHERE steamID = '%d' AND %s LIKE '%%_multi%%';", ClassDefs[GetEntProp(client, Prop_Send, "m_iClass") - 1], GetSteamAccountID(client), ClassDefs[GetEntProp(client, Prop_Send, "m_iClass") - 1]);
      KSYS_Database.Query(KSYS_Query_Multi, query, client);
    }
    //It's single file, load the thing.
    else {
      PrintToConsole(client, "[Fartsy's Killstreak System] Playing %s...", PD[client].sound);
      InjectAndLoadSound(PD[client].sound);
      ResetSound(client, false);
      CSEClient(client, PD[client].sound, 100, 0, 1.0, 100);
      strcopy(PD[client].sound_old, 256, PD[client].sound);
    }
  }
}

void KSYS_Query_Multi(Database db, DBResultSet results, const char[] error, int client){
  if (!results) return;
  int i = 0;
  while (results.FetchRow()){
    results.FetchString(0, TEMPSNDS[client][i].sound, 256);
    InjectAndLoadSound(TEMPSNDS[client][i].sound);
    PrintToConsole(client, "[Fartsy's Killstreak System] Loading file %s!", TEMPSNDS[client][i].sound);
    ++i;
  }
  switch(PD[client].killcount){
    case 5:{
      if (strcmp(TEMPSNDS[client][0].sound, "null") == 0 || strcmp(TEMPSNDS[client][0].sound, "") == 0){
        return;
      }
      CSEClient(client, TEMPSNDS[client][0].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][1].sound, 100, 1, 0.05, 100);
      CSEClient(client, TEMPSNDS[client][2].sound, 100, 1, 0.05, 100);
      CSEClient(client, TEMPSNDS[client][3].sound, 100, 1, 0.05, 100);
      CSEClient(client, TEMPSNDS[client][4].sound, 100, 1, 0.05, 100);
      strcopy(PD[client].sound_old, 256, TEMPSNDS[client][0].sound);
    }
    case 10:{
      if (strcmp(TEMPSNDS[client][1].sound, "null") == 0 || strcmp(TEMPSNDS[client][1].sound, "") == 0) return;
      CSEClient(client, TEMPSNDS[client][0].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][1].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][2].sound, 100, 1, 0.05, 100);
      CSEClient(client, TEMPSNDS[client][3].sound, 100, 1, 0.05, 100);
      CSEClient(client, TEMPSNDS[client][4].sound, 100, 1, 0.05, 100);
      strcopy(PD[client].sound_old, 256, TEMPSNDS[client][1].sound);
    }
    case 15:{
      if (strcmp(TEMPSNDS[client][2].sound, "null") == 0 || strcmp(TEMPSNDS[client][2].sound, "") == 0) return;
      CSEClient(client, TEMPSNDS[client][0].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][1].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][2].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][3].sound, 100, 1, 0.05, 100);
      CSEClient(client, TEMPSNDS[client][4].sound, 100, 1, 0.05, 100);
      strcopy(PD[client].sound_old, 256, TEMPSNDS[client][2].sound);
    }
    case 20:{
      if (strcmp(TEMPSNDS[client][3].sound, "null") == 0 || strcmp(TEMPSNDS[client][3].sound, "") == 0) return;
      CSEClient(client, TEMPSNDS[client][0].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][1].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][2].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][3].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][4].sound, 100, 1, 0.05, 100);
      strcopy(PD[client].sound_old, 256, TEMPSNDS[client][3].sound);
    }
    case 25:{
      if (strcmp(TEMPSNDS[client][4].sound, "null") == 0 || strcmp(TEMPSNDS[client][4].sound, "") == 0) return;
      CSEClient(client, TEMPSNDS[client][0].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][1].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][2].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][3].sound, 100, 1, 1.0, 100);
      CSEClient(client, TEMPSNDS[client][4].sound, 100, 1, 1.0, 100);
      strcopy(PD[client].sound_old, 256, TEMPSNDS[client][4].sound);
    }
  }
}

void InjectAndLoadSound(const char[] sound){
  PrecacheSound(sound, true);
}

//Play sound to client. Ripped straight from potato. AGAIN. Allows us to play sounds directly to people when they join.
void CSEClient(int client, char[] sndName, int TSNDLVL, int flags, float vol, int pitch) {
  EmitSoundToClient(client, sndName, _, 6, TSNDLVL, flags, vol, pitch, _, _, _, _, _);
}

void ResetSound(int client, bool override){
  if (strcmp(PD[client].sound_old, "") != 0 || strcmp(PD[client].sound_old, PD[client].sound) != 0 || override){
    for (int i = 0; i < 10; i++){
      StopSound(client, i, PD[client].sound_old);
      if (strcmp(TEMPSNDS[client][i].sound, "") != 0) for (int x = 0; x < 10; x++) StopSound(client, x, TEMPSNDS[client][i].sound);
    }
    PrintToServer("[Fartsy's Killstreak System] Stopping %s for %N", PD[client].sound_old, client);
    Format (PD[client].sound_old, sizeof(PD[client].sound_old), "");
  } 
}
