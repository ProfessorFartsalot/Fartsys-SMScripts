// Hooks all events used for the plugin
public void HookAllEvents() {
  AssLogger(1, "Hooking game events...");
  HookEvent("player_death", EventDeath);
  HookEvent("player_spawn", EventSpawn);
  HookEvent("server_cvar", Event_Cvar, EventHookMode_Pre);
  HookEvent("mvm_begin_wave", OnWaveBegin);
  HookEvent("mvm_wave_complete", EventWaveComplete);
  HookEvent("mvm_wave_failed", EventWaveFailed);
  HookEvent("mvm_bomb_alarm_triggered", EventWarning);
  HookEventEx("player_hurt", Event_PlayerHurt, EventHookMode_Pre);
  HookEventEx("player_healed", Event_PlayerHealed, EventHookMode_Pre);
}

//Silence cvar changes to minimize chat spam.
public Action Event_Cvar(Event event, const char[] name, bool dontBroadcast) {
  event.BroadcastDisabled = true;
  return Plugin_Handled;
}

//Sync client stats when they leave
public void OnClientDisconnect(int client) {
  GlobalAudio.OnClientDisconnect(client);
  int steamID = GetSteamAccountID(client);
  if (steamID) { EmnityManager[client].Clear(); }
  if (!Ass_Database || !steamID || steamID <= 10000) return;
  char query[256]; char clientName[128];
  GetClientInfo(client, "name", clientName, 128);
  Format(query, sizeof(query), "INSERT INTO ass_activity (name, steamid, date, seconds) VALUES ('%s', %d, CURRENT_DATE, %d) ON DUPLICATE KEY UPDATE name = '%s', seconds = seconds + VALUES(seconds);", clientName, steamID, GetClientMapTime(client), clientName);
  Ass_Database.Query(Database_FastQuery, query);
  if (GetClientCount(false) == 0) { CreateTimer(600.0, TimedOperator, 99999); }
}

//Check who spawned and log their class
public void EventSpawn(Event Spawn_Event, const char[] Spawn_Name, bool Spawn_Broadcast) {
  int client = GetClientOfUserId(Spawn_Event.GetInt("userid"));
  if (IsValidClient(client)) {
    int class = Spawn_Event.GetInt("class");
    AudioManager[client].OnClientRespawned();
    EmnityManager[client].OnSpawned(client, class);
  }
}

//Log Damage!
public void Event_PlayerHurt(Handle event, const char[] name, bool dontBroadcast) {
  int client = GetClientOfUserId(GetEventInt(event, "userid"));
  int attacker = GetClientOfUserId(GetEventInt(event, "attacker"));
  int damage = GetEventInt(event, "damageamount");
  if (IsValidClient(attacker) && attacker != client) {
    EmnityManager[attacker].Add(attacker, 0, damage);
  }
}

// Log Healing
public void Event_PlayerHealed(Handle event, const char[] name, bool dontBroadcast) {
  char client[32];
  int healer = GetClientOfUserId(GetEventInt(event, "healer"));
  int amount = GetEventInt(event, "amount");
  if (IsValidClient(GetClientOfUserId(GetEventInt(event, "patient")))) Format(client, sizeof(client), "%N", GetClientOfUserId(GetEventInt(event, "patient")));
  if (IsValidClient(healer)) EmnityManager[healer].Add(healer, 1, amount, client);
}

//Check who died by what and announce it to chat if need be.
public void EventDeath(Event Spawn_Event, const char[] Spawn_Name, bool Spawn_Broadcast) {
  int client = GetClientOfUserId(Spawn_Event.GetInt("userid"));
  if (client <= 0 || client > MaxClients || !IsClientInGame(client)) return; // Return early in the event the client that died does not exist at all
  if (GetSteamAccountID(client) != 0) { EmnityManager[client].Clear(); }
  // Get Attacker
  int attacker = GetClientOfUserId(Spawn_Event.GetInt("attacker"));
  // Get Weapon
  char weapon[32];
  Spawn_Event.GetString("weapon", weapon, sizeof(weapon));
  // Set up damagebits
  int damagebits = Spawn_Event.GetInt("damagebits");
  if (!attacker) { SendDeathLog(attacker, client, damagebits, weapon); }
}

// When a wave starts. Defeats the need for operator 2 in most cases.
public void OnWaveBegin(Event bgEvent, const char[] bgName, bool bgBroadcast) {
  WaveSystem().run();
  return;
}

//When we win
public Action EventWaveComplete(Event Spawn_Event, const char[] Spawn_Name, bool Spawn_Broadcast) {
  sudo(300);
  CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}You've defeated the wave!");
  return Plugin_Continue;
}

//Announce when we are in danger.
public Action EventWarning(Event Spawn_Event, const char[] Spawn_Name, bool Spawn_Broadcast) {
  if (core.doFailsafe) {
    core.failsafeCount++;
    CPrintToChatAll("%s Counter: %i", failsafe[GetRandomInt(0, sizeof(failsafe) - 1)], core.failsafeCount);
    EmitSoundToAll(SFXArray[55].realPath);
    PerformWaveFailsafe(GetRandomInt(0,1));
    core.doFailsafe = false;
  } else CPrintToChatAll("{darkviolet}[{red}WARN{darkviolet}] {darkred}PROFESSOR'S ASS IS ABOUT TO BE DEPLOYED!!!");
  return Plugin_Handled;
}

//When the wave fails
public Action EventWaveFailed(Event Spawn_Event, const char[] Spawn_Name, bool Spawn_Broadcast) {
  CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}Wave {red}failed {white}successfully!");
  sudo(300);
  return Plugin_Continue;
}

//Sacrifice target and grant bonus points
public Action SacrificeClient(int client, int attacker, bool wasBombReset) {
  if (attacker <= MaxClients && IsClientInGame(attacker)) {
    core.bombReset = false;
    ass_sacPoints += wasBombReset ? 5 : 1;
    core.sacrificedByClient = false;
    int steamID = GetSteamAccountID(attacker);
    if (!Ass_Database || !steamID) return Plugin_Handled;
    char query[256];
    CPrintToChatAll(wasBombReset ? "{darkviolet}[{forestgreen}CORE{darkviolet}] {white}Client {red}%N {white}has reset the ass! ({limegreen}+5 pts{white})" : "{darkviolet}[{forestgreen}CORE{darkviolet}] {white}Client {red}%N {white}has sacrificed {blue}%N {white}for the ass! ({limegreen}+1 pt{white})", attacker, client);
    Format(query, sizeof(query), wasBombReset ? "UPDATE ass_activity SET bombsreset = bombsreset + 1, bombsresetsession = bombsresetsession + 1 WHERE steamid = %i;" : "UPDATE ass_activity SET sacrifices = sacrifices + 1, sacrificessession = sacrificessession + 1 WHERE steamid = %i;", steamID);
    Ass_Database.Query(Database_FastQuery, query);
  }
  return Plugin_Handled;
}

//Queue music for new clients, also track their stats.
public void OnClientPostAdminCheck(int client) {
  int steamID = GetSteamAccountID(client);
  if (!steamID || steamID <= 10000) return;
  else {
    if (!Ass_Database) {
      PrintToServer("No database detected, setting soundPreference for %N to default.", client);
      soundPreference[client] = GetConVarInt(cvarSNDDefault);
    }
    if (!core.tickingClientHealth) {
      CreateTimer(1.0, TickClientHealth);
      core.tickingClientHealth = true;
    }
    if(!IsFakeClient(client)) AudioManager[client].OnClientConnected(client);
    char query[1024];
    Format(query, sizeof(query), "INSERT INTO ass_activity (name, steamid, date, damagedealtsession, killssession, deathssession, bombsresetsession, sacrificessession) VALUES ('%N', %d, CURRENT_DATE, 0, 0, 0, 0, 0) ON DUPLICATE KEY UPDATE name = '%N', date = CURRENT_DATE, damagedealtsession = 0, killssession = 0, deathssession = 0, bombsresetsession = 0, sacrificessession = 0;", client, steamID, client);
    Ass_Database.Query(Database_FastQuery, query);
    char queryID[256];
    Format(queryID, sizeof(queryID), "SELECT soundprefs from ass_activity WHERE steamid = '%d';", steamID);
    Ass_Database.Query(SQL_SNDPrefs, queryID, client);
  }
}

// Send a death message for a client
void SendDeathLog(int attacker, int client, int damagebits, const char[] wep) {
  char name[64];
  Format(name, sizeof(name), attacker == 0 ? "[INTENTIONAL GAME DESIGN]" : "%N", IsValidClient(attacker) ? client : attacker);
  char weapon[32];
  strcopy(weapon, sizeof(weapon), wep);
  if (IsValidClient(attacker) && core.sacrificedByClient) SacrificeClient(client, attacker, core.bombReset);
  switch (damagebits) {
    case 1: { //CRUSH
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N was crushed by a {red}FALLING ROCK FROM OUTER SPACE{white}!", client);
      weapon = "Meteor to the Face";
    }
    case 8: { //BURN
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N was {red}MELTED{white}.", client);
      weapon = "Melted by Sharts or Ass Gas";
    }
    case 16: { //FREEZE
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N was flattened out by a {red}TRAIN{white}!", client);
      weapon = "Attempted Train Robbery";
    }
    case 32: { //FALL
      if (WeatherManager.hasTornado) {
        switch (GetClientTeam(client)) {
          case 2: {
            CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N was {red}%s{white}!", client, DeathMessage[GetRandomInt(0, 5)]);
            weapon = "Yeeted into Orbit via Tornado";
          }
          case 3: {
            CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N was {red}%s{white}! ({limegreen}+1 pt{white})", client, DeathMessage[GetRandomInt(0, 5)]);
            ass_sacPoints++;
            CustomSoundEmitter(SFXArray[GetRandomInt(11, 26)].realPath, 65, false, 0, 1.0, 100);
          }
        }
      } else {
        CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N fell to a {red}CLUMSY PAINFUL DEATH{white}!", client);
        weapon = "Tripped on a LEGO";
      }
    }
    case 64: { //BLAST
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N went {red} KABOOM{white}!", client);
      weapon = "Gone Kaboom!";
    }
    case 128: { //CLUB
      if (core.canHindenburg) {
        CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N is {red}CRASHING THE HINDENBURG{white}!!!", client);
        weapon = "Crashing the Hindenburg";
      }
    }
    case 256: { //SHOCK
      if(core.CodeEntry > 0) {
        CPrintToChatAll("{darkviolet}[{red}EXTERMINATUS{darkviolet}] {white}Client %N has humliated themselves with an {red}incorrect {white}key entry!", client);
        weapon = "Failed FB Code Entry";
        int i = GetRandomInt(1, 16);
        switch (i) {
          case 1, 3, 10: {
            FastFire2("BG.Meteorites1", "ForceSpawn", "", 0.0, false);
            CPrintToChatAll("{darkviolet}[{red}WARN{darkviolet}] {white}Uh oh, a {red}METEOR{white}has been spotted coming towards Dovah's Ass!!!");
            FastFire2("bg.meteorite1", "StartForward", "", 0.1, false);
          }
          case 2, 5, 16: {
            CreateTimer(0.5, TimedOperator, 71);
            FastFire2("FB.TankTrain", "TeleportToPathTrack", "Tank01", 0.0, false);
            FastFire2("FB.TankTrain", "StartForward", "", 0.25, false);
            FastFire2("FB.TankTrain", "SetSpeed", "1", 0.35, false);
            FastFire2("FB.Tank", "Enable", "", 1.0, false);
          }
          case 4, 8, 14: {
            CustomSoundEmitter("ambient/alarms/train_horn_distant1.wav", 65, false, 0, 1.0, 100);
            FastFire2("TrainSND","PlaySound", "", 0.0, false);
            FastFire2("TrainDamage", "Enable", "", 0.0, false);
            FastFire2("Train01", "Enable", "", 0.0, false);
            CPrintToChatAll("{darkviolet}[{red}WARN{darkviolet}] {orange}KISSONE'S TRAIN{white}is {red}INCOMING{white}. Look out!");
            FastFire2("TrainTrain", "TeleportToPathTrack", "TrainTrack01", 0.0, false);
            FastFire2("TrainTrain", "StartForward", "", 0.1, false);
          }
          case 6, 9: {
            WeatherManager.canTornado = true;
            CreateTimer(1.0, TimedOperator, 41);
          }
          case 7, 13: {
            CPrintToChatAll("{darkviolet}[{red}WARN{darkviolet}] {white}Uh oh, it appears to have started raining a {red}METEOR SHOWER{white}!!!");
            core.canSENTMeteors = true;
            CreateTimer(1.0, SENTMeteorTimer);
            CreateTimer(30.0, TimedOperator, 12);
          }
          case 11: {
            FastFire2("FB.Slice", "Enable", "", 0.0, false);
            CustomSoundEmitter("ambient/sawblade_impact1.wav", 65, false, 0, 1.0, 100);
            FastFire2("FB.Slice", "Disable", "", 1.0, false);
          }
        }
      }
    }
    case 512: { //SONIC
      CPrintToChatAll("{darkviolet}[{red}EXTERMINATUS{darkviolet}] {white}Client %N has sacrificed themselves with a {forestgreen}correct {white}key entry! Prepare your anus!", client);
      sudo(1006);
      weapon = "Correct FB Code Entry";
    }
    case 1024: { //ENERGYBEAM
      char EnergyDeath[32];
      Format(EnergyDeath, sizeof(EnergyDeath), (crusader ? "THE CRUSADER" : core.waveFlags == 1 ? "ONSLAUGHTER" : "LIGHTNING"));
      weapon = (crusader ? "Crusader" : core.waveFlags == 1 ? "Onslaughter" : "Lightning");
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N has been %s by {red}%s{white}!", client, (crusader ? "nuked" : core.waveFlags == 1 ? "deleted" : "struck"), EnergyDeath);
    }
    case 16384: { //DROWN
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N {red}DROWNED{white}.", client);
      weapon = "Darwin Award for Drowning";
    }
    case 32768: { //PARALYZE
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N has been crushed by a {darkviolet}MYSTERIOUS BLUE BALL{white}!", client);
      weapon = "Mysterious Blue Ball";
    }
    case 65536: { //NERVEGAS
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N has been {red}SLICED TO RIBBONS{white}!", client);
      weapon = "FB Code Entry Failed";
    }
    case 131072: { //POISON
      CPrintToChat(client, "{darkviolet}[{red}CORE{darkviolet}] {white}Please don't sit {red}IDLE {white}in the FC Tavern.");
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N was killed for standing in the Tavern instead of helping their team!", client);
      weapon = "Idle in FC Tavern..?";
    }
    case 262144: { //RADIATION
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N was blown away by a {red}NUCLEAR EXPLOSION{white}!", client);
      weapon = "Nuclear Explosion";
    }
    case 524288: { //DROWNRECOVER
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}Client %N experienced {red}TACO BELL{white}!", client);
      weapon = "Taco Bell";
    }
    case 1048576: { //ACID
      CPrintToChatAll( isGoobbue ? "{darkviolet}[{red}CORE{darkviolet}] {white}Client %N has been crushed by a {forestgreen}FALLING GOOBBUE FROM OUTER SPACE{white}!" : "{darkviolet}[{red}CORE{darkviolet}] {white}Client %N was deleted by {orange}AN AREA OF EFFECT{white}!", client);
      weapon = isGoobbue ? "Falling Goobbue" : "AREA_EFFECT";
    }
  }
  //Log if a player killed someone or was killed by a robot
  if (attacker != client && Ass_Database) {
    char query[256];
    int steamID = (IsValidClient(attacker) ? GetSteamAccountID(attacker) : GetSteamAccountID(client));
    char escName[128];
    SQL_EscapeString(Ass_Database, name, escName, sizeof(escName));
    Format(query, sizeof(query), IsValidClient(attacker) && !StrEqual(weapon, "world") ? "UPDATE ass_activity SET kills = kills +1, killssession = killssession + 1, lastkilledname = '%s', lastweaponused = '%s' WHERE steamid = %i;" : !StrEqual(weapon, "world") && !IsValidClient(attacker) ? "UPDATE ass_activity SET deaths = deaths + 1, deathssession = deathssession + 1, killedbyname = '%s', killedbyweapon = '%s' WHERE steamid = %i;" : "RETURN", escName, weapon, steamID);
    if (StrEqual(query, "RETURN")) return;
    Ass_Database.Query(Database_FastQuery, query);
  }
}