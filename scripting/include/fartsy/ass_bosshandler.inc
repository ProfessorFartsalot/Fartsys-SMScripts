 // Fartsy's Boss Handler v1.0.0
bool canCrusaderNuke = false;
bool crusader = false;
int INCOMINGDISPLAYED = 0;
BULKFIRE crusaderAtk[64];
//Registry of all playable classes, used for stats module
char ClassDefinitions[][32] = {
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
//Crusader Incoming Timer for Crusader
public Action CRUSADERINCOMING(Handle timer) {
  if (crusader && INCOMINGDISPLAYED < 17) {
    INCOMINGDISPLAYED++;
    FastFire2("FB.INCOMING", "Display");
    CreateTimer(1.75, CRUSADERINCOMING);
  }
  if (INCOMINGDISPLAYED == 17) INCOMINGDISPLAYED = 0;
  return Plugin_Stop;
}

public int iDmgHealingTotal;
enum struct EMNITYMANAGER {
  int iAssisted;
  int iBossDamage;
  int iDamage;
  int iHealing;
  int InitHealingValue;
  int getClientEmnity() {
    //PrintToConsoleAll ("Getting emnity returned Damage %i + Healing %i / %i Total, or %i Emnity ", this.iDamage, this.iHealing, iDmgHealingTotal, RoundToFloor(((this.iDamage * 1.0 + this.iHealing * 1.0) / ((iDmgHealingTotal == 0) ? 1 : iDmgHealingTotal) * 100)));
    return RoundToFloor((((this.iAssisted + this.iDamage + this.iHealing) * 1.0) / (iDmgHealingTotal * 1.0)) * 100.0);
  }
  bool IsAboveEmnityThreshold(int value) {
    return (RoundToFloor(((this.iAssisted + this.iDamage + this.iHealing) * 1.0) / (iDmgHealingTotal * 1.0)) > value);
  }
  void Add(int client, int type, int i, const char[] optTarget = "") {
    int steamID = GetSteamAccountID(client);
    switch (type) {
      // Damage
      case 0: {
        // Add bonus emnity for healers
        int healer = GetHealerOfClient(client);
        if (healer != -1) { AddHealerEmnity(healer, RoundToFloor(i * 0.35)); }
        // Add their damage to the stats system, additionally log it to MySQL stats if possible
        this.iDamage += i;
        if (!Ass_Database || !steamID) return;
        char query[256];
        Format(query, sizeof(query), "UPDATE ass_activity SET damagedealt = damagedealt + %i, damagedealtsession = damagedealtsession + %i WHERE steamid = %i;", i, i, steamID);
        Ass_Database.Query(Database_FastQuery, query);
      }
      // Healing
      case 1: {
        this.iHealing += i;
        if (!Ass_Database || !steamID) return;
        char escTarget[32];
        //if(!StrEqual(optTarget, "")) SQL_EscapeString(Ass_Database, optTarget, escTarget, sizeof(escTarget));
        //char query[256];
        //Format(query, sizeof(query), "UPDATE ass_activity SET healingtotal = healingtotal + %i, healingsession = healingsession + %i, lasthealedname = %N, WHERE steamid = %i;", i, i, escTarget, steamID); //TO DO: RESET ON CLIENT DISCONNECT.
        //Ass_Database.Query(Database_FastQuery, query);
      }
      // Other
      case 2: {
        this.iAssisted += i;
      }
    }
    iDmgHealingTotal += i;
  }
  void Clear() {
    iDmgHealingTotal -= this.iAssisted;
    iDmgHealingTotal -= this.iDamage;
    iDmgHealingTotal -= this.iHealing;
    this.iAssisted = 0;
    this.iBossDamage = 0;
    this.iDamage = 0;
    this.iHealing = 0;
  }
  void OnSpawned(int client, int class) {
    int steamID = GetSteamAccountID(client);
    if (!Ass_Database || !steamID) return;
    char strClass[32]; char query[256];
    strClass = ClassDefinitions[class - 1];
    Format(query, sizeof(query), "UPDATE ass_activity SET class = '%s' WHERE steamid = %i;", strClass, steamID);
    Ass_Database.Query(Database_FastQuery, query);
  }
}
EMNITYMANAGER EmnityManager[MAXPLAYERS+1];

void AddHealerEmnity(int healer, int i) {
  if (IsValidClient(healer)) EmnityManager[healer].Add(healer, 2, i);
}

public int GetEmnityMax() {
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

enum struct BOSSHANDLER {
  int bossID;
  bool bTargetInRange[MAXPLAYERS + 1];
  bool bDoTankBuster;
  bool isAlive;
  bool shouldTick;
  bool shouldTickBGM;
  bool sOverflow;
  bool tickBuster;
  bool tickBusterNuclear;
  float fAng[3];
  float fPos[3];
  int bustTarget;
  int bustNukeTarget;
  int iBossEnt;
  int iBossEntHealth;
  int iBossEntMaxHealth;
  int iBossEntTarget;
  int iBossPhase;
  int iBossPhaseTmp;
  void EmitSpawnSound(int boss) {
    switch (boss) {
      //Onslaughter
      case 0: {
        //Onslaughter has no spawn sound for now.... maybe when brute justice happens c:
      }
      //Sephiroth (wip)
      case 1: {
        
      }
      //Metallizer (wip) To Do: Only change to Inferno if we're in boss attack range. Use a trigger_multiple for this. To Do, delay this by like 1 second so the DJMM sound can play properly.
      case 2: {
        sudo(1001);
        AudioManager.Stop(true);
        AudioManager.setBGM_Global(20, true);
        CustomSoundEmitter(SFXArray[106].realPath, 65, false, 0, 1.0, 100); //DJMM SFX
        CreateTimer(1.5, DelayedMetalFace);
      }
    }
  }
  //Handle deaths
  void OnKilled() {
    if (this.isAlive) SCR_SendEvent("Boss Slain", this.bossID == 0 ? "Onslaughter has been killed!" : this.bossID == 1 ? "Sephiroth has been killed!" : "Metal Face has been killed!");
    switch (this.bossID) {
      case 0: {
        FastFire2("OnslaughterTank", "SetHealth", "1");
        FastFire2("FB.BruteJustice", "Disable");
        FastFire2("FB.BruteJusticeTrain", "Stop");
        FastFire2("FB.BruteJusticeParticles", "Stop");
        FastFire2("FB.BruteJusticeDMGRelay", "Break");
        FastFire2("FB.BruteJusticeTrain", "TeleportToPathTrack", "tank_path_a_10", 0.5);
        FastFire2("TankRelayDMG", "Enable", "", 1.0);
        FastFire2("TankRelayDMG", "Disable", "", 10.0);
        ass_sacPoints += 25;
      }
      //Sephiroth lost
      case 1: {
        
      }
      //Metal Face lost
      case 2: {
        FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "MFTHOME");
      }
    }
    this.isAlive = false;
    this.shouldTick = false;
    this.tickBuster = false;
    this.iBossPhase = 0;
  }
  //For tracking model and skipping player ready check if boss is alive
  void Tick() {
    //PrintToServer("Boss %i, hp %i/%i", this.bossID, this.iBossEntHealth, this.iBossEntMaxHealth);
    if (this.shouldTickBGM)(this.TickBGM());
    if (this.isAlive && !WaveSystem().IsActive()) {
      PrintToChatAll("PLACEHOLDER -- FORCE PLAYERS TO BE READY IF NOT IN WAVE.");
    }
    if (this.tickBuster)this.TickBuster();
    switch (this.bossID) {
      case 0: {
        float pos[3], ang[3], vel[3], newPos[3];
        vel[0] = 0.0;
        vel[1] = 0.0;
        vel[2] = 0.0;
        int BossEnt = FindEntityByTargetname("FB.BruteJusticeTrain", "func_tracktrain");
        int BossTP = FindEntityByTargetname("FB.OnslaughterBase", "base_boss");
        if (!IsValidEntity(BossTP)) { sudo(13); this.OnKilled(); }
        GetEntPropVector(BossEnt, Prop_Send, "m_vecOrigin", pos);
        GetEntPropVector(BossEnt, Prop_Data, "m_angRotation", ang);
        newPos[0] = pos[0];
        newPos[1] = pos[1];
        newPos[2] = pos[2] + 0.0;
        TeleportEntity(BossTP, newPos, ang, vel);
      }
      case 1: {  }
      case 2: {
        this.iBossEnt = FindEntityByTargetname("FB.MetalBase", "base_boss");
        if (!IsValidEntity(this.iBossEnt))this.iBossPhase = 4;
        else {
          this.iBossEntHealth = GetEntProp(this.iBossEnt, Prop_Data, "m_iHealth");
          this.iBossEntMaxHealth = GetEntProp(this.iBossEnt, Prop_Data, "m_iMaxHealth");
          this.iBossEntTarget = FindEntityByTargetname("FB.MetalFace.Train", "func_tracktrain");
          GetEntPropVector(this.iBossEntTarget, Prop_Send, "m_vecOrigin", this.fPos);
          GetEntPropVector(this.iBossEntTarget, Prop_Data, "m_angRotation", this.fAng);
          float vel[3];
          float newPos[3];
          vel[0] = 0.0;
          vel[1] = 0.0;
          vel[2] = 0.0;
          newPos[0] = this.fPos[0];
          newPos[1] = this.fPos[1];
          newPos[2] = this.fPos[2] - 50.0;
          TeleportEntity(this.iBossEnt, newPos, this.fAng, vel);
        }
        //Also need to set entity angles to its target... we can use this.bustTarget for the index of the client to get the angles...
        //Do phase changes
        switch (RoundToFloor((this.iBossEntHealth * 1.0 / this.iBossEntMaxHealth * 1.0) * 10.0)) {
          case 10: {
            this.iBossPhase = 1;
          }
          case 7: {
            this.iBossPhase = 2;
          }
          case 4: {
            this.iBossPhase = 3;
          }
          case 1: {
            this.iBossPhase = 4;
          }
        }
        if (this.iBossPhase != this.iBossPhaseTmp) {
          PrintToChatAll("PHASE CHANGED, %i to %i", this.iBossPhaseTmp, this.iBossPhase);
          this.iBossPhaseTmp = this.iBossPhase;
          if (this.iBossPhase > 1) {
            FastFire2("mfTelefect", "Start");
            FastFire2("FB.MetalFace.ReturnSND", "PlaySound");
            FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "MFTHOME", 0.25);
            FastFire2("FB.MetalFace.Invuln", "Enable", "", 3.0);
          }
          //Phase changing, these only ever happen once per phase.
          switch (this.iBossPhase) {
            case 0: {
              PrintToChatAll("Phase 0 happened, boss must must be dead /shrug");
            }
            case 1: {
              //PrintToChatAll("Recognized 0 -> 1, the boss must have just spawned!");
              FastFire2("FB.MetalBase", "SetHealth", "320000");
              FastFire2("FB.MetalBase", "SetMaxHealth", "320000");
              FastFire2("FB.MetalBase", "AddOutput", "health 320000");
              FastFire2("FB.MetalFace", "Enable");
            }
            case 2: {
              //PrintToChatAll("Recognized 1 -> 2, things are heating up!");
              AudioManager.setBGM_Global(22, false);
              FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "MFT1x0", 3.0);
              FastFire2("FB.MetalFace.SkeleSpawner", "Enable");
              FastFire2("FB.MetalFace.SkeleSpawner", "Disable", "", 120.0);
              FastFire2("mfTelefect", "Start", "", 120.0);
              FastFire2("FB.MetalFace.ReturnSND", "PlaySound", "", 120.0);
              FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "MFTHOME", 120.25);
              FastFire2("FB.MetalFace.Invuln", "Disable", "", 123.0);
              FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "tank_path_a_100", 126.85);
              FastFire2("mfTelefect", "Start", "", 127.0);
              FastFire2("FB.MetalFace.ReturnSND", "PlaySound", "", 127.0);
            }
            case 3: {
              //PrintToChatAll("Recognized 2 -> 3, oh boi");
              AudioManager.setBGM_Global(24, false);
              FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "MFT2x0", 3.00);
              FastFire2("FB.MetalFace.MerasmusSpawner", "ForceSpawn", "", 5.0); //Spawn halloween bosses at MetalFace's location
              FastFire2("FB.MetalFace.MonoculusSpawner", "ForceSpawn", "", 7.0); //Spawn halloween bosses at MetalFace's location
              FastFire2("FB.MetalFace.HHHSpawner", "ForceSpawn", "", 9.0); //Spawn halloween bosses at MetalFace's location
              FastFire2("FB.MetalFace.Invuln", "Disable", "", 123.0);
              FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "tank_path_a_250", 126.85);
              FastFire2("mfTelefect", "Start", "", 127.0);
              FastFire2("FB.MetalFace.ReturnSND", "PlaySound", "", 127.0);
            }
            case 4: {
              //PrintToChatAll("Recognized 3 -> 4, OHSHITOHFUCKOHNO");
              AudioManager.setBGM_Global(26, true);
              FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "MFT3x0", 3.0);
              FastFire2("FB.MetalFace.GigaBuster", "ForceSpawn", "", 10.0);
            }
          }
        }
      }
      case 69: {
        int eCount = 0;
        for (int i = 1; i <= MaxClients; i++) {
          if (!IsClientInGame(i))continue;
          if (GetClientTeam(i) == 2 && TF2_GetPlayerClass(i) == TFClass_Engineer)eCount++;
        }
        int sCount = 0;
        int ent = -1
        while ((ent = FindEntityByClassname(ent, "obj_sentrygun")) != -1) {
          if (!IsValidEntity(ent))continue;
          if (this.sOverflow) {
            SetEntProp(ent, Prop_Send, "m_iTeamNum", TFTeam_Blue);
            SetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity", 0);
          }
          if (GetEntProp(ent, Prop_Send, "m_iTeamNum") == 2) {
            sCount++;
            PrintToChatAll("Found %i sentry guns. I'm watching your ass...", sCount);
            if (sCount > eCount * 7) {
              PrintToChatAll("[FARTSY.DLL] TOO MANY SENTRIES DETECTED. PREPARE TO DIE. WOLOLOOOOOOO!");
              this.sOverflow = true;
              ent = -1;
              sCount = 0;
              CreateTimer(3.0, TimedOperator, 42690);
            }
          }
        }
      }
    }
  }
  //For tracking bgm and updating bgm based on player emnity
  void TickBGM() {
    if (this.shouldTickBGM) {
      int iBGM0;
      int iBGM1;
      switch (this.iBossPhase) {
        case 1: {
          iBGM0 = 20;
          iBGM1 = 21;
        }
        case 2: {
          iBGM0 = 22;
          iBGM1 = 23;
        }
        case 3: {
          iBGM0 = 24;
          iBGM1 = 25;
        }
        case 4: {
          iBGM0 = 26;
          iBGM1 = 27;
        }
      }
      for (int i = 0; i < MaxClients; i++) {
        if (EmnityManager[i].IsAboveEmnityThreshold(SuggestEmnityThreshold())) {
          CSEClient(i, BGMArray[iBGM1].realPath, BGMArray[iBGM1].SNDLVL, true, 1, 1.0, 100);
          CSEClient(i, BGMArray[iBGM0].realPath, BGMArray[iBGM0].SNDLVL, true, 1, 0.05, 100);
        }
        else {
          CSEClient(i, BGMArray[iBGM1].realPath, BGMArray[iBGM1].SNDLVL, true, 1, 0.05, 100);
          CSEClient(i, BGMArray[iBGM0].realPath, BGMArray[iBGM0].SNDLVL, true, 1, 1.0, 100);
        }
      }
      AudioManager.setBGM_Global(iBGM0, false);
    }
  }
  //Tank buster thing
  void TickBuster() {
    float cPos[3]; //Client's Position
    float pPos[3]; //Ground Particle Target Position
    float sPos[3]; //Sign/Arrow's Target Position
    float rPos[3];
    float fVelocity[3]; //Velocity for teleporting entities, used for angles as well so we don't need two vectors.
    int iAirParticle = FindEntityByTargetname("FB.TankBuster.AirParticle", "func_brush");
    int iBuster = FindEntityByTargetname("FB.TankBuster", "trigger_hurt");
    int iExploParticle = FindEntityByTargetname("FB.TankBuster.ExplodeParticle", "info_particle_system");
    int iParticle = FindEntityByTargetname("FB.TankBuster.GroundParticle", "info_particle_system");
    int iSign = FindEntityByTargetname("FB.TankBuster.Sign", "func_rotating");
    int iSpin = FindEntityByTargetname("FB.TankBuster.Spinner", "func_rotating");
    GetClientAbsOrigin(this.bustTarget, cPos);
    rPos[0] = cPos[0];
    rPos[1] = cPos[1];
    rPos[2] = cPos[2] + 50.0;
    pPos[0] = cPos[0];
    pPos[1] = cPos[1];
    pPos[2] = cPos[2] + 25.0;
    sPos[0] = cPos[0];
    sPos[1] = cPos[1];
    sPos[2] = cPos[2] + 150.0;
    TeleportEntity(iAirParticle, rPos, fVelocity, fVelocity);
    TeleportEntity(iParticle, pPos, fVelocity, fVelocity);
    TeleportEntity(iExploParticle, pPos, fVelocity, fVelocity);
    TeleportEntity(iBuster, cPos, fVelocity, fVelocity);
    TeleportEntity(iSign, sPos, fVelocity, fVelocity);
    TeleportEntity(iSpin, pPos, fVelocity, fVelocity);
  }
  //Tick client post
  void TickForClient(int client) {
    float cPos[3];
    GetClientAbsOrigin(client, cPos);
    if (GetVectorDistance(cPos, this.fPos) < 500 && client == GetEmnityMax() && this.bDoTankBuster) {
      char sDamage[32];
      this.bustTarget = client;
      this.bDoTankBuster = false;
      int iHealth = GetClientHealth(this.bustTarget);
      Format(sDamage, sizeof(sDamage), "damage %i", RoundToFloor(iHealth * 0.65));
      FastFire2("FB.TankBuster", "AddOutput", sDamage, 0.0);
      FastFire2("FB.TankBusterSND", "PlaySound", "", 1.25);
      FastFire2("FB.TankBuster.GroundParticle", "Start");
      FastFire2("FB.TankBuster.ExplodeSND", "PlaySound", "", 5.50);
      FastFire2("FB.TankBuster.ExplodeParticle", "Start", "", 5.75);
      FastFire2("FB.TankBuster.ExplodeParticle", "Stop", "", 9.0);
      FastFire2("FB.TankBuster", "Enable", "", 5.75);
      FastFire2("FB.TankBuster", "Disable", "", 6.0);
      CreateTimer(1.0, StartTickBuster);
      PrintToServer("Dropping a tank buster on %N, they have %i HP, the tank buster will do %s.", this.bustTarget, iHealth, sDamage);
      CreateTimer(7.0, ResetTickBuster);
    }
  }
  // Tank buster - Potato cannon edition!
  void TickBusterNuclear() {
    float cPos[3]; //Client's Position
    float pPos[3]; //Ground Particle Target Position
    float sPos[3]; //Sign/Arrow's Target Position
    float rPos[3];
    float nPos[3];
    float fVelocity[3]; //Velocity for teleporting entities, used for angles as well so we don't need two vectors.
    int iAirParticle = FindEntityByTargetname("FB.TankBuster.AirParticle", "func_brush");
    int iBuster = FindEntityByTargetname("FB.TankBuster", "trigger_hurt");
    int iExploParticle = FindEntityByTargetname("FB.TankBuster.ExplodeParticle", "info_particle_system");
    int iParticle = FindEntityByTargetname("FB.TankBuster.GroundParticle", "info_particle_system");
    int iSign = FindEntityByTargetname("FB.TankBuster.Sign", "func_rotating");
    int iSpin = FindEntityByTargetname("FB.TankBuster.Spinner", "func_rotating");
    int iNukeTemplate = FindEntityByTargetname("PL.NukeCart", "env_entity_maker");
    GetClientAbsOrigin(this.bustNukeTarget, cPos);
    rPos[0] = cPos[0];
    rPos[1] = cPos[1];
    rPos[2] = cPos[2] + 50.0;
    pPos[0] = cPos[0];
    pPos[1] = cPos[1];
    pPos[2] = cPos[2] + 25.0;
    sPos[0] = cPos[0];
    sPos[1] = cPos[1];
    sPos[2] = cPos[2] + 150.0;
    nPos[0] = cPos[0];
    nPos[1] = cPos[1];
    nPos[2] = cPos[2] + 750.0;
    TeleportEntity(iAirParticle, rPos, fVelocity, fVelocity);
    TeleportEntity(iParticle, pPos, fVelocity, fVelocity);
    TeleportEntity(iExploParticle, pPos, fVelocity, fVelocity);
    TeleportEntity(iBuster, cPos, fVelocity, fVelocity);
    TeleportEntity(iSign, sPos, fVelocity, fVelocity);
    TeleportEntity(iSpin, pPos, fVelocity, fVelocity);
    TeleportEntity(iNukeTemplate, nPos, fVelocity, fVelocity);
  }

  //Boss reached bomb hatch
  void TriggerVictory() {
    this.shouldTick = false;
    this.tickBuster = false;
    float bossOrigin[3];
    float targetOrigin[3];
    int targetEnt = FindEntityByTargetname("capturezone_blue", "func_capturezone");
    GetEntPropVector(targetEnt, Prop_Send, "m_vecOrigin", targetOrigin);
    switch (this.bossID) {
      //Onslaughter Won
      case 0: {
        int bossEnt = FindEntityByTargetname("FB.MetalBase", "base_boss");
        if (bossEnt == -1)return;
        GetEntPropVector(bossEnt, Prop_Send, "m_vecOrigin", bossOrigin);
        if (GetVectorDistance(bossOrigin, targetOrigin) < 2200) {
          //Do something here because Onslaughter is at the pit, victory for boss.
        }
      }
      //Sephiroth Won
      case 1: {
        
      }
      //Metallizer Won
      case 2: {
        int bossEnt = FindEntityByTargetname("FB.MetalBase", "base_boss");
        if (bossEnt == -1)return;
        GetEntPropVector(bossEnt, Prop_Send, "m_vecOrigin", bossOrigin);
        if (GetVectorDistance(bossOrigin, targetOrigin) < 2200) {
          //Do something here because Metal Face is at the pit, victory for boss.
        }
      }
    }
  }
  //Spawn the boi
  void OnSpawned(int boss) {
    this.bossID = boss;
    this.isAlive = true;
    this.shouldTick = true;
    int baseHP = GetBossHealthModifier(640000)
    char bossHP[64];
    Format(bossHP, 64, "%i", baseHP);
    CreateTimer(3.0, OnBossSpawn_Post, baseHP);
    switch (this.bossID) {
      case 0: {
        FastFire2("FB.OnslaughterBase", "SetHealth", bossHP, 0.5);
        FastFire2("FB.OnslaughterBase", "SetHealth", bossHP, 1.5);
        FastFire2("FB.OnslaughterBase", "SetModelScale", "0.19", 0.0);
        FastFire2("FB.BruteJusticeTrain", "TeleportToPathTrack", "tank_path_a_10", 0.0);
        FastFire2("FB.BruteJustice", "Enable", "", 3.0);
        FastFire2("FB.BruteJusticeTrain", "StartForward", "", 3.0);
        FastFire2("FB.BruteJusticeParticles", "Start", "", 3.0);
        CreateTimer(5.0, OnslaughterATK);
        FastFire2("tank_boss", "AddOutput", "rendermode 10", 3.0);
        FastFire2("tank_boss", "AddOutput", "rendermode 10", 7.0);
        CreateTimer(10.0, BossHPTimer);
      }
      case 1: {
        FastFire2("FB.Sephiroth", "Enable", "", 0.0);
        FastFire2("SephMeteor", "SetParent", "FB.Sephiroth", 0.0);
        FastFire2("SephTrain", "SetSpeedReal", "12", 0.0);
        FastFire2("SephTrain", "TeleportToPathTrack", "Seph01", 0.0);
        FastFire2("SephTrain", "StartForward", "", 0.1);
        FastFire2("SephTrain", "SetSpeedReal", "12", 20.5);
        FastFire2("FB.SephParticles", "Start", "", 3.0);
        FastFire2("tank_boss", "AddOutput", "rendermode 10", 3.0);
        FastFire2("tank_boss", "AddOutput", "rendermode 10", 7.0);
        FastFire2("FB.BruteJusticeDMGRelay", "Kill", "", 0.0);
        switch (GetClientCount(true)) {
          case 1: {
            FastFire2("SephTrain", "SetSpeedReal", "40", 23.0);
            FastFire2("tank_boss", "SetHealth", "409600", 1.0);
            FastFire2("FB.SephDMGRelay", "SetHealth", "32768000", 1.0);
          }
          case 2: {
            FastFire2("SephTrain", "SetSpeedReal", "35", 23.0);
            FastFire2("tank_boss", "SetHealth", "614400", 1.0);
            FastFire2("FB.SephDMGRelay", "SetHealth", "32768000", 1.0);
          }
          case 3: {
            FastFire2("SephTrain", "SetSpeedReal", "35", 23.0);
            FastFire2("tank_boss", "SetHealth", "614400", 1.0);
            FastFire2("FB.SephDMGRelay", "SetHealth", "131072000", 1.0);
          }
          case 4: {
            FastFire2("SephTrain", "SetSpeedReal", "30", 23.0);
            FastFire2("tank_boss", "SetHealth", "819200", 1.0);
            FastFire2("FB.SephDMGRelay", "SetHealth", "262144000", 1.0);
          }
          case 5, 6, 7, 8, 9, 10: {
            FastFire2("SephTrain", "SetSpeedReal", "25", 23.0);
            FastFire2("tank_boss", "SetHealth", "819200", 1.0);
            FastFire2("FB.SephDMGRelay", "SetHealth", "655360000", 1.0);
          }
        }
        CreateTimer(30.0, BossHPTimer);
      }
      case 2: {
        this.EmitSpawnSound(2);
        FastFire2("FB.MetalFace.Train", "TeleportToPathTrack", "MFT0x0", 1.0);
        FastFire2("FB.MetalFace.SpawnParticle", "Start", "", 1.0);
        FastFire2("FB.MetalFace.Train", "SetSpeedReal", "1024", 1.0);
        //switch(GetClientCount(true))
        FastFire2("FB.MetalBase", "AddOutput", "health 320000");
      }
    }
  }
}
BOSSHANDLER BossHandler;
BOSSHANDLER Get_Boss_Handler() {
  return BossHandler;
}

//Custom Bosses - For Sephiroth, index may vary because we don't want to be able to execute code that shouldn't be executed. Example SephBoss[5].attack would run code that's part of SephBoss[4].attack. Also, this.fireCount is how many times FastFire should run, as well being added to this.index to determine what it does.
enum struct CUSTOMBOSS {
  bool shouldNuke;
  char ChatMessage[128];
  char fireString[128];
  char sound[64];
  int index;
  int fireCount;
  void attack() {
    if (this.shouldNuke) {
      CreateTimer(1.0, NukeTimer, 2);
      CreateTimer(7.0, TimedOperator, 11);
    }
    if (!StrEqual(this.ChatMessage, ""))CPrintToChatAll(this.ChatMessage);
    for (int i = 0; i < this.fireCount; i++)FastFire2(this.fireString[this.index + i]);
  }
  void Reset() {
    this.shouldNuke = false;
    canCrusaderNuke = false;
  }
}
CUSTOMBOSS SephBoss[12];
CUSTOMBOSS HWBoss[12];
//CUSTOMBOSS Onslaughter[10];

//Used for nukes, obviously
public Action NukeTimer(Handle timer, int type) {
  if (canCrusaderNuke || SephBoss[0].shouldNuke) {
    CustomSoundEmitter(SFXArray[8].realPath, 65, false, 0, 1.0, 100);
    FastFire2(type == 1 ? "FB.CrusaderNuke" : "SephNuke", "ForceSpawn");
    CreateTimer(GetRandomFloat(1.5, 3.0), NukeTimer, type);
  }
  return Plugin_Stop;
}

//Player specific async tick process
public Action OnPlayerRunCmd(int client, int &buttons, int &impulse, float vel[3], float angles[3], int &weapon) {
  if (BossHandler.shouldTick)BossHandler.TickForClient(client);
  return Plugin_Continue;
}

stock int SuggestEmnityThreshold() {
  switch (GetClientCount(true)) {
    case 1: {
      return 75;
    }
    case 2: {
      return 60;
    }
    case 3: {
      return 50;
    }
    case 4: {
      return 35;
    }
    case 5: {
      return 25;
    }
    case 6: {
      return 20;
    }
    case 7: {
      return 15;
    }
    case 8, 9, 10: {
      return 10;
    }
  }
  return -1;
}

//Used for emitting boss sounds
public Action QueueBossSound(Handle timer, int sound) {
  AssLogger(LOGLVL_DEBUG, "QueueBossSound played %i", sound);
  switch (sound) {
    case 2: {
    }
    case 10: {
      CustomSoundEmitter(SFXArray[71].realPath, 65, false, 0, 1.0, 100); //Interrupting
      CreateTimer(3.1, QueueBossSound, GetRandomInt(16, 22));
    }
    case 11: {
      CustomSoundEmitter(SFXArray[69].realPath, 65, false, 0, 1.0, 100); //FunnyLooking
      CreateTimer(3.0, QueueBossSound, GetRandomInt(18, 21));
    }
    case 12: {
      CustomSoundEmitter(SFXArray[75].realPath, 65, false, 0, 1.0, 100); //Racket
      CreateTimer(3.2, QueueBossSound, GetRandomInt(16, 22));
    }
    case 13: {
      CustomSoundEmitter(SFXArray[78].realPath, 65, false, 0, 1.0, 100); //WhatDoing
    }
    case 14: {
      CustomSoundEmitter(SFXArray[79].realPath, 65, false, 0, 1.0, 100); //WhateverDoing
      CreateTimer(4.3, QueueBossSound, GetRandomInt(16, 17));
    }
    case 15: {
      CustomSoundEmitter(SFXArray[72].realPath, 65, false, 0, 1.0, 100); //Join In
      CreateTimer(3.2, QueueBossSound, 16);
    }
    //pt 2
    case 16: {
      CustomSoundEmitter(SFXArray[70].realPath, 65, false, 0, 1.0, 100); //Good show
    }
    case 17: {
      CustomSoundEmitter(SFXArray[67].realPath, 65, false, 0, 1.0, 100); //Carryon
    }
    case 18: {
      CustomSoundEmitter(SFXArray[66].realPath, 65, false, 0, 1.0, 100); //Carnage
    }
    case 19: {
      CustomSoundEmitter(SFXArray[73].realPath, 65, false, 0, 1.0, 100); //Lets Start
    }
    case 20: {
      CustomSoundEmitter(SFXArray[74].realPath, 65, false, 0, 1.0, 100); //Make Way
    }
    //Attack sounds, but also shared with spawn sounds
    case 21: {
      CustomSoundEmitter(SFXArray[76].realPath, 65, false, 0, 1.0, 100); //SliceYou
    }
    case 22: {
      CustomSoundEmitter(SFXArray[77].realPath, 65, false, 0, 1.0, 100); //Stroll
    }
    //Attack sounds, used for random grunts
    case 23: {
      CustomSoundEmitter(SFXArray[80].realPath, 65, false, 0, 1.0, 100); //gah
    }
    case 24: {
      CustomSoundEmitter(SFXArray[81].realPath, 65, false, 0, 1.0, 100); //guh
    }
    case 25: {
      CustomSoundEmitter(SFXArray[82].realPath, 65, false, 0, 1.0, 100); //hah
    }
    //Defeats
    case 26: {
      CustomSoundEmitter(SFXArray[83].realPath, 65, false, 0, 1.0, 100); //BadSide
    }
    case 27: {
      CustomSoundEmitter(SFXArray[84].realPath, 65, false, 0, 1.0, 100); //NotFair
    }
    case 28: {
      CustomSoundEmitter(SFXArray[85].realPath, 65, false, 0, 1.0, 100); //Nothing
    }
    case 29: {
      CustomSoundEmitter(SFXArray[86].realPath, 65, false, 0, 1.0, 100); //Scream
    }
    case 30: {
      CustomSoundEmitter(SFXArray[87].realPath, 65, false, 0, 1.0, 100); //ToGo
    }
    //Special attack sounds like phase changes
    case 31: {
      CustomSoundEmitter(SFXArray[88].realPath, 65, false, 0, 1.0, 100); //Clever
    }
    case 32: {
      CustomSoundEmitter(SFXArray[89].realPath, 65, false, 0, 1.0, 100); //Cmon
    }
    case 33: {
      CustomSoundEmitter(SFXArray[90].realPath, 65, false, 0, 1.0, 100); //Die
    }
    case 34: {
      CustomSoundEmitter(SFXArray[91].realPath, 65, false, 0, 1.0, 100); //GetOff
    }
    case 35: {
      CustomSoundEmitter(SFXArray[92].realPath, 65, false, 0, 1.0, 100); //HearYouScream
    }
    case 36: {
      CustomSoundEmitter(SFXArray[93].realPath, 65, false, 0, 1.0, 100); //IsntOver
    }
    case 37: {
      CustomSoundEmitter(SFXArray[94].realPath, 65, false, 0, 1.0, 100); //Like
    }
    case 38: {
      CustomSoundEmitter(SFXArray[95].realPath, 65, false, 0, 1.0, 100); //Little
    }
    case 39: {
      CustomSoundEmitter(SFXArray[96].realPath, 65, false, 0, 1.0, 100); //Pay
    }
    case 40: {
      CustomSoundEmitter(SFXArray[97].realPath, 65, false, 0, 1.0, 100); //Pointers
    }
    case 41: {
      CustomSoundEmitter(SFXArray[98].realPath, 65, false, 0, 1.0, 100); //Seat
    }
    case 42: {
      CustomSoundEmitter(SFXArray[99].realPath, 65, false, 0, 1.0, 100); //Size
    }
    case 43: {
      CustomSoundEmitter(SFXArray[100].realPath, 65, false, 0, 1.0, 100); //There
    }
    case 44: {
      CustomSoundEmitter(SFXArray[101].realPath, 65, false, 0, 1.0, 100); //TimeToDie
    }
    case 45: {
      CustomSoundEmitter(SFXArray[102].realPath, 65, false, 0, 1.0, 100); //Way
    }
    //Ultimates
    case 46: {
      CustomSoundEmitter(SFXArray[103].realPath, 65, false, 0, 1.0, 100); //FinalWords
    }
    case 47: {
      CustomSoundEmitter(SFXArray[104].realPath, 65, false, 0, 1.0, 100); //PartingGift
    }
    case 48: {
      CustomSoundEmitter(SFXArray[105].realPath, 65, false, 0, 1.0, 100); //WatchAndLearn
    }
  }
  return Plugin_Stop;
}

//When a boss spawns - delayed by 3s to give vars time to update.
public Action OnBossSpawn_Post(Handle timer, int hp) {
  SCR_SendEvent("Boss Spawned", BossHandler.bossID == 0 ? "Onslaughter has spawned with %i HP!" : BossHandler.bossID == 1 ? "Sephiroth has spawned with %i HP!" : "Metal Face has spawned with %i HP!", hp); return Plugin_Stop;
}

//Teleport all entities to the client every frame
public Action StartTickBuster(Handle timer) {
  BossHandler.tickBuster = true;
  return Plugin_Stop;
}

//Used for stopping tank busters from ticking
public Action ResetTickBuster(Handle time) {
  BossHandler.tickBuster = false;
  float fAirPart[3] = {
    -1437.270019,
    -3906.739990,
    -1334.989990
  };
  float fVelocity[3];
  int iAirParticle = FindEntityByTargetname("FB.TankBuster.AirParticle", "func_brush");
  int iBuster = FindEntityByTargetname("FB.TankBuster", "trigger_hurt");
  int iExploParticle = FindEntityByTargetname("FB.TankBuster.ExplodeParticle", "info_particle_system");
  int iParticle = FindEntityByTargetname("FB.TankBuster.GroundParticle", "info_particle_system");
  int iSign = FindEntityByTargetname("FB.TankBuster.Sign", "func_rotating");
  int iSpin = FindEntityByTargetname("FB.TankBuster.Spinner", "func_rotating");
  TeleportEntity(iAirParticle, fAirPart, fVelocity, fVelocity);
  TeleportEntity(iParticle, fAirPart, fVelocity, fVelocity);
  TeleportEntity(iExploParticle, fAirPart, fVelocity, fVelocity);
  TeleportEntity(iBuster, fAirPart, fVelocity, fVelocity);
  TeleportEntity(iSign, fAirPart, fVelocity, fVelocity);
  TeleportEntity(iSpin, fAirPart, fVelocity, fVelocity);
  return Plugin_Stop;
}

//Spawn metal face
public Action DelayedMetalFace(Handle timer) {
  AudioManager.setBGM_Global(20, true);
  CustomSoundEmitter(SFXArray[68].realPath, 65, false, 0, 1.0, 100); //Evil Laugh
  CreateTimer(3.2, QueueBossSound, GetRandomInt(10, 14));
  sudo(1000);
  BossHandler.shouldTickBGM = true;
  return Plugin_Stop;
}

// Calculate bonus boss health from client weapon stats
public int GetBossHealthModifier(int baseHP) {
  float tD = GetTotalWeaponDamageBonus();
  PrintToServer("Got Attribute Damage Bonus %f", tD);
  return RoundFloat(baseHP * (tD / 1.5));
}