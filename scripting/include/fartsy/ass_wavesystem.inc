char WaveSystem_Version[8] = "1.2.0";
const int WaveTypes_NORMAL = 0;
const int WaveTypes_TACOBELL = 1;
const int WaveTypes_WAVENULL = 2;

enum struct WAVESYSTEMDATA {
  bool IsActive;
  int type;
  int wave;
}
WAVESYSTEMDATA WSData;

methodmap WAVESYSTEM {
  public int GetType() {
    return WSData.type;
  }
  public int GetWave() {
    return WSData.wave;
  }
  public void SetWave(int w) {
    WSData.wave = w;
  }
  public bool IsActive() {
    return WSData.IsActive;
  }
  public void SetActive(bool a) {
    WSData.IsActive = a;
  }
  public bool IsDefault() {
    return WSData.type == WaveTypes_NORMAL;
  }
  public bool IsTacoBell() {
    return WSData.type == WaveTypes_TACOBELL;
  }
  public bool IsWaveNull() {
    return WSData.type == WaveTypes_WAVENULL;
  }
  public void init() {
    _serverActive = true;
    this.update();
    AudioManager.Reset(true);
    core.tacobell = false;
    ServerCommand("fb_startmoney 50000");
    CPrintToChatAll("{darkviolet}[{yellow}INFO{darkviolet}] {red}%s {white} (Core v%s, System v%s). Prepare yourself for the unpredictable... [{limegreen}by TTV/ProfessorFartsalot{white}]", "mvm_rottenburg_fartsys_ass-r22", PLUGIN_VERSION, WaveSystem_Version);
    FastFire2("rain", "Alpha", "0", 0.0);
    PrintToChatAll("WaveSystem type %i", this.GetType());
    if (this.IsWaveNull()) {
      PrintToChatAll("UH OH");
    }
  }

  public void run() {
    this.update();
    WSData.IsActive = true;
    g_BombIndex = DefaultsArray[WSData.wave].defBombStatus;
    BombRegistry[0].stateMax = DefaultsArray[WSData.wave].defBombStatusMax;
    core.canHWBoss = DefaultsArray[WSData.wave].defCanHWBoss;
    WeatherManager.canTornado = DefaultsArray[WSData.wave].defCanTornado;
    ass_sacPointsMax = DefaultsArray[WSData.wave].defSacPointsMax;
    core.FailedCount = 0; // Reset fail count to zero. (See EventWaveFailed - where we play the BGM.)
    WeatherManager.Activate();
    CreateTimer(0.25, TimedOperator, 0); // Print wave information to global chat
    CreateTimer(2.5, PerformWaveAdverts); // Activate the mini hud
    CreateTimer(0.1, BombStatusUpdater); // Activate the bomb status updater
    CreateTimer(1.0, BombStatusAddTimer); // Activate bomb status timer
    CreateTimer(1.0, RobotLaunchTimer); // Activate robot launch timer
    CreateTimer(1.0, SacrificePointsTimer); // Activate sacrifice points add timer
    CreateTimer(1.0, SacrificePointsUpdater); // Activate sacrifice points updater
    for (int i = 0; i < sizeof(WaveSetup) - 4; i++) FastFire2(WaveSetup[i]);
    sudo(1002);
    sudo(1007);
    switch (WSData.type) {
      case WaveTypes_NORMAL: {
        this.run_normal();
      }
      /*case WaveTypes_TACOBELL: {
        this.run_tacobell();
      }*/
      case WaveTypes_WAVENULL: {
        this.run_wavenull();
      }
    }
    if (core.canHWBoss) {
      float hwn = GetRandomFloat(core.HWNMin, core.HWNMax);
      CreateTimer(hwn, HWBosses);
    }
  }
  // Normal mode
  public void run_normal() {
    PrintToServer("Wave manager is calling run_normal because this.type is %i", WSData.type);
    if (WSData.wave == 0) WSData.wave = 1;
    AudioManager.setBGM_Global(DefaultsArray[WSData.wave].defBGMIndex, true);
    switch (WSData.wave) {
      case 2: {
        CreateTimer(GetRandomFloat(10.0, 45.0), TimedAOEs);
      }
      case 3, 10, 17: {
        core.HWNMax = 360.0;
        for (int i = 8; i < sizeof(WaveSetup) - 3; i++) FastFire2(WaveSetup[i]);
        float f = GetRandomFloat(60.0, 180.0);
        CreateTimer(f, TimedOperator, 70);
      }
      case 4, 11, 18: {
        core.HWNMax = 360.0;
        for (int i = 8; i < sizeof(WaveSetup) - 2; i++) FastFire2(WaveSetup[i]);
      }
      case 5, 12, 19: {
        Get_Boss_Handler().bossID = 0;
        Get_Boss_Handler().shouldTick = true;
        core.HWNMax = 260.0;
        core.HWNMin = 140.0;
        for (int i = 8; i < sizeof(WaveSetup) - 2; i++) FastFire2(WaveSetup[i]);
        FastFire2("w5_engie_hints", "Trigger", "", 3.0);
        char onsHP[32];
        Format(onsHP, 32, "%i", GetBossHealthModifier(640000));
        FastFire2("FB.OnslaughterBase", "SetHealth", onsHP, 0.0);
        FastFire2("FB.OnslaughterBase", "SetHealth", onsHP, 1.0);
        float f = GetRandomFloat(60.0, 180.0);
        CreateTimer(f, TimedOperator, 70);
      }
      case 6, 13, 20: {
        core.HWNMax = 260.0;
        core.HWNMin = 140.0;
        for (int i = 8; i < sizeof(WaveSetup) - 1; i++) FastFire2(WaveSetup[i]);
      }
      case 7, 14, 21: {
        core.HWNMax = 240.0;
        core.HWNMin = 120.0;
        for (int i = 8; i < sizeof(WaveSetup); i++) FastFire2(WaveSetup[i]);
        FastFire2("w5_engie_hints", "Trigger", "", 3.0);
      }
      case 8, 15: {
        core.HWNMax = 240.0;
        core.HWNMin = 120.0;
        for (int i = 8; i < sizeof(WaveSetup); i++) FastFire2(WaveSetup[i]);
      }
    }
    if (core.canHWBoss) {
      float hwn = GetRandomFloat(core.HWNMin, core.HWNMax);
      CreateTimer(hwn, HWBosses);
    }
  }
  // Taco Bell Edition
  public void run_tacobell() {
    PrintToServer("Wave manager is calling run_tacobell because this.mode is %i", WSData.type);
    //AudioManager.setBGM(tacoBellBGMIndex[WSData.wave], true);
  }
  // Wave Null
  public void run_wavenull() {
    PrintToServer("Wave manager is calling run_wavenull because this.mode is %i", WSData.type);
    switch (WSData.wave) {
      case 0: {
        CPrintToChatAll("{fartsyred} [WARNING]: YOU HAVE CHOSEN TO LAUNCH WAVE NULL.");
        CPrintToChatAll("{fartsyred} [WARNING]: SHOULD YOU FAIL THIS WAVE, YOU *WILL* BE RESTARTING FROM THE VERY BEGINNING WITH NO UPGRADES.");
        CPrintToChatAll("{fartsyred} [WARNING]: THE FATE OF THIS SERVER IS IN YOUR HANDS. DO. NOT. FAIL.");
        FastFire2("Weather.Sky", "Skin", "3");
        WeatherManager.fogDensity = 0.1;
        PrintToChatAll("playing sound\\fartsy\\music\\wavenull_intro.mp3, waves should be timed so that the music changes right at the end of this intro.");
        FastFire2("weather.sky", "Skin", "2");
        sudo(1001);
        EmitSoundToAll("sound/fartsy/music/wavenull_intro.mp3");
        WeatherManager.fogColorRTarget = 95.0;
        WeatherManager.fogColorGTarget = 35.0;
        WeatherManager.fogColorBTarget = 35.0;
      }
    }
    WSData.wave++;
  }
  // WIPE MECHANIC - If all players  die, the wave fails.
  public void run_bodycheck() {
    int alive = 0;
    for (int i = 1; i <= MaxClients; i++) if (IsValidClient(i)) if(IsPlayerAlive(i) && GetClientTeam(i) == 2) alive++;
    if (alive == 0) {
      CPrintToChatAll("{red}You've all died! Are you happy?");
      FastFire2("bots_win", "RoundWin");
    }
  }
  //Update internal gamemode identifier, reload configs if needed.
  public void update() {
    char logdata[192], popfile[128];
    int ent = FindEntityByClassname(-1, "tf_objective_resource");
    if (ent == -1) return;
    GetEntPropString(ent, Prop_Send, "m_iszMvMPopfileName", popfile, sizeof(popfile));
    int target = (StrContains(popfile, "tacobell") != -1 ? 1 : (StrContains(popfile, "wavenull") != -1) ? 2 : 0);
    if (WSData.type != target) {
      Format(logdata, sizeof(logdata), "Current type %i does not match detected type %i! Someone must have changed the gamemode! Setting up core data immediately!!!", this.GetType(), target);
      AssLogger(LOGLVL_DEBUG, logdata);
      WSData.type = target;
      core.init_post();
    }
    WSData.wave = this.IsWaveNull() ? WSData.wave : GetEntData(ent, FindSendPropInfo("CTFObjectiveResource", "m_nMannVsMachineWaveCount"))
  }
}
WAVESYSTEM WS;
WAVESYSTEM WaveSystem() { return WS; }

//Wave failsafe system - To be implemented better by making an enum struct and doing failSafe[x].execute(); in FartsysAss.sp
void PerformWaveFailsafe(int x){
  switch(x) {
    case 0:{
      ServerCommand("sm_freeze @blue; wait 180; sm_smash @blue;sm_evilrocket @blue");
    }
    case 1:{
      ServerCommand("sm_smite @blue");
    }
  }
}

//Get current wave
public int GetCurWave() {
  int ent = FindEntityByClassname(-1, "tf_objective_resource");
  if (ent != -1) return GetEntData(ent, FindSendPropInfo("CTFObjectiveResource", "m_nMannVsMachineWaveCount"));
  AssLogger(LOGLVL_ERROR, "tf_objective_resource not found");
  return -1;
}