char SUDO_VER[8] = "1.4.1";
void OnSudoStartup() { LightLogger(LOGLVL_INFO, "[Sudo @ v%s] Module is starting up!", SUDO_VER); RegServerCmd("hr_send", Command_Operator, "Serverside only. Does nothing when executed as client."); }

/** Interfaces our command to the actual netcode */
public Action Command_Operator(int args) {
  char arg1[16]; GetCmdArg(1, arg1, sizeof(arg1));
  sudo(StringToInt(arg1));
  return Plugin_Continue;
}

/** This is the plugin netcode. This handles literally everything. */
void sudo(int task) {
  LightLogger(LOGLVL_DEBUG, "Calling sudo with %i", task);
  switch (task) {
    //When the map is complete
    case 0: {
      CPrintToChatAll(core.tacobell ? "WOW, YOU DID IT! The server will restart in 30 seconds, prepare to do it again! LULW" : "{darkviolet}[{forestgreen}CORE{darkviolet}] {white}YOU HAVE SUCCESSFULLY COMPLETED PROF'S ASS ! THE SERVER WILL RESTART IN 10 SECONDS.");
      CreateTimer(10.0, TimedOperator, 100);
    }
    case 10100: { WDATA.tickWeather = true; }
    case 1111: { WaveSystem().run(); }
    case 2222: { WeatherManager.doLightning(); }
    case 3333: { Photon.Fire("hr_cannon_template", "ForceSpawn"); Photon.Fire("hr_cannon*_yaw", "SetPosition", "0.5", 0.1); }
    case 4444: { CustomSoundEmitterEX(SFXArray[122].realPath, _, _, SFXArray[122].SNDLVL, false, 0, 1.0, 100); sudo(4445); CreateTimer(5.55, TimedOperator, 4444); }
    case 4445: { tt = !tt; Photon.Fire("envglow", tt ? "ShowSprite" : "HideSprite"); }
    case 5555: { Photon.Fire("hr_cannon*_yaw", "SetPosition", "0.5"); }
    case 5556: { Photon.Fire("hr_cannon*_yaw", "SetPosition", "1.0"); }
    case 5557: { Photon.Fire("hr_cannon*_pitch", "SetPosition", "0.5"); }
    case 5600: { Photon.Fire("hr_cannon*_yaw", "SetPosition", "0.0"); }
    case 5750: { Photon.Fire("hr_cannon*_yaw", "SetPosition", "-0.5"); }
    case 5900: { Photon.Fire("hr_cannon*_pitch", "SetPosition", "0.0"); }
    //Prepare yourself!
    case 1: {
      WaveSystem().init();
    }
    case 2 : { WeatherManager.setCanTornado(true); WaveSystem().SetActive(true); WeatherManager.IssueTornadoWarning(); }
    case 2100 : { WeatherManager.Dissipate(); }
    case 2200: {
      int target = Photon.Create("info_target", "ImADumbass");
      SetVariantString("OnUser2 weather.LightningLaser:TurnOn::0.0:1");
      AcceptEntityInput(target, "AddOutput");
      SetVariantString("OnUser2 weather.LightningLaser:TurnOff::0.1:1");
      AcceptEntityInput(target, "AddOutput");
    }
    case 2201: { Photon.Fire("ImADumbass", "FireUser2"); }
    case 2300: { Photon.Fire("ImADumbass", "Kill"); }
    case 2500: {
      int l1 = Photon.Create("env_laser", "SpLaser00");
      int l2 = Photon.Create("info_target", "SpLaser00TGT");
      TeleportEntity(l1, {-1503.445923, 1369.923584, -45.968681});
      TeleportEntity(l2, {-1503.445923, 1369.923584, 50.000000});
      Photon.SetKv(l1, "LaserTarget", "SpLaser00TGT");
      Photon.SetKv(l1, "rendercolor", "255 255 0");
      Photon.SetKv(l1, "width", "20");
      Photon.SetKv(l1, "texture", "sprites/laserbeam.spr");
      Photon.SetKv(l1, "NoiseAmplitude", "1");
    }
    case 2501: {
      float fStart[3] = {-1500.0, 1375.0, -50.0}; float fEnd[3] = {-1500.0, 1375.0, 75.0};
      int ent = CreateEntityByName("env_beam");
      if (ent != -1) {
          TeleportEntity(ent, fStart, NULL_VECTOR, NULL_VECTOR);
          SetEntityModel(ent, "sprites/laserbeam.spr"); // This is where you would put the texture, ie "sprites/laser.vmt" or whatever.
          SetEntPropVector(ent, Prop_Data, "m_vecEndPos", fEnd);
          DispatchKeyValue(ent, "targetname", "beam2");
          DispatchKeyValue(ent, "rendercolor", "255 255 255");
          DispatchKeyValue(ent, "renderamt", "100");
          DispatchSpawn(ent);
          SetEntPropFloat(ent, Prop_Data, "m_fWidth", 4.0); // how big the beam will be, i.e "4.0"
          SetEntPropFloat(ent, Prop_Data, "m_fEndWidth", 4.0); // same as above
          ActivateEntity(ent);
          AcceptEntityInput(ent, "TurnOn");
      }
    }
    case 2550: { Photon.Fire("SpLaser00", "TurnOn"); }
    case 3000: {
    float flStart[3] = {-1500.0, 1375.0, -50.0};
    int ent = Photon.Find("phys_model", "generic_actor") == -1 ? CreateEntityByName("generic_actor") : -1;
    if (ent != -1) {
      DispatchKeyValue(ent, "targetname", "phys_model");
      DispatchKeyValue(ent, "model", "models/player/heavy.mdl");
      DispatchKeyValue(ent, "rendermode", "10");
      DispatchKeyValue(ent, "movetype", "4");
      DispatchSpawn(ent);
      ActivateEntity(ent);
      TeleportEntity(ent, flStart, NULL_VECTOR, NULL_VECTOR);
      int ent2 = Photon.Find("Spec.Burrito", "prop_dynamic");
      TeleportEntity(ent2, flStart, NULL_VECTOR, NULL_VECTOR);
      Photon.Fire("Spec.Burrito", "SetParent", "phys_model");
    }
 }
 case 3001: {
  int ent = Photon.Find("phys_model", "generic_actor");
  if (ent == -1) { ThrowError("Entity not found!!!"); return; }
  SetEntityMoveType(ent, MOVETYPE_FLY);
  SetEntPropFloat(ent, Prop_Data, "m_flGravity", 0.0);
  SetEntPropVector(ent, Prop_Data, "m_vecVelocity", {0.0,0.0,-15.0});
  TeleportEntity(ent, NULL_VECTOR, NULL_VECTOR, {0.0,0.0,-15.0});
 }
 case 3002:{
  for (int i = 0; i <= sizeof(WBLasers[0]); i++) {
    int ent = CreateEntityByName("env_beam");
    if (ent != -1) {
      TeleportEntity(ent, WBLasers[0][i].sPos, NULL_VECTOR, NULL_VECTOR);
      SetEntityModel(ent, "sprites/laserbeam.spr"); // This is where you would put the texture, ie "sprites/laser.vmt" or whatever.
      SetEntPropVector(ent, Prop_Data, "m_vecEndPos", WBLasers[0][i].ePos);
      DispatchKeyValue(ent, "targetname", WBLasers[0][i].tName);
      DispatchKeyValue(ent, "rendercolor", WBLaserColors[0].cRGB);
      DispatchKeyValue(ent, "renderamt", "100");
      DispatchSpawn(ent);
      SetEntPropFloat(ent, Prop_Data, "m_fWidth", 7.0); // how big the beam will be, i.e "4.0"
      SetEntPropFloat(ent, Prop_Data, "m_fEndWidth", 7.0); // same as above
      ActivateEntity(ent);
      AcceptEntityInput(ent, "TurnOn");
    }
  }
 }
 case 3003:{
  for (int i = 0; i <= sizeof(WBLasers[1]); i++) {
    int ent = CreateEntityByName("env_beam");
    if (ent != -1) {
      TeleportEntity(ent, WBLasers[1][i].sPos, NULL_VECTOR, NULL_VECTOR);
      SetEntityModel(ent, "sprites/laserbeam.spr"); // This is where you would put the texture, ie "sprites/laser.vmt" or whatever.
      SetEntPropVector(ent, Prop_Data, "m_vecEndPos", WBLasers[1][i].ePos);
      DispatchKeyValue(ent, "targetname", WBLasers[1][i].tName);
      DispatchKeyValue(ent, "rendercolor", WBLaserColors[1].cRGB);
      DispatchKeyValue(ent, "renderamt", "100");
      DispatchSpawn(ent);
      SetEntPropFloat(ent, Prop_Data, "m_fWidth", 7.0); // how big the beam will be, i.e "4.0"
      SetEntPropFloat(ent, Prop_Data, "m_fEndWidth", 7.0); // same as above
      ActivateEntity(ent);
      AcceptEntityInput(ent, "TurnOn");
    }
  }
 }
 case 3004:{
  for (int i = 0; i <= sizeof(WBLasers[2]); i++) {
    int ent = CreateEntityByName("env_beam");
    if (ent != -1) {
      TeleportEntity(ent, WBLasers[2][i].sPos, NULL_VECTOR, NULL_VECTOR);
      SetEntityModel(ent, "sprites/laserbeam.spr"); // This is where you would put the texture, ie "sprites/laser.vmt" or whatever.
      SetEntPropVector(ent, Prop_Data, "m_vecEndPos", WBLasers[2][i].ePos);
      DispatchKeyValue(ent, "targetname", WBLasers[2][i].tName);
      DispatchKeyValue(ent, "rendercolor", WBLaserColors[2].cRGB);
      DispatchKeyValue(ent, "renderamt", "100");
      DispatchSpawn(ent);
      SetEntPropFloat(ent, Prop_Data, "m_fWidth", 7.0); // how big the beam will be, i.e "4.0"
      SetEntPropFloat(ent, Prop_Data, "m_fEndWidth", 7.0); // same as above
      ActivateEntity(ent);
      AcceptEntityInput(ent, "TurnOn");
    }
  }
 }
  //Force Tornado
    case 3: {
      if (WeatherManager.CanFormTornado()) {
        CreateTimer(0.1, TimedOperator, 41);
        PrintCenterTextAll("OH NOES... PREPARE YOUR ANUS!");
      } else PrintToServer("Error spawning manual tornado... Perhaps we are not in a wave, tornadoes are banished, or a tornado has already spawned???");
      return;
    }
  //Signal that previous boss should spawn.
    case 4: {
      core.waveFlags--;
    }
  //Signal that a boss should spawn
    case 5: {
      if (core.waveFlags < 0) core.waveFlags = 0;
      switch (core.waveFlags) {
        case 0: {
          PrintToChatAll("Caught unhandled exception: core.waveFlags 0 but opcode 5 was invoked.");
          return;
        }
        case 1: {
          BossHandler.OnSpawned(0);
        }
        case 2: {
          BossHandler.OnSpawned(1);
        }
      }
    }
  //Signal that next boss should spawn
    case 6: {
      core.waveFlags++;
    }
  //Signal to fastforward boss spawn.
    case 7: {
      core.waveFlags = 2;
      if (WaveSystem().GetWave() == 8 && !WaveSystem().IsTacoBell()) {
        sudo(1001);
        CreateTimer(0.0, TimedOperator, 8);
      }
    }
  //When a tornado intersects a tank.
    case 8: {
      Photon.Fire("FB.FakeTankSpawner", "ForceSpawn", "", 0.1);
    }
    case 9 : { Bombs.OnBombReady(); }
  //Client was Sacrificed.
    case 10: {
      core.sacrificedByClient = true;
    }
  //Damage relay took damage
    case 11: {
      Photon.Fire("TankRelayDMG", "Enable", "", 0.0);
      Photon.Fire("TankRelayDMG", "Disable", "", 0.5);
    }
  //dmg relay was killed
    case 12: {
      Photon.Fire("tank_boss", "SetHealth", "1", 0.0);
      Photon.Fire("TankRelayDMG", "Enable", "", 0.1);
      Photon.Fire("TankRelayDMG", "Enable", "", 0.5);
      Photon.Fire("TankRelayDMG", "Enable", "", 1.0);
      Photon.Fire("TankRelayDMG", "Disable", "", 60.0);
    }
  //Tank Destroyed (+1), includes disabling onslaughter.
    case 13: {
      switch (core.waveFlags) {
        case 0: {
          CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}A tank has been destroyed. ({limegreen}+1 pt{white})");
          ass_sacPoints++;
        }
        case 1: {
          core.waveFlags = 0;
          CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {red}ONSLAUGHTER {white}has been destroyed. ({limegreen}+25 pts{white})");
          BossHandler.OnKilled();
        }
        case 2: {
          CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {red}SEPHIROTH {white}has been destroyed. ({limegreen}+100 pts{white})");
          Photon.Fire("TankRelayDMG", "Enable", "", 1.0);
          Photon.Fire("TankRelayDMG", "Disable", "", 60.0);
          Photon.Fire("FB.Sephiroth", "Disable", "", 0.0);
          Photon.Fire("SephTrain", "TeleportToPathTrack", "Seph01", 0.0);
          Photon.Fire("SephTrain", "Stop", "", 0.0);
          SephBoss[0].shouldNuke = false;
          ass_sacPoints += 100;
          core.waveFlags = 0;
          WeatherManager.setCanTornado(false);
          BossHandler.OnKilled();
        }
      }
      return;
    }
  //Bomb Reset (+5)
    case 14: {
      core.bombReset = true;
      ass_sacPoints += 5;
    }
  //Bomb Deployed
    case 15: {
      Photon.Fire("FB.PayloadWarning", "Disable", "", 0.0);
      if (WaveSystem().IsActive()) {
        if (g_BombIndex == 69) {
          Bombs.explode(7, false);
          BombRegistry[0].stateMax = 64;
          CreateTimer(5.0, TimedOperator, 99);
        }
        Bombs.explode(Bombs.getCurBombID(), true);
        BombRegistry[0].stateMax += 10;
        if (g_BombIndex >= BombRegistry[0].stateMax) return;
        else g_BombIndex += 2;
      }
      Photon.SetEntityLife("BombPush", 1.0);
      Photon.SetEntityLife("BombPushRegen", 1.0);
      Photon.SetEntityLife("BombTrain0x*", 1.0);
      Photon.SetEntityLife("BombTrainDamage", 1.0);
      return;
    }
  //Tank deployed its bomb or boss reached hatch
    case 16: {
      sudo(18);
      CPrintToChatAll("{darkviolet}[{hikarired}CORE{darkviolet}] {white}A tank has deployed its bomb!");
      return;
    }
    case 17 : {
      sudo(18);
      CPrintToChatAll("{darkviolet}[{hikarired}CORE{darkviolet}] {white}A robot has deployed its bomb!");
      return;
    }
    case 18: {
      if(!WaveSystem().IsActive()) { CPrintToChatAll("{hikarired} PLEASE DO NOT ATTEMPT TO USE OPCODE 18 WITHOUT A WAVE."); return; }
      SetupAttackZone(9000000.0, DMG_BLAST, g_HatchExplode, _, _, 1.0, 4.0);
      Photon.Fire("bots_win", "RoundWin", "", 1.0);
      Photon.Fire("hatch_magnet_pit", "Enable", "", 1.0);
      Photon.Fire("cap_hatch_destroy_animated_prop", "Enable", "", 1.0);
      Photon.Fire("cap_hatch_destroy_delete_prop", "Kill", "", 1.0);
      Photon.Fire("end_pit_destroy_particle", "Start", "", 1.0);
      Photon.Fire("trigger_hurt_hatch_fire", "Enable", "", 1.0);
      Photon.Fire("cap_hatch_glasswindow", "Break", "", 1.0);
      Photon.Fire("hatch_explo_kill_players", "Disable", "", 1.50);
      Photon.Fire("hatch_magnet_pit", "Disable", "", 5.0);
      Photon.Fire("cap_hatch_destroy_animated_prop", "SetAnimation", "explode", 1.0);
      Photon.Fire("tank_boss", "DestroyIfAtCapturePoint", "1", 1.0);
      Photon.Fire("tank_boss", "DestroyIfAtCapturePoint", "2", 1.0);
      Photon.Fire("tank_boss", "DestroyIfAtCapturePoint", "3", 1.0);
      EmitSoundToAll("MVM.BombExplodes");
      return;
    }
  //Shark Enable & notify bomb push began
    case 20: {
      Bombs.StartPush();
    }
  //Shark Disable
    case 21: {
      BombRegistry[0].isMoving = false;
      core.canSENTShark = false;
    }
  //HINDENBOOM ACTIVATION
    case 28: {
      EmitSoundToAll(SFXArray[5].realPath);
      Photon.Fire("HindenTrain", "StartForward");
      Photon.Fire("DeliveryBurg", "Lock");
      Photon.Fire("Boom", "Enable");
      Photon.Fire("Bombs.TheHindenburg", "Enable");
      Photon.Fire("Boom", "Disable", "", 1.0);
    }
  //HINDENBOOM!!!
    case 29: {
      CPrintToChatAll("{darkviolet}[{red}CORE{darkviolet}] {white}OH GOD, THEY'RE {red}CRASHING THE HINDENBURG{white}!!!");
      EmitSoundToAll(SFXArray[36].realPath);
      CreateTimer(4.0, TimedOperator, 21);
      CreateTimer(7.0, TimedOperator, 37);
      Photon.Fire("FrontMapShake", "Frequency", "16", 6.9);
      Photon.Fire("FrontMapShake", "Amplitude", "20", 6.9);
      Photon.Fire("LargeExplosion", "Explode", "", 7.0);
      Photon.Fire("FrontMapShake", "StartShake", "", 7.0);
      Photon.Fire("NukeAll", "Enable", "", 7.0);
      Photon.Fire("FB.Fade", "Fade", "", 7.0);
      Photon.Fire("Bombs.TheHindenburg", "Disable", "", 7.0);
      Photon.Fire("HindenTrain", "TeleportToPathTrack", "Hinden01", 7.0);
      Photon.Fire("HindenTrain", "Stop", "", 7.0);
      Photon.Fire("NukeAll", "Disable", "", 9.0);
      CreateTimer(8.0, TimedOperator, 99);
      g_BombIndex = 4;
      BombRegistry[0].stateMax = 8;
    }
  //Bath Salts spend
    case 30: {
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}INSTANT BATH SALT DETONATION! BOTS ARE FROZEN FOR 10 SECONDS! ({red}-10 pts{white})");
      ServerCommand("sm_freeze @blue 10");
      ass_sacPoints -= 10;
      Bombs.explode(3, false);
    }
  //Goob/Kirb spend
    case 31: {
      int i = GetRandomInt(1, 2);
      if (i == 1) CreateTimer(1.5, TimedOperator, 21);
      Photon.Fire(i == 1 ? "FB.GiantGoobTemplate" : "FB.BlueKirbTemplate", "ForceSpawn");
      CPrintToChatAll(i == 1 ? "{darkviolet}[{forestgreen}CORE{darkviolet}] {white}GOOBBUE COMING IN FROM ORBIT! ({red}-20 pts{white})" : "{darkviolet}[{forestgreen}CORE{darkviolet}] {white}BLUE KIRBY FALLING OUT OF THE SKY! ({red}-20 pts{white})");
      CustomSoundEmitterEX(i == 1 ? SFXArray[51].realPath : SFXArray[21].realPath, _, _, 65, false, 0, 1.0, 100);
      ass_sacPoints -= 20;
    }
  //35K ubup cash spend
    case 32: {
      if (!core.doFailsafe) {
        core.doFailsafe = true;
        CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}Wave fail-safe activated! ({red}-30 pts{white})");
        ass_sacPoints -= 30;
      } else return;
    }
  //Explosive paradise spend
    case 33: {
      CustomSoundEmitterEX(SFXArray[10].realPath, _, _, 65, false, 0, 1.0, 100);
      Photon.Fire("FB.FadeBLCK", "Fade");
      ServerCommand("sm_evilrocket @blue");
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}We're spending most our lives living in an EXPLOSIVE PARADISE! Robots will be launched into orbit, too! ({red}-40 pts{white})");
    //Add this to explosion array?
      Photon.Fire("NukeAll", "Enable", "", 11.50);
      Photon.Fire("HUGEExplosion", "Explode", "", 11.50);
      Photon.Fire("FB.Fade", "Fade", "", 11.50);
      Photon.Fire("FB.ShakeBOOM", "StartShake", "", 11.50);
      Photon.Fire("NukeAll", "Disable", "", 12.30);
      ass_sacPoints -= 40;
    }
  //Banish Tornadoes
    case 34: {
      if (WeatherManager.CanFormTornado()) ass_sacPoints -= 50;
      CPrintToChatAll(WeatherManager.CanFormTornado() ? "{darkviolet}[{forestgreen}CORE{darkviolet}] {white}A PINK KIRBY HAS BANISHED TORNADOES FOR THIS WAVE! ({red}-50 pts{white})" : !WeatherManager.canTornado() ? "{red}TTV/professorfartsalot {white}: Please do not, the tornado button. Thanks. ({red} -0 pts{white})" : WeatherManager.hasTornado() ? "{red}TTV/professorfartsalot {white}:  You fool. It is much too late for you. ({red} -0 pts{white})" : "{red}TTV/professorfartsalot {white}:  To be honest, I never thought I'd get this far... ({red} -0 pts{white})");
      WeatherManager.setCanTornado(WeatherManager.hasTornado() ? true : false);
    }
  //Ass Gas spend
    case 35: {
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}ANYTHING BUT THE ASS GAS!!!! ({red}-60 pts{white})");
      ass_sacPoints -= 60;
      SetupAttackZone(400.0, DMG_DROWNRECOVER, g_FrontRottenburg, true, GetRandomInt(45, 75), 0.1, 4.1);
      Photon.Fire("FB.ShakeBOOM", "StartShake", "", 0.1);
      Photon.Fire("FB.ShakeBOOM", "StopShake", "", 4.1);
    }
  //Nuclear fallout spend
    case 36: {
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}INSTANT FAT MAN DETONATION! ({red}-70 pts{white})");
      ass_sacPoints -= 70;
      Bombs.explode(7, false);
    }
  //Meteor shower spend
    case 37: {
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}COSMIC DEVASTATION IMMINENT. ({red}-80 pts{white})");
      ass_sacPoints -= 80;
      core.canSENTMeteors = true;
      CreateTimer(1.0, SENTMeteorTimer);
      CreateTimer(30.0, TimedOperator, 12);
    }
  //300K UbUp Cash
    case 38: {
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}150K UbUp Cash Granted to {red}RED{white}! ({red}-90 pts{white})");
      ass_sacPoints -= 90;
      ServerCommand("sm_addcash @red 300000");
    }
  //Fartsy of the Seventh Taco Bell
    case 39: {
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}NOW PRESENTING... PROFESSOR FARTSALOT OF THE SEVENTH TACO BELL! ({red}-100 points{white})");
      ass_sacPoints -= 100;
      BombRegistry[0].stateMax = 69;
      g_BombIndex = 69;
      Photon.Fire("Delivery", "Unlock");
      Photon.Fire("BombExplo*", "Disable");
      Photon.Fire("Bombs*", "Disable");
      Photon.Fire("Bombs.Professor", "Enable", "", 3.0);
    }
  //Found blue ball
    case 40: {
      isGoobbue = false;
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {forestgreen}What on earth IS that? It appears to be a... \x075050FFBLUE BALL{white}!");
      CustomSoundEmitterEX(SFXArray[21].realPath, _, _, 65, false, 0, 1.0, 100);
      Photon.Fire("FB.BlueKirbTemplate", "ForceSpawn", "", 4.0);
      CreateTimer(4.0, TimedOperator, 21);
    }
  //Found burrito
    case 41: {
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {forestgreen}Why would you even eat {red}The Forbidden Burrito{white}?");
      CustomSoundEmitterEX("vo/sandwicheat09.mp3", _, _, 65, false, 0, 1.0, 100);
      SetupAttackZone(2400.0, DMG_BURN, g_FrontRottenburg, true, GetRandomInt(25, 75), 4.0, 8.0);
    }
  //Found goobbue
    case 42: {
      isGoobbue = true;
      CustomSoundEmitterEX(SFXArray[51].realPath, _, _, 65, false, 0, 1.0, 100);
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {forestgreen}ALL HAIL \x07FF00FFGOOBBUE{forestgreen}!");
      CreateTimer(4.0, TimedOperator, 21);
      Photon.Fire("FB.GiantGoobTemplate","ForceSpawn", "", 4.0);
    }
  //Found Mario
    case 43: {
      CustomSoundEmitterEX(SFXArray[52].realPath, _, _, 65, false, 0, 1.0, 100);
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {forestgreen}Welp, someone is playing \x0700FF00Mario{white}...");
    }
  //Found Waffle
    case 44: {
      CustomSoundEmitterEX(SFXArray[53].realPath, _, _, 65, false, 0, 1.0, 100);
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {forestgreen}Oh no, someone has found and (probably) consumed a {red}WAFFLE OF MASS DESTRUCTION{white}!");
      // Yup, another one for the explosion array
      SetupAttackZone(2000000.0, DMG_RADIATION, g_FrontRottenburg, _, GetRandomInt(10, 50), 10.3, 13.7);
      Photon.Fire("ShakeGlobal", "Amplitude", "16", 10.1);
      Photon.Fire("ShakeGlobal", "Frequency", "32", 10.1);
      Photon.Fire("ShakeGlobal", "StartShake", _, 10.3);
      Photon.Fire("FB.Fade", "Fade", "", 10.3);
    }
    case 75:{
      char onsHP[32];
      Format(onsHP, 32, "%i", GetBossHealthModifier(640000));
      PrintToChatAll(onsHP);
    }
  //Prev wave
    case 98: {
      int prev_wave = GetCurWave() - 1;
      if (prev_wave < 1) return;
      JumpToWave(prev_wave);
    }
  //Next wave
    case 99: {
      int ent = FindEntityByClassname(-1, "tf_objective_resource");
      if (ent == -1) {
        LogMessage("tf_objective_resource not found");
        return;
      }
      int max_wave = GetEntData(ent, FindSendPropInfo("CTFObjectiveResource", "m_nMannVsMachineMaxWaveCount"));
      int next_wave = GetCurWave() + 1;
      if (next_wave > max_wave) {
        int flags = GetCommandFlags("tf_mvm_force_victory");
        SetCommandFlags("tf_mvm_force_victory", flags & ~FCVAR_CHEAT);
        ServerCommand("tf_mvm_force_victory 1");
        FakeClientCommand(0, ""); //Not sure why, but this has to be here. Otherwise the specified commands simply refuse to work...
        SetCommandFlags("tf_mvm_force_victory", flags | FCVAR_CHEAT);
        CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {white}VICTORY HAS BEEN FORCED! THE SERVER WILL RESTART IN 10 SECONDS.");
        CreateTimer(10.0, TimedOperator, 100);
        return;
      }
      JumpToWave(next_wave);
    }
  //Code Entry from FC Keypad
    case 100: {
      if (core.CodeEntry == 17) {
        Photon.Fire("FB.BOOM", "StartShake");
        CustomSoundEmitterEX(SFXArray[3].realPath, _, _, 65, false, 0, 1.0, 100);
        Photon.Fire("FB.CodeCorrectKill", "Enable");
        Photon.Fire("FB.KP*", "Lock");
        Photon.Fire("FB.CodeCorrectKill", "Disable", "", 1.0);
      } else {
        core.CodeEntry = 0;
        Photon.Fire("FB.CodeFailedKill", "Enable");
        Photon.Fire("FB.CodeFailedKill", "Disable", "", 1.0);
        CustomSoundEmitterEX("fartsy/memes/priceisright_fail.wav", _, _, 65, false, 0, 1.0, 100);
      }
    }
    case 101: {
      core.CodeEntry++;
    }
    case 102: {
      core.CodeEntry += 2;
    }
    case 103: {
      core.CodeEntry += 3;
    }
    case 104: {
      core.CodeEntry += 4;
    }
    case 105: {
      core.CodeEntry += 5;
    }
    case 106: {
      core.CodeEntry += 6;
    }
    case 107: {
      core.CodeEntry += 7;
    }
    case 108: {
      core.CodeEntry += 8;
    }
    case 109: {
      core.CodeEntry += 9;
    }
  //Lighting Repairs
    case 110: {
      MapLighting[0].Repair();
    }
    case 111: {
      MapLighting[1].Repair();
    }
    case 112: {
      MapLighting[2].Repair();
    }
    case 113: {
      MapLighting[3].Repair();
    }
    case 114: {
      MapLighting[4].Repair();
    }
    case 115: {
      MapLighting[5].Repair();
    }
    case 116: {
      MapLighting[6].Repair();
    }
    case 117: {
      MapLighting[7].Repair();
    }
    case 118: {
      MapLighting[8].Repair();
    }
    case 119: {
      MapLighting[9].Repair();
    }
    case 120: {
      MapLighting[10].Repair();
    }
    case 121: {
      MapLighting[11].Repair();
    }
  //Spoopy
    case 130: {
      Photon.Fire("gTrain", "StartForward");
      Photon.Fire("gTelefect", "Start");
      Photon.Fire("gTelefect", "Stop", "", 3.0);
      Photon.Fire("gTrainSndHaunt", "PlaySound", "", GetRandomFloat(3.0, 7.0));
    }
  //Despawn ghosty boi in like 3 seconds
    case 131: { GhostHandler.Kill(); }
  //Determine what boss just deployed.
    case 132: {
    //if(MetalFaceDidIt){
    //Force phase 4 metal face here
    //}
      switch(BossHandler.bossID) {
      // brute justice
        case 0:{
          Photon.Fire("bots_win", "roundwin");
        }
      }
    }
  //Metal Face reached spawn point
    case 133: {
      CustomSoundEmitterEX(SFXArray[3].realPath, _, _, 45, false, 0, 1.0, 100);
      Photon.Fire("FB.MetalFace.Train", "Stop");
      Photon.Fire("FB.MetalFace.Train", "SetSpeedReal", "50", 3.0);
      Photon.Fire("FB.MetalFace.SpawnShake", "StartShake");
      Photon.Fire("FB.MetalFace.SpawnParticle", "Stop");
    }
  //Tank deployed its bomb
    case 134: {
      BossHandler.TriggerVictory();
      Photon.Fire("boss_deploy_relay", "Trigger", "", 5.0);
    }
  //Spawn metal face
    case 135: {
      BossHandler.OnSpawned(2);
    }
  //Metal Face dead
    case 136: {
      if(BossHandler.iBossPhase != 4){
        BossHandler.iBossPhase = 4;
      }
    }
  //Sephiroth dead (future code)
    case 138: {
    }
  //Wipe mechanic down (MetalFace)
    case 139: {
      BossHandler.shouldTick = false;
      Photon.Fire("FB.MetalFace", "Kill");
      Photon.Fire("FB.MetalFace.Train", "Kill");
      Photon.Fire("FB.MetalFace.SpawnParticle", "Kill");
      Photon.Fire("FB.MetalFace.SummonRockets", "Kill");
      Photon.Fire("FB.MetalFace.GigaBusterSNDTimer", "Kill");
      Photon.Fire("FB.MetalFace.GigaBuster.Explosion", "Kill");
      Photon.Fire("FB.MetalFace.GigaBusterMDL", "Kill");
      Photon.Fire("FB.MetalFace.GigaBusterSND", "Kill");
      BossHandler.OnKilled();
    }
  //Debug, somethingsomething aoe blah blah blah
    case 200: {
      CreateTimer(0.1, TimedOperator, 201);
    }
    case 203: {
      BossHandler.bDoTankBuster = true;
    }
    case 205: {
      PrintToChatAll("%N has the max emnity.", GetEmnityMax());
    }
  //Taco Bell Edition
    case 210: {
      core.tacobell = true;
      ServerCommand("fb_startmoney 200000");
      CPrintToChatAll("{darkviolet}[{orange}INFO{darkviolet}] {white}You have chosen {red}DOVAH'S ASS - TACO BELL EDITION{white}. Warning: All upgrade progress will be lost if you fail the wave. DO NOT FAIL US.");
    }
  //Reset Map
    case 300: {
      AudioManager.Reset(true);
      BossHandler.shouldTick = false;
      core.Reset();
      for (int i = 0; i < sizeof(EmnityManager); i++) EmnityManager[i].Clear();
      for (int i = 0; i < sizeof(MapLighting); i++) MapLighting[i].Repair();
      WeatherManager.Reset();
      BombRegistry[0].isReady = false;
      BombRegistry[0].stateMax = 7;
      g_BombIndex = 5;
      iDmgHealingTotal = 0;
      CreateTimer(1.0, PerformAdverts);
      CreateTimer(40.0, SelectAdminTimer);
      Photon.Fire("Barricade_Rebuild_Relay", "Trigger");
      Photon.Fire("FB.KP*", "Lock");
      Photon.Fire("OldSpawn", "Disable");
      Photon.Fire("NewSpawn", "Enable");
      Photon.Fire("CommonSpells", "Disable");
      Photon.Fire("RareSpells", "Disable");
      Photon.Fire("dovahsassprefer", "Disable");
      Photon.Fire("bombpath_left_arrows", "Disable");
      Photon.Fire("bombpath_right_arrows", "Disable");
      sudo(1000);
      sudo(1002);
      sudo(1007);
    }
  //TEMP FUNCTIONS
    case 301: {
      EmitSoundToAll(BGMArray[5].realPath, _, g_chanBGM, BGMArray[5].SNDLVL, SND_CHANGEVOL, 0.05, _, _, _, _, _, _);
    }
  //TEMP FUNCTIONS
    case 302: {
      EmitSoundToAll(BGMArray[5].realPath, _, g_chanBGM, BGMArray[5].SNDLVL, SND_CHANGEVOL, 1.0, _, _, _, _, _, _);
    }
    case 304: {
      EmitSoundToAll(BGMArray[6].realPath, _, g_chanBGM, BGMArray[6].SNDLVL, SND_CHANGEVOL, 0.05, _, _, _, _, _, _);
    }
    case 305: {
      EmitSoundToAll(BGMArray[6].realPath, _, g_chanBGM, BGMArray[6].SNDLVL, SND_CHANGEVOL, 1.0, _, _, _, _, _, _);
    }
    case 425: {
      for (int i = 0; i < 32; i++) {
        if (IsValidClient(i)) BossHandler.bustNukeTarget = i;
      }
      BossHandler.tickBusterNuclear = true;
    }
    case 426: {
    // Photon.Fire("NukeTankBusterThing", "ForceSpawn");
      Photon.Fire("PL.CannonPitch", "SetPosition", "1.0");
      Photon.Fire("PL.SentCart", "ForceSpawn", "", 5.0);
      Photon.Fire("PL.SentPhys", "Wake", "", 5.1);
      Photon.Fire("PL.CannonFodder", "Enable", "", 5.1);
      Photon.Fire("PL.CannonSND", "PlaySound", "", 5.1);
      Photon.Fire("PL.CannonFodder", "Disable", "", 5.3);
      Photon.Fire("PL.SentCartPhys", "Break", "", 6.0);
    }
    case 427: {
      Photon.Fire("PL.NukeCart", "ForceSpawn");
      Photon.Fire("PL.NukeCartPhys", "Break", "", 1.25);
      Photon.Fire("PL.NukeCartParticle", "Start", "", 1.25);
      Photon.Fire("PL.NukeCartShake", "StartShake", "", 1.25);
      Photon.Fire("PL.NukeCartExplo", "Explode", "", 1.25);
    }
  //LOOP SYSTEM
    case 500: {
      AudioManager.setBGM_Global(9, false);
    }
    case 501: {
      AudioManager.setBGM_Global(10, false);
    }
    case 502: {
      PrintToChatAll("Got 502 but not implemented in popfile, please report this to fartsy!");
      PrintToConsoleAll("[CORE] Phase Change started... phase 4!");
      AudioManager.setBGM_Global(11, false);
    }
  //Increment Wave Null
    case 700: {

    }
  // FINAL Music system rewrite (again) AGAINNNNNNNNNNNN.... and again!... and again :p and again!
    case 1000: {
      AudioManager.UpdateBGM();
    }
  //Stop current song
    case 1001: {
      for (int i = 0; i <= MaxClients; i++) {
        if (StrEqual(BGMArray[AudioState[i].indexBGM].realPath, "null")) continue;
        StopSound(i, g_chanBGM, BGMArray[AudioState[i].indexBGM].realPath);
      }
      return;
    }
  //Feature an admin
    case 1002: {
      char index[2];
      int i = GetRandomInt(1, 10);
      if (i == core.lastAdmin) i = GetRandomInt(1, 10);
      IntToString(i, index, sizeof(index));
      Photon.Fire("AdminPicker", "SetTextureIndex", index);
      core.lastAdmin = i;
      return;
    }
  //Flash Lightning
    case 1003: {
      for (int i = 0; i < sizeof(lightningFlash); i++) Photon.Fire(lightningFlash[i].fireStr);
    }
  //Activate Tornado Timer
    case 1004: {
      if (!WaveSystem().IsActive() || !WeatherManager.canTornado() || WeatherManager.hasTornado()) return;
      WeatherManager.setTornadoTimerActive(true);
      WeatherManager.StartTornadoTimer(WaveSystem().GetWave() == 4 ? GetRandomFloat(30.0, 90.0) : GetRandomFloat(90.0, 210.0));
    }
  //Despawn the tornado
    case 1005: {
      if (GetRandomInt(0, 9) != 9){
        CPrintToChatAll("{red}TTV/professorfartsalot{white}: Nope :>");
        return;
      }
      WeatherManager.Dissipate();
      return;
    }
  //Crusader
    case 1006: {
      if (Photon.CanCreate(84)) {
        AudioManager.SetEnabled(false);
        sudo(1001);
        crusader = true;
        LightLogger(LOGLVL_INFO, "Starting Crusader!");
        Photon.Fire("TheCrusaderTemplate", "ForceSpawn");
        Photon.SetEntityLife("CrusaderTrack*", 80.0);
        Photon.SetEntityLife("CrusaderTrain", 80.0);
        Photon.SetEntityLife("FB.CrusaderNuke", 80.0);
        Photon.SetEntityLife("FB.CRUSADER", 80.0);
        Photon.SetEntityLife("Target.FBLaserCore", 80.0);
        Photon.SetEntityLife("Target.FBLaserInner*", 80.0);
        Photon.SetEntityLife("Target.FBLaserOuterMost*", 80.0);
        Photon.SetEntityLife("CrusaderLaserBase", 80.0);
        Photon.SetEntityLife("CrusaderLaserBaseAlt", 80.0);
        Photon.SetEntityLife("CrusaderLaserBaseAlt1", 80.0);
        Photon.SetEntityLife("CrusaderLaserBaseAlt2", 80.0);
        Photon.SetEntityLife("CrusaderLaserBaseAlt3", 80.0);
        Photon.SetEntityLife("FB.LaserInnerMost", 80.0);
        Photon.SetEntityLife("FB.LaserOuterMost", 80.0);
        Photon.SetEntityLife("FB.LaserCore", 80.0);
        CreateTimer(1.75, CRUSADERINCOMING);
        CreateTimer(25.20, TimedOperator, 9);
        CreateTimer(25.20, TimedOperator, 78);
        CreateTimer(42.60, TimedOperator, 10);
        CreateTimer(63.20, TimedOperator, 80);
        EmitSoundToAll("fartsy/misc/fartsyscrusader_bgm_locus.mp3");
        CreateTimer(80.0, TimedOperator, 79);
        for (int i = 0; i < sizeof(crusaderAtk); i++) Photon.Fire(crusaderAtk[i].fireStr);
        SetupAttackZone(2000000.0, DMG_ENERGYBEAM, g_FrontRottenburg, false, GetRandomInt(25, 35), 65.10, 70.0);
      }
    }
  //Choose bomb path
    case 1007: {
      Photon.Fire("Nest_*", "Disable");
      Photon.Fire("bombpath_right_prefer_flankers", "Disable");
      Photon.Fire("bombpath_left_prefer_flankers", "Disable");
      Photon.Fire("bombpath_left_navavoid", "Disable");
      Photon.Fire("bombpath_right_navavoid", "Disable");
      Photon.Fire("bombpath_right_arrows", "Disable");
      Photon.Fire("bombpath_left_arrows", "Disable");
      switch (GetRandomInt(1, 3)) {
        case 1: {
          Photon.Fire("Nest_R*", "Enable", "", 0.25);
          Photon.Fire("bombpath_right_prefer_flankers", "Enable", "", 0.25);
          Photon.Fire("bombpath_right_navavoid", "Enable", "", 0.25);
          Photon.Fire("bombpath_right_arrows", "Enable", "", 0.25);
        }
        case 2: {
          Photon.Fire("Nest_L*", "Enable", "", 0.25);
          Photon.Fire("bombpath_left_prefer_flankers", "Enable", "", 0.25);
          Photon.Fire("bombpath_left_navavoid", "Enable", "", 0.25);
          Photon.Fire("bombpath_left_arrows", "Enable", "", 0.25);
        }
        case 3: {
          Photon.Fire("dovahsassprefer", "Enable", "",  0.25);
          Photon.Fire("Nest_EN*", "Enable","",  0.25);
          Photon.Fire("bombpath_right_prefer_flankers", "Enable", "", 0.25);
          Photon.Fire("bombpath_right_navavoid", "Enable", "", 0.25);
          Photon.Fire("bombpath_right_arrows", "Enable", "", 0.25);
        }
      }
    }
  //Monitor power up/down!
    case 1008: {
      core.monitorOn = (!core.monitorOn ? true : false);
      Photon.Fire("FB.MonitorSprite", "Color", !core.monitorColor && core.monitorOn ? "0 0 255" : core.monitorOn ? "0 255 0" : "255 0 0", 0.0);
      Photon.Fire(!core.monitorColor && core.monitorOn ? "FB.MonitorBW" : core.monitorOn ? "FB.Monitor" : "FB.Monitor*", core.monitorOn ? "Enable" : "Disable");
      Photon.Fire(!core.monitorColor && core.monitorOn ? "FB.Monitor" : core.monitorOn ? "FB.MonitorBW" : "FB.MonitorBlank", core.monitorOn ? "Disable" : "Enable");
      Photon.Fire("FB.MonitorBlank", core.monitorOn ? "Disable" : "Enable", "", 0.2);
    }
  //Cycle monitor forward
    case 1009: {
      core.camSel++;
      if (core.camSel > 6) core.camSel = 0;
      Photon.Fire("FB.Monitor", "SetCamera", SelectedCamera[core.camSel]);
      Photon.Fire("FB.MonitorBW", "SetCamera", SelectedCamera[core.camSel]);
    }
  //Cycle monitor back
    case 1010: {
      core.camSel--;
      if (core.camSel < 0) core.camSel = 5;
      Photon.Fire("FB.Monitor", "SetCamera", SelectedCamera[core.camSel]);
      Photon.Fire("FB.MonitorBW", "SetCamera", SelectedCamera[core.camSel]);
    }
  //Toggle black and white mode.
    case 1011: {
      if (!core.monitorOn) return;
      core.monitorColor = (!core.monitorColor ? true : false);
      Photon.Fire("FB.MonitorSprite", "Color", core.monitorColor ? "0 255 0" : "0 0 255");
      Photon.Fire(core.monitorColor ? "FB.MonitorBW" : "FB.Monitor", "Disable");
      Photon.Fire(core.monitorColor ? "FB.Monitor" : "FB.MonitorBW", "Enable");
    }
    case 6942: {
      ass_sacPoints = 2147483647;
    }
  //Do not EVER EVER execute this unless it's an emergency...
    case 6969: {
      CPrintToChatAll(!WaveSystem().IsActive() ? "{darkred} [CORE] ERROR, attempted to invoke function without an active wave." : brawler_emergency ? "{darkred}[CORE] Failed to enter emergency mode, it is already active." : WaveSystem().IsWaveNull() ? "{darkred} [CORE] Failed to enter emergency mode, it has been disabled by an unknown entity." : "{darkred}[CORE] EMERGENCY MODE ACTIVATED... (v%s)", BEM_VER);
      if (!WaveSystem().IsActive() || brawler_emergency || WaveSystem().IsWaveNull()) return;
      Emerge(0);
    }
  //DEBUG
    case 7500:{
      Photon.Fire("LargeExplosion", "Explode", "", 0.5);
    }
    case 8000: {
      SephBoss[GetRandomInt(0, 6)].attack();
    }
    case 9000: {
      CreateTimer(10.0, BossHPTimer);
    }
  /*case 9010: {
    CustomSoundEmitterEX(TBGM6, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM4, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM5, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM3, 65, true, 1, 0.05, 100);
  }
  case 9011: {
    CustomSoundEmitterEX(TBGM6, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM4, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM5, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM3, 65, true, 1, 0.05, 100);
  }
  case 9012: {
    CustomSoundEmitterEX(TBGM6, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM4, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM5, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM3, 65, true, 1, 0.05, 100);
  }
  case 9013: {
    CustomSoundEmitterEX(TBGM6, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM4, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM5, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM3, 65, true, 1, 1.0, 100);
  }
  case 9014: {
    CustomSoundEmitterEX(TBGM6, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM4, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM5, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM3, 65, true, 1, 1.0, 100);
  }
  case 9015: {
    CustomSoundEmitterEX(TBGM6, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM4, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM5, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM3, 65, true, 1, 1.0, 100);
  }
  case 9016: {
    CustomSoundEmitterEX(TBGM6, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM4, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM5, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM3, 65, true, 1, 1.0, 100);
  }
  //Play Instrumental
  case 9020: {
    CustomSoundEmitterEX(TBGM0, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM1, 65, true, 1, 0.05, 100);
  }
  //Play Both
  case 9021: {
    CustomSoundEmitterEX(TBGM0, 65, true, 1, 1.0, 100);
    CustomSoundEmitterEX(TBGM1, 65, true, 1, 1.0, 100);
  }
  //Play vocal only
  case 9022: {
    CustomSoundEmitterEX(TBGM0, 65, true, 1, 0.05, 100);
    CustomSoundEmitterEX(TBGM1, 65, true, 1, 1.0, 100);
  }*/
  //Test
    case 10000:{
      int ent = FindEntityByClassname(-1, "tf_objective_resource");
      if (ent != -1){
        PrintToChatAll("%i / 3", GetEntData(ent, FindSendPropInfo("CTFObjectiveResource", "m_nFlagCarrierUpgradeLevel")));
      }
    }
    case 20000:{
      BossHandler.EmitSpawnSound(2);
    }
    case 21000:{
      int i = FindTarget(0, "Professor", true, false);
      AudioManager.setEventMode(i, 44, true);
    }
    case 22000:{
      int i = FindTarget(0, "Professor", true, false);
      AudioManager.setEventMode(i, 44, false);
    }
    case 42690:{
      BossHandler.bossID = 69;
      BossHandler.shouldTick = true;
      BossHandler.isAlive = true;
      EmitSoundToAll("fartsy/misc/brawler/fartsy_dll/intro.mp3");
      PrintToConsoleAll("Loading FARTSY.DLL from C:\\Windows\\System32 ...");
      PrintToServer("Loading FARTSY.IMG from /boot/efi ...");
      PrintToChatAll("FARTSY.DLL has joined the game");
      ServerCommand("sm_mortal @all");
      AddCommandListener(CommandOverride, "sm_buddha");
      AddCommandListener(CommandOverride, "sm_gimme");
      AddCommandListener(CommandOverride, "sm_givew");
      AddCommandListener(CommandOverride, "sm_gi");
      AddCommandListener(CommandOverride, "sm_god");
      AddCommandListener(CommandOverride, "sm_admin");
      AddCommandListener(CommandOverride, "sm_noclip");
      AddCommandListener(CommandOverride, "sm_noclipme");
      CreateTimer(6.1, TimedOperator, 50000);
    }
    case 42696:{
      RemoveCommandListener(CommandOverride, "sm_buddha");
      RemoveCommandListener(CommandOverride, "sm_god");
      RemoveCommandListener(CommandOverride, "sm_gimme");
      RemoveCommandListener(CommandOverride, "sm_givew");
      RemoveCommandListener(CommandOverride, "sm_gi");
      RemoveCommandListener(CommandOverride, "sm_admin");
      RemoveCommandListener(CommandOverride, "sm_noclip");
      RemoveCommandListener(CommandOverride, "sm_noclipme");
    }
    default: {
      char LogData[256];
      Format(LogData, sizeof(LogData), "[Sudo] No valid opcode %i found.", task);
      LightLogger(LOGLVL_ERROR, LogData);
    }
  }
  return;
}

public Action CommandOverride(int client, const char[] command, int argc) {
  ServerCommand(GetRandomInt(0, 1 == 1) ? "sm_slay #%i" : "sm_smite #i", GetClientUserId(client));
  PrintToChatAll("[FARTSY.DLL] Did you REALLY think I would let you cheat, %N? How about instead of using %s you actually play the game, loser!", client, command);
  return Plugin_Handled;
}

//Timed commands
public Action TimedOperator(Handle timer, int job) {
  switch (job) {
    case 0: {
      for (int i = 1; i < MaxClients; ++i) if(IsValidClient(i)) CPrintToChat(i, AudioState[i].VIPBGM >= 0 && AudioState[i].VIPIndex > 0 ? "{darkviolet}[{forestgreen}CORE{darkviolet}] {white}Wave %i: {forestgreen}%s{white} (requested by VIP {forestgreen}%N{white})" : "{darkviolet}[{forestgreen}CORE{darkviolet}] {white}Wave %i: {forestgreen}%s", WaveSystem().GetWave(), AudioState[i].songName, AudioState[i].VIPIndex);
    }
    case 4444: { sudo(4445); mIndex = -1; }
    //Boss script
    case 2: {
      sudo(1001); // Stop all bgm.
      CreateTimer(0.0, TimedOperator, 3);
    }
    //Boss script pt 2
    case 3: {
      AudioManager.setBGM_Global(16, true);
      Photon.Fire("FB.FadeTotalBLCK", "Fade");
      Photon.Fire("FB.FadeTotalBLCK", "Fade", "", 3.0);
      Photon.Fire("FB.FadeTotalBLCK", "Fade", "", 7.0);
      Photon.Fire("FB.FadeTotalBLCK", "Fade", "", 12.0);
      Photon.Fire("SephMeteor", "ForceSpawn", "", 19.6);
      CreateTimer(23.0, TimedOperator, 4);
    }
    //Boss script pt 3
    case 4: {
      CustomSoundEmitterEX(SFXArray[9].realPath, _, _, 65, false, 0, 1.0, 100);
      CreateTimer(4.1, TimedOperator, 5);
    }
    //Boss script pt 4
    case 5: {
      CustomSoundEmitterEX(SFXArray[58].realPath, _, _, 65, false, 0, 1.0, 100);
      Photon.Fire("SephNuke", "ForceSpawn", "", 3.0);
      CreateTimer(3.2, TimedOperator, 6);
    }
    //Boss script pt 5
    case 6: {
      CustomSoundEmitterEX(SFXArray[35].realPath, _, _, 65, false, 0, 1.0, 100);
      Photon.Fire("FB.Fade", "Fade");
      CreateTimer(1.0, SephATK);
      CreateTimer(1.7, TimedOperator, 7);
    }
    //DO THE THING ALREADY
    case 7: {
      core.sephiroth = true;
      //AudioState.ticksBGM = 0;
      //AudioState.loopSeconds = BGMArray[16].loopSeconds;
    }
    //Signal boss to actually spawn after delay.
    case 8: {
      CustomSoundEmitterEX(SFXArray[57].realPath, _, _, 65, false, 0, 1.0, 100);
      CPrintToChatAll("{darkgreen}[CORE] You did it!!! {darkred}Or... did you...?");
      Photon.Fire("FB.FadeBLCK", "Fade");
      CreateTimer(4.8, TimedOperator, 2);
    }
    //Crusader Nuke activation
    case 9: {
      core.canCrusaderNuke = true;
      CreateTimer(1.0, NukeTimer, 1);
    }
    //Crusader Nuke Deactivation
    case 10: {
      core.canCrusaderNuke = false;
      return Plugin_Stop;
    }
    //Seph Nuke Deactivation
    case 11: {
      SephBoss[0].shouldNuke = false;
      return Plugin_Stop;
    }
    //SENTMeteor Timeout
    case 12: {
      core.canSENTMeteors = false;
      return Plugin_Stop;
    }
    //SENTStars Timeout
    case 14: {
      core.canSENTStars = false;
      return Plugin_Stop;
    }
    //Incoming
    case 21: {
      CustomSoundEmitterEX(SFXArray[37].realPath, _, _, 65, false, 0, 1.0, 100);
      return Plugin_Stop;
    }
    case 37: {
      EmitSoundToAll(SFXArray[35].realPath);
      return Plugin_Stop;
    }
    //Severe storm coming!!!
    case 40: {
      WeatherManager.IssueTornadoWarning();
      return Plugin_Stop;
    }
    // start tornado
    case 41: {
      WeatherManager.TryFormTornado();
      return Plugin_Stop;
    }
    case 42: {
      WeatherManager.Dissipate();
      sudo(1004);
    }
    case 60: {
      CustomSoundEmitterEX(SFXArray[40].realPath, _, _, SFXArray[40].SNDLVL, false, 0, 1.0, 100);
      return Plugin_Stop;
    }
    case 70: {
      if (WaveSystem().IsActive()) {
        Photon.Fire("FB.KP*", "Unlock");
        CustomSoundEmitterEX(SFXArray[0].realPath, _, _, SFXArray[0].SNDLVL, false, 0, 1.0, 100);
      }
      return Plugin_Stop;
    }
    case 71: {
      CustomSoundEmitterEX(SFXArray[62].realPath, _, _, SFXArray[62].SNDLVL, false, 0, 1.0, 100);
      return Plugin_Stop;
    }
    case 78: {
      EmitSoundToAll(SFXArray[7].realPath);
      return Plugin_Handled;
    }
    case 79: {
      crusader = false;
      if (WaveSystem().IsActive() && (WaveSystem().GetWave() == 3 || WaveSystem().GetWave() == 5)) { sudo(99); }
      return Plugin_Stop;
    }
    case 80: {
      EmitSoundToAll(SFXArray[59].realPath);
      return Plugin_Stop;
    }
    case 99: {
      if (WaveSystem().IsActive()) sudo(99);
      return Plugin_Stop;
    }
    case 100: {
      ServerCommand("_restart");
    }
    case 200:{
      for (int i = 0; i < sizeof(CircleAOEenabled); i++){
        CircleAOEenabled[i] = true;
      }
    }
    case 201:{
      int attemptSpawn = 0;
      int i = GetRandomInt(0, sizeof(CircleAOEpos)-1);
      if (!CircleAOEenabled[i]) {
        if (attemptSpawn >= sizeof(CircleAOEpos)) return Plugin_Stop;
        attemptSpawn++;
        sudo(200);
        return Plugin_Stop;
      }
      attemptSpawn = 0;
      DispatchCircleAOE(CircleAOEpos[i]);
      PrintToChatAll("AOE %i CircleAOEen[i] = %i attempt %i sizeof %i", i, CircleAOEenabled[i], attemptSpawn, sizeof(CircleAOEpos));
      CircleAOEenabled[i] = false;
      CreateTimer(3.0, TimedOperator, 200);
      return Plugin_Stop;
    }
    case 202:{
      CustomSoundEmitterEX(SFXArray[60].realPath, _, _, SFXArray[60].SNDLVL, false, 0, 1.0, 100);
      return Plugin_Stop;
    }
    case 42690: {
      BossHandler.sOverflow = false;
      return Plugin_Stop;
    }
    case 50000: {
      ServerCommand("sv_cheats 1");
      for (int client = 1; client <= MaxClients; client++) if (IsClientInGame(client)) ClientCommand(client, "r_screenoverlay fartsy/fartsy_dll_core");
      ServerCommand("sv_cheats 0");
      Photon.Fire("FB.FadeBLCK", "Alpha", "127", 0.0);
      Photon.Fire("FB.FadeBLCK", "Fade");
      SendHudTextAll(1, "Did you think this would be easy...?", 0.1, 0.15, 9.0, {0, 160, 0, 255}, {0, 255, 0, 255}, 2, 0.5, 0.05, 0.75);
      EmitSoundToAll("fartsy/misc/brawler/fartsy_dll/terror.mp3");
      CreateTimer(3.0, TimedOperator, 50001);
      return Plugin_Stop;
    }
    case 50001: {
      Photon.Fire("FB.FadeBLCK", "Alpha", "127", 0.0);
      Photon.Fire("FB.FadeBLCK", "Fade");
      SendHudTextAll(2, "You can't possibly expect the same old tricks to work.... can you?~", 0.2, 0.2, 3.0, {0, 160, 0, 192}, {0, 255, 0, 192}, 2, 0.75, 0.05, 0.75);
      CreateTimer(5.0, TimedOperator, 50002);
      return Plugin_Stop;
    }
    case 50002: {
      Photon.Fire("FB.FadeBLCK", "Alpha", "127", 0.0);
      Photon.Fire("FB.FadeBLCK", "Fade");
      SendHudTextAll(1, "LAUGH IT UP. You and I both know this is 'just another boss fight'...", 0.15, 0.25, 7.0, {0, 160, 0, 192}, {0, 255, 0, 192}, 2, 0.5, 0.05, 0.75);
      CreateTimer(5.0, TimedOperator, 50003);
      return Plugin_Stop;
    }
    case 50003: {
      Photon.Fire("FB.FadeBLCK", "Alpha", "192", 0.0);
      Photon.Fire("FB.FadeBLCK", "Fade");
      SendHudTextAll(2, "...or is it?~", 0.2, 0.3, 2.0, {0, 160, 0, 192}, {0, 255, 0, 192}, 2, 0.5, 0.05, 0.75);
      CreateTimer(4.0, TimedOperator, 50004);
      return Plugin_Stop;
    }
    case 50004:{
      Photon.Fire("FB.FadeBLCK", "Alpha", "224", 0.0);
      Photon.Fire("FB.FadeBLCK", "Fade");
      SendHudTextAll(1, "ARE YOU HAPPY? All of you who knew what was happening...~", 0.25, 0.35, 7.0, {0, 160, 0, 192}, {0, 255, 0, 192}, 2, 0.5, 0.05, 0.75);
      CreateTimer(5.0, TimedOperator, 50005);
      return Plugin_Stop;
    }
    case 50005:{
      Photon.Fire("FB.FadeBLCK", "Alpha", "224", 0.0);
      Photon.Fire("FB.FadeBLCK", "Fade");
      SendHudTextAll(2, "Every last one of you...", 0.35, 0.4, 5.0, {0, 160, 0, 192}, {0, 255, 0, 192}, 1, 0.5, 0.5, 0.75);
      CreateTimer(5.0, TimedOperator, 50006);
      return Plugin_Stop;
    }
    case 50006:{
      Photon.Fire("FB.FadeBLCK", "Alpha", "192", 0.0);
      Photon.Fire("FB.FadeBLCK", "Fade");
      SendHudTextAll(1, "I hope you're as happy as I am...~", 0.15, 0.45, 7.0, {0, 160, 0, 192}, {0, 255, 0, 192}, 2, 0.5, 0.05, 0.75);
      CreateTimer(4.0, TimedOperator, 50007);
      return Plugin_Stop;
    }
    case 50007:{
      Photon.Fire("FB.FadeBLCK", "Fade");
      SendHudTextAll(2, "...knowing you all just spent all your time here~", 0.25, 0.5, 5.0, {0, 160, 0, 192}, {0, 255, 0, 192}, 2, 0.5, 0.05, 0.75);
      CreateTimer(5.0, TimedOperator, 50008);
      return Plugin_Stop;
    }
    case 50008:{
      Photon.Fire("FB.FadeBLCK", "Alpha", "255", 0.0);
      Photon.Fire("FB.FadeBLCK", "Fade");
      ServerCommand("sv_cheats 1");
      for (int client = 1; client <= MaxClients; client++) if (IsClientInGame(client)) ClientCommand(client, "r_screenoverlay fartsy/fartsy_dll_anon");
      ServerCommand("sv_cheats 0");
      Photon.Fire("FB.FadeBLCK", "Fade", "", 3.0);
      SendHudTextAll(1, "JUST TO DIE!!!", 0.4, 0.6, 7.0, {255, 0, 0, 255}, {255, 0, 0, 255}, 1, 0.5, 0.05, 0.75);
      CreateTimer(5.75, TimedOperator, 50009);
      return Plugin_Stop;
    }
    case 50009:{
      ServerCommand("sv_cheats 1");
      for (int client = 1; client <= MaxClients; client++) if (IsClientInGame(client)) ClientCommand(client, "r_screenoverlay \\");
      ServerCommand("sv_cheats 0");
      EmitSoundToAll("fartsy/music/tcmusic/humanhypothesis.wav");
      return Plugin_Stop;
    }
  }
  return Plugin_Stop;
}