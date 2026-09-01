// Helper - Contains Methods such as checking if clients are valid, getting their max health, converting chars to bools, and doing math stuffs.

// Get the client's current healer
public int GetHealerOfClient(int client) {
  for (int i = 1; i <= MaxClients; ++i) {
    if(!IsValidClient(i) || GetClientTeam(i) == 3) continue;
    if (TF2_GetPlayerClass(i) == TFClass_Medic){
      int medigun = GetPlayerWeaponSlot(i, TFWeaponSlot_Secondary);
      if (!IsValidEdict(medigun) || !IsValidEntity(medigun)) { PrintToServer("medigun returned null"); return -1; }
      char s[32]; GetEdictClassname(medigun, s, sizeof(s));
      if (!strcmp(s, "tf_weapon_medigun", false)) {
        if (GetEntProp(medigun, Prop_Send, "m_bHealing") && WaveSystem().IsActive()) { if(GetEntPropEnt(medigun, Prop_Send, "m_hHealingTarget") == client) return i; }
      }
    }
  }
  return -1;
}

//Check if the client is valid
stock bool IsValidClient(int client) { if (0 < client) return (client <= MaxClients && IsClientInGame(client) && !IsFakeClient(client)); else return false; }

//Calculate time spent on server in seconds
int GetClientMapTime(int client) {
  float clientTime = GetClientTime(client), gameTime = GetGameTime();
  if (clientTime > gameTime) { return RoundToZero(gameTime); }
  return RoundToZero(clientTime);
}

//Get a client's health
stock int TF2_GetPlayerMaxHealth(int client) { return GetEntProp(GetPlayerResourceEntity(), Prop_Send, "m_iMaxHealth", _, client); }

//Converts chars to bool by checking if it's a bool statement
public bool CharToBool(const char[] str) { return (StrContains(str, "true") != -1 ? true : false); }

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

//Jump waves.
public void JumpToWave(int wave_number) {
  int flags = GetCommandFlags("tf_mvm_jump_to_wave");
  SetCommandFlags("tf_mvm_jump_to_wave", flags & ~FCVAR_CHEAT);
  ServerCommand("tf_mvm_jump_to_wave %d", wave_number);
  FakeClientCommand(0, "");
  SetCommandFlags("tf_mvm_jump_to_wave", flags | FCVAR_CHEAT);
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

// Gets weapon damage bonus
float GetTotalWeaponDamageBonus() {
  float totalDamageBonus = 0.0;
  for (int client = 1; client <= MaxClients; client++) {
    if (!IsClientInGame(client) || !IsPlayerAlive(client)) continue;
    for (int slot = 0; slot <= 3; slot++) {
      int weaponEnt = GetPlayerWeaponSlot(client, slot);
      if (weaponEnt == -1) continue;
      Address attr = TF2Attrib_GetByName(weaponEnt, "damage bonus");
      if (attr <= view_as<Address>(0)) continue;
      float dmgValue = TF2Attrib_GetValue(attr);
      totalDamageBonus += dmgValue;
      PrintToServer("Got damage bonus value %f from %N", dmgValue, client);
    }
  }
  PrintToServer("Final value %f", totalDamageBonus);
  return totalDamageBonus;
}

methodmap MATHS {
  // Adjust input by rate. Rate can be either positive (add) or negative (subtract)
  public float Adjust(float input, float rate) { return (input + rate); }
  // Clamps a float value between input and min/max value
  public float Clamp(float input, float min, float max) { return input > max ? max : input < min ? min : input; } 
  // Returns the lesser of two floats
  public float FloatMin(float a, float b) { return a <= b ? a : b; }
  // Returns the greater of two floats
  public float FloatMax(float a, float b) { return a >= b ? a : b; }
  // Returns the lesser of two ints
  public int IntMin(int a, int b) { return a <= b ? a : b; }
  // Returns the greater of two ints
  public int IntMax(int a, int b) { return a >= b ? a : b; }
  // Wraps an unsigned int between 0 and (maxSize - 1)
  public int WrapUINT(int inVal, int maxSize) { return inVal % maxSize; }
}
MATHS MathFunc;
MATHS Maths() {return MathFunc;}

// These timers will be moved to their respective files as soon as we can.
//Brute Justice Timer
public Action OnslaughterATK(Handle timer) {
  if (BossHandler.isAlive) {
    if (core.waveFlags == 1) {
      CreateTimer(GetRandomFloat(5.0, 7.0), OnslaughterATK);
      Photon.Fire("BruteJusticeDefaultATK", "FireMultiple", "3", 5.0);
      switch (GetRandomInt(1, 10)) {
        case 1, 6: {
          Photon.Fire("BruteJusticeLaserParticle", "Start");
          CustomSoundEmitterEX(SFXArray[38].realPath, _, _, 65, false, 0, 1.0, 100);
          Photon.Fire("BruteJusticeLaser", "TurnOn", "", 1.40);
          Photon.Fire("BruteJusticeLaserHurtAOE", "Enable", "", 1.40);
          Photon.Fire("BruteJusticeLaserParticle", "Stop", "", 3.00);
          Photon.Fire("BruteJusticeLaser", "TurnOff", "", 3.25);
          Photon.Fire("BruteJusticeLaserHurtAOE", "Disable", "", 3.25);
        }
        case 2, 8: {
          Photon.Fire("BruteJusticeJarateSpawner00", "ForceSpawn", "", 0.0);
          Photon.Fire("BruteJusticeJarateSpawner01", "ForceSpawn", "", 2.5);
          Photon.Fire("BruteJusticeJarateSpawner02", "ForceSpawn", "", 5.0);
          Photon.Fire("BruteJusticeJarateSpawner00", "ForceSpawn", "", 9.0);
          Photon.Fire("BruteJusticeJarateSpawner01", "ForceSpawn", "", 12.0);
          Photon.Fire("BruteJusticeJarateSpawner02", "ForceSpawn", "", 15.0);
        }
        case 3, 7: {
          Photon.Fire("BruteJusticeFlameParticle", "Start");
          Photon.Fire("BruteJusticeFlamethrowerHurtAOE", "Enable");
          CustomSoundEmitterEX(SFXArray[39].realPath, _, _, 65, false, 0, 1.0, 100);
          Photon.Fire("SND.BruteJusticeFlameATK", "PlaySound", "", 1.25);
          Photon.Fire("BruteJusticeFlamethrowerHurtAOE", "Disable", "", 5.0);
          Photon.Fire("BruteJusticeFlameParticle", "Stop", "", 5.0);
          Photon.Fire("SND.BruteJusticeFlameATK", "FadeOut", ".25", 5.0);
          CreateTimer(5.0, TimedOperator, 60);
          Photon.Fire("SND.BruteJusticeFlameATK", "StopSound", "", 5.1);
        }
        case 4: {
          Photon.Fire("BruteJusticeGrenadeSpammer", "FireMultiple", "10", 0.0);
          Photon.Fire("BruteJusticeGrenadeSpammer", "FireMultiple", "10", 3.0);
          Photon.Fire("BruteJusticeGrenadeSpammer", "FireMultiple", "10", 5.0);
        }
        case 5: {
          Photon.Fire("BruteJusticeGrenadeSpammer", "FireMultiple", "50", 0.0);
          Photon.Fire("BruteJusticeJarateSpawner00", "ForceSpawn", "", 0.0);
          Photon.Fire("BruteJusticeJarateSpawner01", "ForceSpawn", "", 2.5);
          Photon.Fire("BruteJusticeJarateSpawner02", "ForceSpawn", "", 5.0);
          Photon.Fire("BruteJusticeJarateSpawner00", "ForceSpawn", "", 9.0);
          Photon.Fire("BruteJusticeJarateSpawner01", "ForceSpawn", "", 12.0);
          Photon.Fire("BruteJusticeJarateSpawner02", "ForceSpawn", "", 15.0);
        }
        case 9: {
          Photon.Fire("BruteJusticeRocketSpammer", "FireOnce", "", 0.0);
          Photon.Fire("BruteJusticeRocketSpammer", "FireOnce", "", 5.0);
        }
        case 10: {
          Photon.Fire("BruteJusticeRocketSpammer", "FireMultiple", "10", 0.0);
          Photon.Fire("BruteJusticeRocketSpammer", "FireMultiple", "10", 3.0);
          Photon.Fire("BruteJusticeRocketSpammer", "FireMultiple", "10", 5.0);
        }
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
    Photon.Fire("SephArrows", "FireMultiple", "3", 5.0);
    SephBoss[GetRandomInt(0, 11)].attack();
  }
  return Plugin_Stop;
}

//Boss Health Timer
public Action BossHPTimer(Handle timer) {
  int BossEntProxy = (core.waveFlags == 1 ? Photon.Find("OnslaughterTank", "tank_boss") : core.waveFlags == 2 ? Photon.Find("SephirothTank", "tank_boss") : -1);
  int BossEnt = (core.waveFlags == 1 ? Photon.Find("FB.OnslaughterBase", "base_boss") : core.waveFlags == 2 ? Photon.Find("FB.SephirothDMGRelay", "func_physbox") : -1);
  if (BossEnt != -1 || BossEntProxy != -1) {
    SetEntProp(BossEntProxy, Prop_Data, "m_iHealth", GetEntProp(BossEnt, Prop_Data, "m_iHealth"));
    CPrintToChatAll((core.waveFlags == 1 ? "{blue}Onslaughter's HP: %i (%i)" : core.waveFlags == 2 ? "{blue}Sephiroth's HP: %i (%i)" : "{blue}Error: Boss not found... core.waveFlags was neither 1 nor 2"), GetEntProp(BossEnt, Prop_Data, "m_iHealth"), GetEntProp(BossEntProxy, Prop_Data, "m_iHealth"));
    CreateTimer(5.0, BossHPTimer);
  }
  return Plugin_Stop;
}

//SENTMeteor (Scripted Entity Meteors)
public Action SENTMeteorTimer(Handle timer) {
  if (core.canSENTMeteors) {
    CreateTimer(5.0, SENTMeteorTimer);
    Photon.Fire("MiniMeteor", "ForceSpawn");
    LightLogger(LOGLVL_WARN, "[Helper:214] Replace me with teleportEntity eventually then forcespawn...");
  }
  return Plugin_Stop;
}

//SENTStars (Scripted Entity Stars)
public Action SENTStarTimer(Handle timer) {
  PrintToChatAll("Core can sent starts: %i", core.canSENTStars == true ? 1 : 0);
  if (core.canSENTStars) {
    Photon.Fire(FB_SENT[10], "ForceSpawn"); // replace me with teleportEntity eventually then forcespawn...
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