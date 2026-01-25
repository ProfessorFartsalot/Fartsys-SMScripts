//ASS_HELPER - Contains the config loaders and all the plugin core data entries as well as some stocks required for base functionality.
//Log debug info
enum {
  LOGLVL_DEBUG = 0,
  LOGLVL_INFO = 1,
  LOGLVL_WARN = 2,
  LOGLVL_ERROR = 3
};
int GetDefaultBGM() {
  return WaveSystem().IsActive() ? (core.sephiroth ? 16 : DefaultsArray[WaveSystem().GetWave()].defBGMIndex) : GetRandomInt(1, 4);
}
//Idle system
bool _serverActive = false;
public void OnClientDisconnect_Post() {
  if (GetClientCount(false) == 0) {
    _serverActive = false;
    CreateTimer(300.0, ServerIdleThread);
  }
  return;
}
public Action ServerIdleThread(Handle timer) {
  if (!_serverActive) ServerCommand("_restart");
  return Plugin_Stop;
}

enum struct PLAYERDATA {
  char classname;
  char name;
}
void AssLogger(int logLevel, const char[] logData, any...) {
  char buffer[256];
  VFormat(buffer, sizeof(buffer), logData, 3);
  switch (logLevel) {
    case LOGLVL_DEBUG: {
      LogMessage("[DEBUG]: %s", buffer);
    }
    case LOGLVL_INFO: {
      LogMessage("[INFO]: %s", buffer);
    }
    case LOGLVL_WARN: {
      LogMessage("[WARN]: %s", buffer);
    }
    case LOGLVL_ERROR: {
      LogMessage("[ERROR]: %s", buffer);
    }
  }
}

int attemptSpawn = 0;
bool isGoobbue = false;

stock int GetEmnityMax() {
  int result = 0;
  int score = -1;
  int tempscore = 0;
  for(int i = 0; i < MaxClients; ++i) {
    if(!IsValidClient(i)) {
      continue;
    }
    tempscore = EmnityManager[i].getClientEmnity();
    if(score == - 1 || tempscore > score) {
      result = i;
      score = tempscore;
    }
  }
  return result;
}

public int GetHealerOfClient(int client){
  for (int i = 1; i <= MaxClients; ++i){
    if(!IsValidClient(i) || GetClientTeam(i) == 3) continue;
    if (TF2_GetPlayerClass(i) == TFClass_Medic){
      int medigun = GetPlayerWeaponSlot(i, TFWeaponSlot_Secondary);
      if (!IsValidEdict(medigun) || !IsValidEntity(medigun)){
        PrintToServer("medigun returned null");
        return -1;
      }
      char s[32];
      GetEdictClassname(medigun, s, sizeof(s));
      if (!strcmp(s, "tf_weapon_medigun", false)) {
        if (GetEntProp(medigun, Prop_Send, "m_bHealing") && WaveSystem().IsActive()) {
          if(GetEntPropEnt(medigun, Prop_Send, "m_hHealingTarget") == client) return i;
        }
      }
    }
  }
  return -1;
}
/*
public Action UpdateMedicHealing(Handle timer){
  int CurHealVal[MAXPLAYERS+1];
  for (int i = 1; i <= MaxClients; ++i){
    if(!IsValidClient(i) || GetClientTeam(i) == 3) continue;
    if (TF2_GetPlayerClass(i) == TFClass_Medic){
      int medigun = GetPlayerWeaponSlot(i, TFWeaponSlot_Secondary);
      if (!IsValidEdict(medigun) || !IsValidEntity(medigun)){
        PrintToServer("medigun returned null");
        return Plugin_Stop;
      }
      CurHealVal[i] = GetEntProp(i, Prop_Send, "m_iHealPoints");
      char s[32];
      GetEdictClassname(medigun, s, sizeof(s));
      if (!strcmp(s, "tf_weapon_medigun", false)) {
        if (GetEntProp(medigun, Prop_Send, "m_bHealing") && WaveSystem().IsActive()){
          int hOffset = (CurHealVal[i] - EmnityManager[i].InitHealingValue);
          PrintToServer("%N is healing %N with %i HP since last check", i, GetEntPropEnt(medigun, Prop_Send, "m_hHealingTarget"), hOffset);
          EmnityManager[i].iHealing += hOffset;
          iDmgHealingTotal += hOffset;
        }
      }
      EmnityManager[i].InitHealingValue = CurHealVal[i];
    }
  }
  CreateTimer(1.0, UpdateMedicHealing);
  return Plugin_Stop;
}

public void UpdateAllHealers() {
  for (int i = 1; i <= MaxClients; ++i){
    if(!IsValidClient(i) || GetClientTeam(i) == 3) continue;
    if (TF2_GetPlayerClass(i) == TFClass_Medic){
      EmnityManager[i].InitHealingValue = ((TF2_GetPlayerClass(i) == TFClass_Medic)) ? GetEntProp(i, Prop_Send, "m_iHealPoints") : 0;
    }
  }
}*/

//Check if the client is valid
stock bool IsValidClient(int client) {
  return (0 < client <= MaxClients && IsClientInGame(client) && !IsFakeClient(client));
}

//Calculate time spent on server in seconds
int GetClientMapTime(int client) {
  float clientTime = GetClientTime(client), gameTime = GetGameTime();
  if (clientTime > gameTime) {
    return RoundToZero(gameTime);
  }
  return RoundToZero(clientTime);
}
//Find an entity by its target name
int FindEntityByTargetname(char[] targetname, char[] classname)
{
  char namebuf[32];
  int index = -1;
  namebuf[0] = '\0';
 
  while(strcmp(namebuf, targetname) != 0 && (index = FindEntityByClassname(index, classname)) != -1){
    GetEntPropString(index, Prop_Data, "m_iName", namebuf, sizeof(namebuf));
    //PrintToServer("Found %s, index %i", namebuf, index);
  }
  return(index);
}

//Get a client's health
stock int TF2_GetPlayerMaxHealth(int client) {
  return GetEntProp(GetPlayerResourceEntity(), Prop_Send, "m_iMaxHealth", _, client);
}

//Converts chars to bool by checking if it's a bool statement
public bool CharToBool(const char[] str){
  return (StrContains(str, "true") > 0 ? true : false);
}

//Track and update client health
public Action TickClientHealth(Handle timer) {
  for (int i = 1; i <= MaxClients; i++) {
    if (IsValidClient(i) && (GetClientTeam(i) == 2)) {
      int health = GetClientHealth(i);
      int healthMax = TF2_GetPlayerMaxHealth(i);
      if (!Ass_Database) return Plugin_Stop;
      else {
        char query[256];
        int steamID = GetSteamAccountID(i);
        Format(query, sizeof(query), "UPDATE ass_activity SET health = %i, maxHealth = %i WHERE steamid = %i;", health, healthMax, steamID);
        Ass_Database.Query(Database_FastQuery, query);
      }
    }
  }
  if (GetClientCount(true) == 0) {
    core.tickingClientHealth = false;
    return Plugin_Stop;
  }
  CreateTimer(1.0, TickClientHealth);
  return Plugin_Stop;
}
public Action TimedAOEs(Handle timer){
  if (WaveSystem().IsActive()){
    for (int i = 0; i < GetRandomInt(1, 10); i++){
      CreateTimer(i/10.0, TimedOperator, 201);
    }
    CreateTimer(GetRandomFloat(10.0, 45.0), TimedAOEs);
  }
  return Plugin_Stop;
}
//Place random AOE barrage
void GroupedAOE() {
  int ent = FindEntityByClassname(-1, "tf_objective_resource");
  int bombBotLevel = GetEntData(ent, FindSendPropInfo("CTFObjectiveResource", "m_nFlagCarrierUpgradeLevel"));
  switch(GetRandomInt(0, bombBotLevel+1)){
    case 0:{

    }
    case 1:{

    }
    case 2:{

    }
    case 3:{

    }
    case 4:{

    }
  }
}

// Returns the lesser of two floats
stock float FloatMin(float a, float b) {
  return a <= b ? a : b;
}

// Returns the greater of two floats
stock float FloatMax(float a, float b) { 
  return a >= b ? a : b;
}

//Jump waves.
public Action JumpToWave(int wave_number) {
  int flags = GetCommandFlags("tf_mvm_jump_to_wave");
  SetCommandFlags("tf_mvm_jump_to_wave", flags & ~FCVAR_CHEAT);
  ServerCommand("tf_mvm_jump_to_wave %d", wave_number);
  FakeClientCommand(0, "");
  SetCommandFlags("tf_mvm_jump_to_wave", flags | FCVAR_CHEAT);
  return Plugin_Handled;
}
//Dispatch a circle AOE
public Action DispatchCircleAOE(float pos[3]) {
  int ent = FindEntityByTargetname("CircleTemplate", "point_template");
  if (ent != -1) {
    float v[3];
    float rot[3];
    TeleportEntity(ent, pos, rot, v);
    FastFire2("CircleTemplate", "ForceSpawn", "", 0.0, false);
  }
  return Plugin_Stop;
}
//Track bombState[0].state and update entities every 0.1 seconds
public Action BombStatusUpdater(Handle timer) {
  if (WaveSystem().IsActive()) {
    CreateTimer(0.1, BombStatusUpdater);
    if (bombState[0].state < bombState[0].stateMax) {
      switch (bombState[0].state) {
      case 8, 16, 24, 32, 40, 48, 56, 64: {
        int curBomb = bombState[0].getCurBomb();
        bombState[0].stateMax = bombState[0].state;
        bombState[0].isReady = true;
        core.canSENTShark = bombState[curBomb].canSENTShark;
        FastFire2("Bombs*", "Disable", "", 0.0, false);
        FastFire2("Delivery", "Unlock", "", 0.0, false);
        FastFire2(bombState[curBomb].identifier, "Enable", "", 0.0, false);
        CustomSoundEmitter(SFXArray[56].realPath, 65, false, 0, 1.0, 100);
        CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {forestgreen}Your team's %s {forestgreen}is now available for deployment!", bombState[curBomb].name);
      }
      }
    } else if (bombState[0].state > bombState[0].stateMax) bombState[0].state = bombState[0].upperLimit();
  }
  return Plugin_Stop;
}

//RobotLaunchTimer (Randomly fling robots)
public Action RobotLaunchTimer(Handle timer) {
  if (!WaveSystem().IsActive()) return Plugin_Stop;
  FastFire2("FB.RobotLauncher", "Enable", "", 0.0, false);
  FastFire2("FB.RobotLauncher", "Disable", "", 7.5, false);
  CreateTimer(GetRandomFloat(5.0, 30.0), RobotLaunchTimer);
  return Plugin_Stop;
}
//Track SacPoints and update entities every 0.1 seconds
public Action SacrificePointsUpdater(Handle timer) {
  if (!WaveSystem().IsActive()) return Plugin_Stop;
  CreateTimer(0.1, SacrificePointsUpdater);
  if (ass_sacPoints > ass_sacPointsMax) ass_sacPoints = ass_sacPointsMax;
  return Plugin_Stop;
}

//BombStatus (Add points to Bomb Status occasionally)
public Action BombStatusAddTimer(Handle timer) {
  if (WaveSystem().IsActive() && (bombState[0].state < bombState[0].stateMax)) {
    bombState[0].isReady = false;
    bombState[0].state++;
    CreateTimer(GetRandomFloat(10.0, 45.0), BombStatusAddTimer);
  }
  return Plugin_Stop;
}
//Adverts for tips/tricks
public Action PerformAdverts(Handle timer) {
  if (!WaveSystem().IsActive()) {
    CreateTimer(180.0, PerformAdverts);
    CPrintToChatAll(AdvMessage[GetRandomInt(0, 7)]);
  }
  return Plugin_Stop;
}

//Adverts for wave information
public Action PerformWaveAdverts(Handle timer) {
  if (WaveSystem().IsActive()) {
    char buffer[16];
    int emnity;
    char tbuffer[16];
    char HintText[256];
    //int sPos = RoundToFloor((AudioManager.timeSeconds() / (GetEngineTime() + AudioManager.loopSeconds)) * AudioManager.loopSeconds);
    for (int i = 0; i <= MaxClients; ++i) { //might need to be i++??
      if (IsValidClient(i)) {
        int sPos = RoundToFloor(AudioManager[i].engineSecondsAdjusted() - AudioManager[i].timeSeconds);
        int tPos = RoundToFloor(AudioManager[i].loopSeconds);
        Format(buffer, 16, "%02d:%02d", sPos / 60, sPos % 60);
        Format(tbuffer, 16, "%02d:%02d", tPos / 60, tPos % 60);
        Format(HintText, sizeof(HintText), (bombState[0].isMoving ? "Payload: MOVING (%i/%i) | !sacpoints: %i/%i \n Music: %s (%s/%s)" : bombState[0].isReady ? "Payload: READY (%i/%i) | !sacpoints: %i/%i \n Music: %s (%s/%s)" : "Payload: PREPARING (%i/%i) | !sacpoints: %i/%i \n Music: %s (%s/%s)"), bombState[0].state, bombState[0].stateMax, ass_sacPoints, ass_sacPointsMax, AudioManager[i].songName, buffer, tbuffer);
        emnity = EmnityManager[i].getClientEmnity();
        PrintHintText(i, (WeatherManager.TornadoWarning ? "%s \n\n[TORNADO WARNING]" : "%s\n\nEmnity: %i٪"), HintText, emnity >= 0 ? emnity : 0);
        StopSound(i, SNDCHAN_STATIC, "UI/hint.wav");
      }
    }
    CreateTimer(2.5, PerformWaveAdverts);
  }
  return Plugin_Stop;
}

//Feature admin timer
public Action SelectAdminTimer(Handle timer) {
  if (!WaveSystem().IsActive()) {
    sudo(1002);
    CreateTimer(GetRandomFloat(40.0, 120.0), SelectAdminTimer);
  }
  return Plugin_Stop;
}

//Brute Justice Timer
public Action OnslaughterATK(Handle timer) {
  if (core.waveFlags == 1) {
    CreateTimer(GetRandomFloat(5.0, 7.0), OnslaughterATK);
    FastFire2("BruteJusticeDefaultATK", "FireMultiple", "3", 5.0, false);
    switch (GetRandomInt(1, 10)) {
    case 1, 6: {
      FastFire2("BruteJusticeLaserParticle", "Start", "", 0.0, false);
      CustomSoundEmitter(SFXArray[38].realPath, 65, false, 0, 1.0, 100);
      FastFire2("BruteJusticeLaser", "TurnOn", "", 1.40, false);
      FastFire2("BruteJusticeLaserHurtAOE", "Enable", "", 1.40, false);
      FastFire2("BruteJusticeLaserParticle", "Stop", "", 3.00, false);
      FastFire2("BruteJusticeLaser", "TurnOff", "", 3.25, false);
      FastFire2("BruteJusticeLaserHurtAOE", "Disable", "", 3.25, false);
    }
    case 2, 8: {
      FastFire2("BruteJusticeJarateSpawner00", "ForceSpawn", "", 0.0, false);
      FastFire2("BruteJusticeJarateSpawner01", "ForceSpawn", "", 2.5, false);
      FastFire2("BruteJusticeJarateSpawner02", "ForceSpawn", "", 5.0, false);
      FastFire2("BruteJusticeJarateSpawner00", "ForceSpawn", "", 9.0, false);
      FastFire2("BruteJusticeJarateSpawner01", "ForceSpawn", "", 12.0, false);
      FastFire2("BruteJusticeJarateSpawner02", "ForceSpawn", "", 15.0, false);
    }
    case 3, 7: {
      FastFire2("BruteJusticeFlameParticle", "Start", "", 0.0, false);
      FastFire2("BruteJusticeFlamethrowerHurtAOE", "Enable", "", 0.0, false);
      CustomSoundEmitter(SFXArray[39].realPath, 65, false, 0, 1.0, 100);
      FastFire2("SND.BruteJusticeFlameATK", "PlaySound", "", 1.25, false);
      FastFire2("BruteJusticeFlamethrowerHurtAOE", "Disable", "", 5.0, false);
      FastFire2("BruteJusticeFlameParticle", "Stop", "", 5.0, false);
      FastFire2("SND.BruteJusticeFlameATK", "FadeOut", ".25", 5.0, false);
      CreateTimer(5.0, TimedOperator, 60);
      FastFire2("SND.BruteJusticeFlameATK", "StopSound", "", 5.1, false);
    }
    case 4: {
      FastFire2("BruteJusticeGrenadeSpammer", "FireMultiple", "10", 0.0, false);
      FastFire2("BruteJusticeGrenadeSpammer", "FireMultiple", "10", 3.0, false);
      FastFire2("BruteJusticeGrenadeSpammer", "FireMultiple", "10", 5.0, false);
    }
    case 5: {
      FastFire2("BruteJusticeGrenadeSpammer", "FireMultiple", "50", 0.0, false);
      FastFire2("BruteJusticeJarateSpawner00", "ForceSpawn", "", 0.0, false);
      FastFire2("BruteJusticeJarateSpawner01", "ForceSpawn", "", 2.5, false);
      FastFire2("BruteJusticeJarateSpawner02", "ForceSpawn", "", 5.0, false);
      FastFire2("BruteJusticeJarateSpawner00", "ForceSpawn", "", 9.0, false);
      FastFire2("BruteJusticeJarateSpawner01", "ForceSpawn", "", 12.0, false);
      FastFire2("BruteJusticeJarateSpawner02", "ForceSpawn", "", 15.0, false);
    }
    case 9: {
      FastFire2("BruteJusticeRocketSpammer", "FireOnce", "", 0.0, false);
      FastFire2("BruteJusticeRocketSpammer", "FireOnce", "", 5.0, false);
    }
    case 10: {
      FastFire2("BruteJusticeRocketSpammer", "FireMultiple", "10", 0.0, false);
      FastFire2("BruteJusticeRocketSpammer", "FireMultiple", "10", 3.0, false);
      FastFire2("BruteJusticeRocketSpammer", "FireMultiple", "10", 5.0, false);
    }
    }
  }
  return Plugin_Stop;
}

//Sephiroth Timer
public Action SephATK(Handle timer) {
  if (core.waveFlags == 2) {
    float f = GetRandomFloat(5.0, 10.0);
    CreateTimer(f, SephATK);
    FastFire2("SephArrows", "FireMultiple", "3", 5.0, false);
    SephBoss[GetRandomInt(0, 11)].attack();
  }
  return Plugin_Stop;
}

//Boss Health Timer
public Action BossHPTimer(Handle timer) {
  int BossEntProxy = (core.waveFlags == 1 ? FindEntityByTargetname("OnslaughterTank", "tank_boss") : core.waveFlags == 2 ? FindEntityByTargetname("SephirothTank", "tank_boss") : -1);
  int BossEnt = (core.waveFlags == 1 ? FindEntityByTargetname("FB.OnslaughterBase", "base_boss") : core.waveFlags == 2 ? FindEntityByTargetname("FB.SephirothDMGRelay", "func_physbox") : -1);
  if (BossEnt != -1 || BossEntProxy != -1) {
    SetEntProp(BossEntProxy, Prop_Data, "m_iHealth", GetEntProp(BossEnt, Prop_Data, "m_iHealth"));
    CPrintToChatAll((core.waveFlags == 1 ? "{blue}Onslaughter's HP: %i (%i)" : core.waveFlags == 2 ? "{blue}Sephiroth's HP: %i (%i)" : "{blue}Error: Boss not found... core.waveFlags was neither 1 nor 2"), GetEntProp(BossEnt, Prop_Data, "m_iHealth"), GetEntProp(BossEntProxy, Prop_Data, "m_iHealth"));
    CreateTimer(5.0, BossHPTimer);
  }
  return Plugin_Stop;
}

//Shark Timer
public Action SharkTimer(Handle timer) {
  if (core.canSENTShark) {
    FastFire2("SentSharkTorpedo", "ForceSpawn", "", 0.0, false);
    CreateTimer(GetRandomFloat(2.0, 5.0), SharkTimer);
    CustomSoundEmitter(SFXArray[GetRandomInt(43, 50)].realPath, 65, false, 0, 1.0, 100);
  }
  return Plugin_Stop;
}

//SpecTimer
public Action SpecTimer(Handle timer) {
  if (WaveSystem().IsActive()) {
    FastFire2("Spec*","Disable","", 0.0, false);
    FastFire2(SpecEnt[GetRandomInt(0, 5)], "Enable", "", 0.1, false);
    CreateTimer(GetRandomFloat(10.0, 30.0), SpecTimer);
  }
  return Plugin_Stop;
}

//SENTMeteor (Scripted Entity Meteors)
public Action SENTMeteorTimer(Handle timer) {
  if (core.canSENTMeteors) {
    CreateTimer(5.0, SENTMeteorTimer);
    FastFire2(FB_SENT[GetRandomInt(0, 4)], "ForceSpawn", "", 0.0, false); // replace me with teleportEntity eventually then forcespawn...
  }
  return Plugin_Stop;
}

//SENTStars (Scripted Entity Stars)
public Action SENTStarTimer(Handle timer) {
  if (core.canSENTStars) {
    FastFire2(FB_SENT[10], "ForceSpawn", "", 0.0, false); // replace me with teleportEntity eventually then forcespawn...
    CreateTimer(GetRandomFloat(0.75, 1.5), SENTStarTimer);
  }
  return Plugin_Stop;
}

//Halloween Bosses
public Action HWBosses(Handle timer) {
  if (WaveSystem().IsActive() && core.canHWBoss) {
    HWBoss[GetRandomInt(0, 11)].attack();
    core.canHWBoss = false;
    CreateTimer(60.0, HWBossesRefire);
  }
  return Plugin_Stop;
}

//Repeat HWBosses Timer
public Action HWBossesRefire(Handle timer) {
  if (WaveSystem().IsActive()) CreateTimer(GetRandomFloat(core.HWNMin, core.HWNMax), HWBosses);
  return Plugin_Stop;
}

//SacPoints (Add points to Sacrifice Points occasionally)
public Action SacrificePointsTimer(Handle timer) {
  if (WaveSystem().IsActive() && (ass_sacPoints < ass_sacPointsMax)) {
    ass_sacPoints++;
    CreateTimer(GetRandomFloat(5.0, 30.0), SacrificePointsTimer);
  }
  return Plugin_Stop;
}

/* Sends hud text to players
* @param channel The channel to use (1 - 4)
* @param text The text to display
* @param posX X pos L-R (0 to 1)
* @param posY Y pos U-D (0 to 1)
* @param holdTime Time to display the text
* @param color1[4] Target Color rgba
* @param color2[4] Initial Color rgba
* @param effect 0/1 Fade In / Out, 2 = Scan Out
* @param fxTime Time it takes for the effect to run
* @param fadeIn Time it takes for the text to fade in
* @param fadeOut Time it takes for the text to fade out
*/
void SendHudTextAll(int channel, const char[] text, float posX, float posY, float holdTime, int color1[4], int color2[4], int effect, float fxTime, float fadeIn, float fadeOut){
  for (int i = 1; i <= MaxClients; i++) {
    if (IsClientInGame(i)) {
      SetHudTextParamsEx(posX, posY, holdTime, color1, color2, effect, fxTime, fadeIn, fadeOut);
      ShowHudText(i, channel, text);
    }
  }
}

// Strips weapons from player
void StripWeapons(int client) { 
  int weapon;
  for (int slot = 0; slot < 6; slot++) {
    while ((weapon = GetPlayerWeaponSlot(client, slot)) != -1) {
      RemovePlayerItem(client, weapon);
      AcceptEntityInput(weapon, "Kill");
    }
  }
}