char WaveSystem_Version[8] = "1.0.0";
enum struct WAVESYSTEM {
  int mode;
  int wave;
  void init() {
    this.update();
    GlobalAudio.Reset();
    core.tacobell = false;
    ServerCommand("fb_startmoney 50000");
    CPrintToChatAll("{darkviolet}[{yellow}INFO{darkviolet}] {red}%s {white} (Core v%s, System v%s). Prepare yourself for the unpredictable... [{limegreen}by TTV/ProfessorFartsalot{white}]", "mvm_rottenburg_fartsys_ass-r22", GET_PLUGIN_VERSION(), WaveSystem_Version);
    FastFire2("rain", "Alpha", "0", 0.0, false);
    PrintToChatAll("%i", WaveSystem().mode);
    if (this.mode == 2) {
      PrintToChatAll("UH OH");
    }
  }

  void run() {
    this.update();
    UpdateAllHealers();
    core.isWave = true;
    bombState[0].state = DefaultsArray[this.wave].defBombStatus;
    bombState[0].stateMax = DefaultsArray[this.wave].defBombStatusMax;
    core.canHWBoss = DefaultsArray[this.wave].defCanHWBoss;
    WeatherManager.canTornado = DefaultsArray[this.wave].defCanTornado;
    core.sacPointsMax = DefaultsArray[this.wave].defSacPointsMax;
    core.FailedCount = 0; // Reset fail count to zero. (See EventWaveFailed - where we play the BGM.)
    WeatherManager.Activate();
    CreateTimer(0.25, TimedOperator, 0); // Print wave information to global chat
    CreateTimer(2.5, PerformWaveAdverts); // Activate the mini hud
    CreateTimer(0.1, BombStatusUpdater); // Activate the bomb status updater
    CreateTimer(1.0, BombStatusAddTimer); // Activate bomb status timer
    CreateTimer(1.0, RobotLaunchTimer); // Activate robot launch timer
    CreateTimer(1.0, SacrificePointsTimer); // Activate sacrifice points add timer
    CreateTimer(1.0, SacrificePointsUpdater); // Activate sacrifice points updater
    for (int i = 0; i < sizeof(WaveSetup) - 4; i++) FastFire2(WaveSetup[i], "", "", 0.0, true);
    sudo(1002);
    sudo(1007);
    switch (this.mode) {
      case 0, 1: {
        this.run_normal();
      }
      /*case 1: {
        this.run_tacobell();
      }*/
      case 2: {
        this.run_wavenull();
      }
    }
    if (core.canHWBoss) {
      float hwn = GetRandomFloat(core.HWNMin, core.HWNMax);
      CreateTimer(hwn, HWBosses);
    }
  }
  // Normal mode
  void run_normal() {
    GlobalAudio.setBGM(core.tacobell ? tacoBellBGMIndex[this.wave] : DefaultsArray[this.wave].defBGMIndex, true);
    switch (this.wave) {
      case 2: {
        CreateTimer(GetRandomFloat(10.0, 45.0), TimedAOEs);
      }
      case 3, 10, 17: {
        core.HWNMax = 360.0;
        for (int i = 8; i < sizeof(WaveSetup) - 3; i++) FastFire2(WaveSetup[i], "", "", 0.0, true);
        float f = GetRandomFloat(60.0, 180.0);
        CreateTimer(f, TimedOperator, 70);
      }
      case 4, 11, 18: {
        core.HWNMax = 360.0;
        for (int i = 8; i < sizeof(WaveSetup) - 2; i++) FastFire2(WaveSetup[i], "", "", 0.0, true);
      }
      case 5, 12, 19: {
        Get_Boss_Handler().bossID = 0;
        Get_Boss_Handler().shouldTick = true;
        core.HWNMax = 260.0;
        core.HWNMin = 140.0;
        for (int i = 8; i < sizeof(WaveSetup) - 2; i++) FastFire2(WaveSetup[i], "", "", 0.0, true);
        FastFire2("w5_engie_hints", "Trigger", "", 3.0, false);
        FastFire2("FB.OnslaughterBase", "SetHealth", "320000", 0.0, false);
        FastFire2("FB.OnslaughterBase", "SetHealth", "320000", 1.0, false);
        float f = GetRandomFloat(60.0, 180.0);
        CreateTimer(f, TimedOperator, 70);
      }
      case 6, 13, 20: {
        core.HWNMax = 260.0;
        core.HWNMin = 140.0;
        for (int i = 8; i < sizeof(WaveSetup) - 1; i++) FastFire2(WaveSetup[i], "", "", 0.0, true);
      }
      case 7, 14, 21: {
        core.HWNMax = 240.0;
        core.HWNMin = 120.0;
        for (int i = 8; i < sizeof(WaveSetup); i++) FastFire2(WaveSetup[i], "", "", 0.0, true);
        FastFire2("w5_engie_hints", "Trigger", "", 3.0, false);
      }
      case 8, 15: {
        core.HWNMax = 240.0;
        core.HWNMin = 120.0;
        for (int i = 8; i < sizeof(WaveSetup); i++) FastFire2(WaveSetup[i], "", "", 0.0, true);
      }
    }
    if (core.canHWBoss) {
      float hwn = GetRandomFloat(core.HWNMin, core.HWNMax);
      CreateTimer(hwn, HWBosses);
    }
  }
  // Taco Bell Edition
  void run_tacobell() {}
  // Wave Null
  void run_wavenull() {
    switch (this.wave) {
      case 0: {
        CPrintToChatAll("{fartsyred} [WARNING]: YOU HAVE CHOSEN TO LAUNCH WAVE NULL.");
        CPrintToChatAll("{fartsyred} [WARNING]: SHOULD YOU FAIL THIS WAVE, YOU *WILL* BE RESTARTING FROM THE VERY BEGINNING WITH NO UPGRADES.");
        CPrintToChatAll("{fartsyred} [WARNING]: THE FATE OF THIS SERVER IS IN YOUR HANDS. DO. NOT. FAIL.");
        FastFire2("Weather.Sky", "Skin", "3", 0.0, false);
        WeatherManager.fogDensity = 0.1;
        PrintToChatAll("playing sound\\fartsy\\music\\wavenull_intro.mp3, waves should be timed so that the music changes right at the end of this intro.");
        FastFire2("weather.sky", "Skin", "2", 0.0, false);
        sudo(1001);
        EmitSoundToAll("sound/fartsy/music/wavenull_intro.mp3");
        WeatherManager.fogColorRTarget = 95.0;
        WeatherManager.fogColorGTarget = 35.0;
        WeatherManager.fogColorBTarget = 35.0;
      }
    }
    this.wave++;
  }
  // WIPE MECHANIC - If all players  die, the wave fails.
  void run_bodycheck() {
    int alive = 0;
    for (int i = 1; i <= MaxClients; i++) if (IsValidClient(i)) if(IsPlayerAlive(i) && GetClientTeam(i) == 2) alive++;
    if (alive == 0) {
        CPrintToChatAll("{red}You've all died! Are you happy?");
        FastFire2("bots_win", "roundwin", "", 0.0, false);
    }
  }
  //Update internal gamemode identifier, reload configs if needed.
  void update() {
    char logdata[192], popfile[128];
    int ent = FindEntityByClassname(-1, "tf_objective_resource");
    if (ent == -1) return;
    GetEntPropString(ent, Prop_Send, "m_iszMvMPopfileName", popfile, sizeof(popfile));
    int target = (StrContains(popfile, "tacobell") != -1 ? 1 : (StrContains(popfile, "wavenull") != -1) ? 2 : 0);
    if (this.mode != target) {
        Format(logdata, sizeof(logdata), "Current gamemode %i does not match detected gamemode %i! Someone must have changed the gamemode! Setting up core data immediately!!!", this.mode, target);
        AssLogger(LOGLVL_DEBUG, logdata);
        this.mode = target;
        SetupCoreData();
    }
    this.wave = this.mode != 2 ? GetEntData(ent, FindSendPropInfo("CTFObjectiveResource", "m_nMannVsMachineWaveCount")) : 0;
  }
}
WAVESYSTEM WS;

WAVESYSTEM WaveSystem() { return WS; }