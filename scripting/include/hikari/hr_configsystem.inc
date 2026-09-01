 //ASS_CONFIGSYSTEM - Contains the config loaders and all the plugin core data entries.
#include <smjansson> // Needed for JSON parsing
char HRCFG_VERSION[8] = "2.2.1";

//Default values per wave
enum struct DEFAULTS {
  bool defCanHWBoss;
  bool defCanTornado;
  int defBGMIndex;
  int defBombStatus;
  int defBombStatusMax;
  int defSacPointsMax;
}
DEFAULTS DefaultsArray[9];

methodmap DEFAULTSMANAGER {
  // Gets defaults for a specified wave
  public DEFAULTS GetDefaults(int wave) { return DefaultsArray[wave]; }
  // Only called during plugin startup
  public void SetupDefaults(Handle confCoreData) {
    LightLogger(LOGLVL_INFO, "[ConfigSystem @ v%s] Setting up Wave Defaults... ", HRCFG_VERSION);
    char bufCoreData[256];
    LightLogger(LOGLVL_DEBUG, "[ConfigSystem:23] OVERRIDE_AND_GET_COREDATA() returned %s", OVERRIDE_AND_GET_COREDATA());
    confCoreData = OpenFile(OVERRIDE_AND_GET_COREDATA(), "rt", false);
    int defIndexHW, defIndexTornado, defIndexBGM, defIndexBS, defIndexBSM, defIndexSPM; // Start at 0 because we don't have a wave 0
    if (confCoreData == INVALID_HANDLE) return;
    while (ReadFileLine(confCoreData, bufCoreData, sizeof(bufCoreData))) {
      TrimString(bufCoreData);
      if (!StrContains(bufCoreData, "Wave.defCanHWBoss:")) {
        defIndexHW++;
        DefaultsArray[defIndexHW].defCanHWBoss = CharToBool(bufCoreData);
      }
      else if (!StrContains(bufCoreData, "Wave.defCanTornado:")) {
        defIndexTornado++;
        DefaultsArray[defIndexTornado].defCanTornado = CharToBool(bufCoreData);
      }
      else if (!StrContains(bufCoreData, "Wave.defBGMIndex:")) {
        defIndexBGM++;
        ReplaceString(bufCoreData, sizeof(bufCoreData), "Wave.defBGMIndex:", "", true);
        DefaultsArray[defIndexBGM].defBGMIndex = StringToInt(bufCoreData);
      }
      else if (!StrContains(bufCoreData, "Wave.defBombStatus:")) {
        defIndexBS++;
        ReplaceString(bufCoreData, sizeof(bufCoreData), "Wave.defBombStatus:", "", true);
        DefaultsArray[defIndexBS].defBombStatus = StringToInt(bufCoreData);
      }
      else if (!StrContains(bufCoreData, "Wave.defBombStatusMax:")) {
        defIndexBSM++;
        ReplaceString(bufCoreData, sizeof(bufCoreData), "Wave.defBombStatusMax:", "", true);
        DefaultsArray[defIndexBSM].defBombStatusMax = StringToInt(bufCoreData);
      }
      else if (!StrContains(bufCoreData, "Wave.defSacPointsMax:")) {
        defIndexSPM++;
        ReplaceString(bufCoreData, sizeof(bufCoreData), "Wave.defSacPointsMax:", "", true);
        DefaultsArray[defIndexSPM].defSacPointsMax = StringToInt(bufCoreData);
      }
    }
    CloseHandle(confCoreData);
  }
}
DEFAULTSMANAGER DefaultsManager;
DEFAULTSMANAGER GetDefaultsManager() { return DefaultsManager; }

//Core values
enum struct COREDATA {
  bool bgmPlaying;
  bool bombReset;
  bool canCrusaderNuke;
  bool canHindenburg;
  bool canHWBoss;
  bool canSENTMeteors;
  bool canSENTShark;
  bool canSENTStars;
  bool canTornado;
  bool doFailsafe;
  bool monitorOn;
  bool monitorColor;
  bool sacrificedByClient;
  bool sephiroth;
  bool shouldStopMusic;
  bool tornado;
  bool tacobell;
  bool tickingClientHealth;
  bool tickMusic;
  bool tickBGMOffset;
  char cachedPath[64];
  char configDefault[72];
  char configTacoBell[72];
  char configWaveNull[72];
  char songName[64];
  float HWNMin;
  float HWNMax;
  int BGMINDEX;
  int FailedCount;
  int camSel;
  int CodeEntry;
  int failsafeCount;
  int lastAdmin;
  int nullIndex;
  int refireTime;
  int SNDCHAN;
  int ticksMusic;
  int VIPBGM;
  int VIPIndex;
  int waveFlags;
  void init_pre() {
    char basePath[PLATFORM_MAX_PATH]; char plgName[64]; GetPluginFilename(INVALID_HANDLE, plgName, sizeof(plgName)); ReplaceString(plgName, sizeof(plgName), ".smx", "", false);
    Format(basePath, sizeof(basePath), "configs/%s/", plgName); BuildPath(Path_SM, basePath, sizeof(basePath), basePath); if(!DirExists(basePath)) CreateDirectory(basePath, 711);
    char subFolder[PLATFORM_MAX_PATH];
    AudioManager.OnStartup(0, basePath);
    WeatherManager.OnStartup(0, basePath);
    BossHandler.OnStartup();
    OnPhotonStartup(basePath);
    OnSudoStartup();
    Format(subFolder, sizeof(subFolder), "%s/Core", basePath);
    CreateDirectory(subFolder, 711);
    Format(subFolder, sizeof(subFolder), "%s/Files", basePath);
    CreateDirectory(subFolder, 711);
    LightLogger(LOGLVL_DEBUG, "[ConfigSystem @ v%s] HikarisRequiem folder structure created!", HRCFG_VERSION);
  }
  void init_post() {
    CPrintToChatAll("{gray}[Server]: Reloading!");
    LightLogger(LOGLVL_INFO, "[Server]: Reloading!");
    strcopy(this.configDefault, sizeof(this.configDefault), "addons/sourcemod/configs/HikarisRequiem/Core/WaveDefaults.ini");
    strcopy(this.configTacoBell, sizeof(this.configTacoBell), "addons/sourcemod/configs/HikarisRequiem/Core/WaveDefaults_TacoBell.ini");
    strcopy(this.configWaveNull, sizeof(this.configWaveNull), "addons/sourcemod/configs/HikarisRequiem/Core/WaveDefaults_WaveNull.ini");
    WaveSystem().SetActive(false);
    WaveSystem().SetWave(0);
    SetupASShop();
    SlotMachine.Init();
    for (int i = 0; i < sizeof(ass); i++) ass[i].price = 10 + (10 * i);
    for (int i = 0; i < sizeof(ass); i++) ass[i].purchase = 30 + i;
    LightLogger(1, "[ConfigSystem @ v%s] Setting up Crusader...", HRCFG_VERSION);
    Handle confCoreData = OpenFile("addons/sourcemod/configs/HikarisRequiem/BulkFire/crusader.ini", "rt", false);
    char bufCoreData[256];
    int crusaderIndex = -1;
    if (confCoreData == INVALID_HANDLE) return;
    while (ReadFileLine(confCoreData, bufCoreData, sizeof(bufCoreData))) {
      TrimString(bufCoreData);
      if (!StrContains(bufCoreData, "OnUser1")) {
        crusaderIndex++;
        Format(crusaderAtk[crusaderIndex].fireStr, 256, bufCoreData);
      }
      if (IsEndOfFile(confCoreData)) break;
    }
    LightLogger(1, "[ConfigSystem @ v%s] Setting up Lightning Flashes...", HRCFG_VERSION);
    confCoreData = OpenFile("addons/sourcemod/configs/HikarisRequiem/BulkFire/lightning_flash.ini", "rt", false);
    int lightningFlashIndex = -1;
    if (confCoreData == INVALID_HANDLE) return;
    while (ReadFileLine(confCoreData, bufCoreData, sizeof(bufCoreData))) {
      TrimString(bufCoreData);
      if (!StrContains(bufCoreData, "OnUser1")) {
        lightningFlashIndex++;
        Format(lightningFlash[lightningFlashIndex].fireStr, 256, bufCoreData);
      }
      if (IsEndOfFile(confCoreData)) break;
    }
    LightLogger(1, "[ConfigSystem @ v%s] Setting up Lightning Strikes...", HRCFG_VERSION);
    confCoreData = OpenFile("addons/sourcemod/configs/HikarisRequiem/BulkFire/lightning_strike.ini", "rt", false);
    int lightningStrikeIndex = -1;
    if (confCoreData == INVALID_HANDLE) return;
    while (ReadFileLine(confCoreData, bufCoreData, sizeof(bufCoreData))) {
      TrimString(bufCoreData);
      if (!StrContains(bufCoreData, "OnUser1")) {
        lightningStrikeIndex++;
        Format(lightningStrike[lightningStrikeIndex].fireStr, 256, bufCoreData);
      }
      if (IsEndOfFile(confCoreData)) break;
    }
    DefaultsManager.SetupDefaults(confCoreData);
    LightLogger(1, "[ConfigSystem @ v%s] Core data setup complete!", HRCFG_VERSION);
  }
  void Reset() {
    SephBoss[0].Reset();
    this.canHindenburg = false;
    this.canHWBoss = false;
    WaveSystem().SetActive(false);
    this.sephiroth = false;
    this.waveFlags = 0;
  }
}
COREDATA core;
COREDATA GetCoreData() { return core; }

//Messages to be printed when the fail safe has been triggered.
char failsafe[][256] =  {
  "{orange}🔥Burnt Mocha🍵 {white}: FAILSAFE HAS BEEN TRIGGERED. ROBOT EXTERMINATION IN PROGRESS. [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Nice try. You think this is funny? How about THIS instead? [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: (╯°□°）╯︵ ¡¡¡pƎ⅄O˥ԀƎp Ǝq O┴ ┴∩Oq∀ SI SS∀ S,ɹOSSƎℲOɹԀ [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Flips a coin and... HEADS! [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Pwofessow's b-butt is a-about to be depwoyed!!! JUST KIDDING - GET FUCKED. [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: We aren't so different, you and I. Oh wait, yes we are. You're a pile of scrap and I'm actually useful! :> [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: *BZZT* Sorry, time's up! [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: It's only me and you, who is gonna save you now? [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Nope. [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Sometimes I even amaze myself. [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Some disassembly required. [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Oops, I did it again! [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Not on my watch! [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: Sometimes you scare me! [Failsafe Activated]", 
  "{orange}🔥Burnt Mocha🍵 {white}: You know, I never really thought about it. [Failsafe Activated]"
};

bool CircleAOEenabled[] = {
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true,
  true
};
float CircleAOEpos[][3] =  {
  { 2370.0, -1670.0, -645.0 },
  { 1050.0, -1790.0, -535.0 },
  { 1150.0, -1520.0, -535.0 },
  { 1830.0, -1550.0, -600.0 },
  { 1925.0, -1875.0, -580.0 },
  { 1485.0, -1460.0, -585.0 },
  { 1630.0, -1680.0, -590.0 },
  { 1370.0, -1360.0, -570.0 },
  { 1350.0, -1690.0, -570.0 },
  { 2270.0, -900.0, -400.0 },
  { 1920.0, -490.0, -400.0 },
  { 1825.0, -650.0, -400.0 },
  { 980.0, -560.0, -400.0 },
  { 650.0, -880.0, -400.0 },
  { 1690.0, -870.0, -500.0 },
  { 1400.0, -870.0, -500.0 },
  { 1050.0, -980.0, -500.0 },
  { 1500.0, -480.0, -480.0 },
  { 800.0, -1100.0, -480.0 },
  { 520.0, -1400.0, -470.0 },
  { 550.0, -1730.0, -500.0 },
  { 1030.0, -1740.0, -530.0 }
};

//Return position
float Return[3] = {
  -3730.0,
  67.0,
  -252.0
};

//Bomb paths
char BOMBPATH[][32] =  {
  "bombpath_right_prefer_flankers", 
  "bombpath_left_prefer_flankers"
}

//Idle advertisements in chat
char AdvMessage[][256] =  {
  "{darkviolet}[{aqua}CORE{darkviolet}] {white}We have a Discord server: {forestgreen}https://discord.com/invite/ZfUUQWxCmw",
  "{darkviolet}[{aqua}CORE{darkviolet}] {white}Remember to buy your upgrades using {forestgreen}!buy",
  "{darkviolet}[{aqua}CORE{darkviolet}] {white}You may invoke {forestgreen}!sounds {white}to configure what sounds you hear from the plugin, or {forestgreen}!stats{white} to see your stats.",
  "{darkviolet}[{aqua}CORE{darkviolet}] {white}Advanced users may quick buy upgrades using {forestgreen}!qbuy",
  "{darkviolet}[{aqua}CORE{darkviolet}] {white}You may invoke {forestgreen}!sacpoints {white}to spend {forestgreen}sacrifice points{white} for many special abilities, including disabling the tornado!",
  "{darkviolet}[{aqua}CORE{darkviolet}] {white}TIP: As a {red}DEFENDER{white}, pushing your team's {forestgreen}payload {white}is crucial to wrecking havoc on the robots!",
  "{darkviolet}[{aqua}CORE{darkviolet}] {white}Remember, if someone is being abusive, you may always invoke {forestgreen}!calladmin{white}.",
  "{darkviolet}[{aqua}CORE{darkviolet}] {white}You may always invoke {forestgreen}!return{white} to be returned to spawn.",
  ""
};

//Random death messages
char DeathMessage[][64] = {
  "YEETED OUT INTO ORBIT",
  "SENT TO SPACE VIA TORNADO",
  "LAUNCHED ACROSS THE SKY",
  "SWALLOWED BY A STORM THAT WAS APPROACHING",
  "REDUCED TO DEBRIS",
  "FARTED INTO THE SKY",
  ""
};

//Scripted entities, used for spawning in extra entities mid wave.
char FB_SENT[][64] = {
  "FB.SentMeteor01",
  "FB.SentMeteor02",
  "FB.SentMeteor03",
  "FB.SentMeteor04",
  "FB.SentMeteor05",
  "FB.SentNuke01",
  "FB.SentNuke02",
  "FB.SentNuke03",
  "FB.SentNuke04",
  "FB.SentNuke05",
  "FB.SentStar01",
  "FB.SentStar02",
  "FB.SentStar03",
  "FB.SentStar04",
  "FB.SentStar05",
  ""
};

//RT camera system
char SelectedCamera[][32] = {
  "CAM.Front",
  "CAM.Mid",
  "CAM.MidTwo",
  "Cam.MidThree",
  "CAM.Rear",
  "CAM.RearTwo",
  "CAM.Spawn"
};

// Register other commands.
void RegisterMiscCommands() {
  LightLogger(1, "Registering misc commands...");
  RegConsoleCmd("sm_stats", Command_MyStats, "Print current statistics");
  RegConsoleCmd("sm_return", Command_Return, "Return to Spawn");
  RegConsoleCmd("sm_discord", Command_Discord, "Join our Discord server!");
  RegConsoleCmd("sm_aoe", Command_AOE, "[TESTING] Dispatch AOE Effect");
}

void RegisterAndPrecacheAllFiles() {
  LightLogger(LOGLVL_DEBUG, "Setting up Lasers...");
  //LoadConfig(3); -- Not implemented yet!!!
  char LsrName[16];
  for (int g = 0; g < 3; g++) { for (int i = 0; i < sizeof(WBLasers[g]); i++) { Format(LsrName, sizeof(LsrName), g == 0 ? "LaserEntWind%i" : g == 1 ? "LaserEntEarth%i" : g == 2 ? "LaserEntIce%i" : "LaserEnt%i", i); strcopy(WBLasers[g][i].tName, 16, LsrName); } }

  // Wind
  WBLasers[0][0].sPos = {5446.0,-2876.0,22.0};
  WBLasers[0][0].ePos = {5446.0,-2710.0,22.0};
  WBLasers[0][1].sPos = {5446.0,-2710.0,22.0};
  WBLasers[0][1].ePos = {5496.0,-2710.0,22.0};
  WBLasers[0][2].sPos = {5496.0,-2710.0,22.0};
  WBLasers[0][2].ePos = {5746.0,-2458.0,22.0};
  WBLasers[0][3].sPos = {5746.0,-2458.0,22.0};
  WBLasers[0][3].ePos = {5496.0,-2210.0,22.0};
  WBLasers[0][4].sPos = {5496.0,-2210.0,22.0};
  WBLasers[0][4].ePos = {5446.0,-2210.0,22.0};
  WBLasers[0][5].sPos = {5446.0,-2210.0,22.0};
  WBLasers[0][5].ePos = {5446.0,-2050.0,22.0};
  WBLasers[0][6].sPos = {5446.0,-2050.0,22.0};
  WBLasers[0][6].ePos = {6276.0,-2050.0,22.0};
  WBLasers[0][7].sPos = {6276.0,-2050.0,22.0};
  WBLasers[0][7].ePos = {6276.0,-2210.0,22.0};
  WBLasers[0][8].sPos = {6276.0,-2210.0,22.0}
  WBLasers[0][8].ePos = {6226.0,-2210.0,22.0}
  WBLasers[0][9].sPos = {6226.0,-2210.0,22.0};
  WBLasers[0][9].ePos = {5974.0,-2458.0,22.0};
  WBLasers[0][10].sPos = {5974.0,-2458.0,22.0};
  WBLasers[0][10].ePos = {6228.0,-2710.0,22.0};
  WBLasers[0][11].sPos = {6228.0,-2710.0,22.0};
  WBLasers[0][11].ePos = {6276.0,-2710.0,22.0};
  WBLasers[0][12].sPos = {6276.0,-2710.0,22.0};
  WBLasers[0][12].ePos = {6276.0,-2876.0,22.0};
  WBLasers[0][13].sPos = {6276.0,-2876.0,22.0};
  WBLasers[0][13].ePos = {5446.0,-2876.0,22.0};
  WBLasers[0][14].sPos = {5724.0,-2710.0,22.0};
  WBLasers[0][14].ePos = {5860.0,-2576.0,22.0};
  WBLasers[0][15].sPos = {5860.0,-2576.0,22.0};
  WBLasers[0][15].ePos = {6000.0,-2710.0,22.0};
  WBLasers[0][16].sPos = {6000.0,-2710.0,22.0};
  WBLasers[0][16].ePos = {5724.0,-2710.0,22.0};
  WBLasers[0][17].sPos = {5860.0,-2344.0,22.0};
  WBLasers[0][17].ePos = {5724.0,-2210.0,22.0};
  WBLasers[0][18].sPos = {5724.0,-2210.0,22.0};
  WBLasers[0][18].ePos = {5994.0,-2210.0,22.0};
  WBLasers[0][19].sPos = {5994.0,-2210.0,22.0};
  WBLasers[0][19].ePos = {5860.0,-2344.0,22.0};
  // Earth
  WBLasers[1][0].sPos = {5462.0,-2774.0,22.0};
  WBLasers[1][0].ePos = {5462.0,-2244.0,22.0};
  WBLasers[1][1].sPos = {5462.0,-2244.0,22.0};
  WBLasers[1][1].ePos = {5728.0,-2244.0,22.0};
  WBLasers[1][2].sPos = {5728.0,-2244.0,22.0};
  WBLasers[1][2].ePos = {5728.0,-2774.0,22.0};
  WBLasers[1][3].sPos = {5728.0,-2774.0,22.0};
  WBLasers[1][3].ePos = {5462.0,-2774.0,22.0}
  WBLasers[1][4].sPos = {5994.0,-2774.0,22.0};
  WBLasers[1][4].ePos = {5994.0,-2244.0,22.0};
  WBLasers[1][5].sPos = {5994.0,-2244.0,22.0};
  WBLasers[1][5].ePos = {6260.0,-2244.0,22.0};
  WBLasers[1][6].sPos = {6260.0,-2244.0,22.0};
  WBLasers[1][6].ePos = {6260.0,-2774.0,22.0};
  WBLasers[1][7].sPos = {6260.0,-2774.0,22.0};
  WBLasers[1][7].ePos = {5994.0,-2774.0,22.0};
  // Ice
  // Left edge
  WBLasers[2][0].sPos = {5328.0,-3042.0,22.0};
  WBLasers[2][0].ePos = {5328.0,-2050.0,22.0};

  // Left Top
  WBLasers[2][1].sPos = {5592.0,-2050.0,22.0};
  WBLasers[2][1].ePos = {5328.0,-2050.0,22.0};

  // Left Top-Right
  WBLasers[2][2].sPos = {5592.0,-2508.0,22.0};
  WBLasers[2][2].ePos = {5592.0,-2050.0,22.0};

  // Left Top Bridge-Top
  WBLasers[2][3].sPos = {5792.0,-2508.0,22.0};
  WBLasers[2][3].ePos = {5592.0,-2508.0,22.0};

  // Left Top Bridge-Bottom
  WBLasers[2][4].sPos = {5792.0,-2640.0,22.0};
  WBLasers[2][4].ePos = {5592.0,-2640.0,22.0};

  // Left Middle-Right
  WBLasers[2][5].sPos = {5592.0,-2640.0,22.0};
  WBLasers[2][5].ePos = {5592.0,-2774.0,22.0};

  // Left Bottom Bridge-Top
  WBLasers[2][6].sPos = {5792.0,-2774.0,22.0};
  WBLasers[2][6].ePos = {5592.0,-2774.0,22.0};

  // Left Bottom Bridge-Bottom
  WBLasers[2][7].sPos = {5792.0,-2906.0,22.0};
  WBLasers[2][7].ePos = {5592.0,-2906.0,22.0};

  // Left Bottom-Right
  WBLasers[2][8].sPos = {5592.0,-2906.0,22.0};
  WBLasers[2][8].ePos = {5592.0,-3042.0,22.0};

  // Left Bottom
  WBLasers[2][9].sPos = {5592.0,-3042.0,22.0};
  WBLasers[2][9].ePos = {5328.0,-3042.0,22.0};

  // Center Bottom-Left
  WBLasers[2][10].sPos = {5792.0,-3042.0,22.0};
  WBLasers[2][10].ePos = {5792.0,-2906.0,22.0};

  // Center Middle-Left
  WBLasers[2][11].sPos = {5792.0,-2774.0,22.0};
  WBLasers[2][11].ePos = {5792.0,-2640.0,22.0};

  // Center Top-Left
  WBLasers[2][12].sPos = {5792.0,-2508.0,22.0};
  WBLasers[2][12].ePos = {5792.0,-2374.0,22.0};

  // Center Top
  WBLasers[2][13].sPos = {5792.0,-2374.0,22.0};
  WBLasers[2][13].ePos = {5928.0,-2374.0,22.0};

  // Center Top-Right
  WBLasers[2][14].sPos = {5928.0,-2374.0,22.0};
  WBLasers[2][14].ePos = {5928.0,-2508.0,22.0};

  // Center Middle-Right
  WBLasers[2][15].sPos = {5928.0,-2640.0,22.0};
  WBLasers[2][15].ePos = {5928.0,-2774.0,22.0};

  // Center Bottom-Right
  WBLasers[2][16].sPos = {5928.0,-2906.0,22.0};
  WBLasers[2][16].ePos = {5928.0,-3042.0,22.0};

  // Center Bottom
  WBLasers[2][17].sPos = {5928.0,-3042.0,22.0};
  WBLasers[2][17].ePos = {5792.0,-3042.0,22.0};

  // Right Bottom-Left
  WBLasers[2][18].sPos = {6128.0,-3042.0,22.0};
  WBLasers[2][18].ePos = {6128.0,-2906.0,22.0};

  // Right Bottom Bridge-Bottom
  WBLasers[2][19].sPos = {6128.0,-2906.0, 22.0};
  WBLasers[2][19].ePos = {5928.0, -2906.0, 22.0};

  // Right Top Bridge-Top
  WBLasers[2][20].sPos = {6128.0,-2774.0, 22.0};
  WBLasers[2][20].ePos = {5928.0, -2774.0, 22.0};

  // Right Middle-Left
  WBLasers[2][21].sPos = {6128.0,-2774.0, 22.0};
  WBLasers[2][21].ePos = {6128.0,-2640.0, 22.0};

  // Right Top Bridge-Bottom
  WBLasers[2][22].sPos = {6128.0,-2640.0, 22.0};
  WBLasers[2][22].ePos = {5928.0,-2640.0, 22.0};

  // Right Bottom Bridge-Top
  WBLasers[2][23].sPos = {6128.0,-2508.0, 22.0};
  WBLasers[2][23].ePos = {5928.0,-2508.0, 22.0};

  // Right Top-Left
  WBLasers[2][24].sPos = {6128.0,-2508.0, 22.0};
  WBLasers[2][24].ePos = {6128.0,-2050.0, 22.0};

  // Right Top
  WBLasers[2][25].sPos = {6128.0,-2050.0, 22.0};
  WBLasers[2][25].ePos = {6392.0,-2050.0, 22.0};

  // Right Right
  WBLasers[2][26].sPos = {6392.0,-2050.0, 22.0};
  WBLasers[2][26].ePos = {6392.0,-3042.0, 22.0};

  // Right Bottom
  WBLasers[2][27].sPos = {6392.0,-3042.0, 22.0};
  WBLasers[2][27].ePos = {6128.0,-3042.0, 22.0};

  //Colors
  strcopy(WBLaserColors[0].cRGB, 16, "0 255 0");
  strcopy(WBLaserColors[1].cRGB, 16, "255 200 0");
  strcopy(WBLaserColors[2].cRGB, 16, "0 200 255");
  InitAllZones();
  SetVarDefaults();
}
//Wave Setup
char WaveSetup[][] = {
  "OnUser1 bombpath_right_arrows,Disable,,0.1,1", //Disable right arrows
  "OnUser1 bombpath_left_arrows,Disable,,0.1,1", //Disable left arrows
  "OnUser1 Classic_Mode_Intel1,Enable,,0.0,1", //Activate Intel 1
  "OnUser1 Classic_Mode_Intel2,Enable,,0.0,1", //Activate Intel 2
  "OnUser1 CommonSpells,Enable,,0.0,1", // Activate common spells
  "OnUser1 OldSpawn,Enable,,0.0,1", //Activate Old Spawn
  "OnUser1 NewSpawn,Disable,,0.0,1", //De-activate New Spawn
  "OnUser1 Classic_Mode_Intel1,Enable,,0.0,1", //Enable intel 1
  "OnUser1 Classic_Mode_Intel2,Enable,,0.0,1", //Enable intel 2
  "OnUser1 Classic_Mode_Intel3,Enable,,0.0,1",
  "OnUser1 Classic_Mode_Intel4,Enable,,0.0,1",
  "OnUser1 Classic_Mode_Intel5,Enable,,0.0,1",
  "OnUser1 Classic_Mode_Intel6,Enable,,0.0,1"
};

//Gets config file based on the game mode, we use the struct because returning differing array sizes is impossible otherwise.
char[] OVERRIDE_AND_GET_COREDATA() {
  int type = WaveSystem().GetType();
  switch (type) {
    case 0:return core.configDefault;
    case 1:return core.configTacoBell;
    case 2:return core.configWaveNull;
    default: {
      LogError("Gamemode %i out of bounds. Reverting to defaults.", type);
      return core.configDefault;
    }
  }
}

// Extra stuff that hasn't been moved to config files yet
void SetVarDefaults() {
  SetupBombSystem();
  core.bombReset = false;
  brawler_emergency = false;
  core.canCrusaderNuke = false;
  core.canHindenburg = false;
  core.canHWBoss = false;
  core.canSENTMeteors = false;
  core.canSENTShark = false;
  core.canSENTStars = false; SephBoss[0].shouldNuke = false;
  crusader = false;
  core.doFailsafe = false;
  core.monitorOn = false;
  core.monitorColor = true;
  core.sacrificedByClient = false;
  core.sephiroth = false;
  core.tacobell = false;
  core.tickingClientHealth = false;
  core.FailedCount = 0;
  INCOMINGDISPLAYED = 0;
  core.camSel = 0;
  core.CodeEntry = 0;
  core.failsafeCount = 0;
  core.lastAdmin = 0;
  core.nullIndex = 0;
  ass_sacPoints = 0;
  ass_sacPointsMax = 60;
  core.VIPIndex = 0;
  core.waveFlags = 0;
  core.HWNMin = 210.0;
  core.HWNMax = 380.0;
  HWBoss[0].index = 0;
  HWBoss[0].fireCount = 2;
  HWBoss[0].fireString = "OnUser1 hhh_maker,ForceSpawn,,0.0,1";
  HWBoss[1].index = 2;
  HWBoss[1].fireString = "OnUser1 hhh_maker2,ForceSpawn,,0.0,1";
  HWBoss[2].index = 3;
  HWBoss[2].fireCount = 1;
  HWBoss[2].fireString = "OnUser1 monoculus_maker,ForceSpawn,,0.0,1";
  HWBoss[3].index = 4;
  HWBoss[3].fireCount = 3;
  HWBoss[3].fireString = "OnUser1 hhh_maker2,ForceSpawn,,0.0,1";
  HWBoss[4].index = 7;
  HWBoss[4].fireString = "OnUser1 SkeleSpawner,Enable,,0.0,1";
  HWBoss[5].index = 8;
  HWBoss[5].fireString = "OnUser1 SkeleSpawner,Disable,,20.0,1";
  HWBoss[6].index = 5; //Does this access HWBoss[4]?
  HWBoss[6].fireCount = 2; //Does this disable skelespawner after enabling it?...
  HWBoss[7].index = 8;
  HWBoss[7].fireString = "OnUser1 merasmus_maker,ForceSpawn,,0.0,1";
  HWBoss[7].fireCount = 3;
  HWBoss[8].index = 11;
  HWBoss[8].fireCount = 2;
  HWBoss[8].fireString = "OnUser1 hhh_maker2,ForceSpawn,,0.0,1";
  HWBoss[9].index = 12;
  HWBoss[9].fireString = "OnUser1 monoculus_maker,ForceSpawn,,0.0,1";
  HWBoss[10].index = 0;
  HWBoss[10].fireCount = 2;
  HWBoss[10].fireString = "OnUser1 merasmus_maker,ForceSpawn,,0.0,1";
  HWBoss[11].index = 2;
  HWBoss[11].fireString = "OnUser1 monoculus_maker,ForceSpawn,,0.0,1";
  HWBoss[11].fireCount = 1;
  SephBoss[0].index = 0;
  SephBoss[0].fireCount = 0;
  SephBoss[0].shouldNuke = true;
  SephBoss[1].fireString = "OnUser1 SephMeteor,ForceSpawn,,0.0,1";
  SephBoss[1].ChatMessage = "{blue}Sephiroth: Say goodbye!";
  SephBoss[1].index = 2;
  SephBoss[1].fireCount = 1;
  SephBoss[2].fireString = "OnUser1 SephNuke,ForceSpawn,,0.0,1";
  SephBoss[2].index = 3;
  SephBoss[2].fireCount = 1;
  SephBoss[2].sound = SFXArray[8].realPath;
  SephBoss[3].fireString = "OnUser1 SephRocketSpammer,FireMultiple,50,0.0,1";
  SephBoss[3].index = 4;
  SephBoss[3].fireCount = 4;
  SephBoss[4].index = 3;
  SephBoss[4].fireCount = 3;
  SephBoss[4].fireString = "OnUser1 SephRocketSpammer,FireMultiple,10,3.0,1";
  SephBoss[5].index = 9;
  SephBoss[5].fireCount = 2;
  SephBoss[5].fireString = "OnUser1 SephRocketSpammer,FireMultiple,10,5.0,1";
  SephBoss[6].index = 9;
  SephBoss[6].fireCount = 2;
  SephBoss[6].fireString = "OnUser1 SkeleSpawner,Enable,,0.0,1";
  SephBoss[7].index = 3;
  SephBoss[7].fireCount = 3;
  SephBoss[7].fireString = "OnUser1 SkeleSpawner,Disable,,20.0,1";
  SephBoss[8].index = 4;
  SephBoss[8].fireCount = 1;
  SephBoss[9].index = 11;
  SephBoss[9].fireCount = 2;
  SephBoss[9].fireString = "OnUser1 SephRocketSpammer,FireOnce,,0.0,1";
  SephBoss[10].index = 12;
  SephBoss[10].fireCount = 1;
  SephBoss[10].fireString = "OnUser1 SephRocketSpammer,FireOnce,,5.0,1";
  SephBoss[11].index = 3;
  SephBoss[11].fireCount = 4;
}