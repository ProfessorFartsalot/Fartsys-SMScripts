//ASS_CONFIGSYSTEM - Contains the config loaders and all the plugin core data entries.

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
    char basePath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, basePath, sizeof(basePath), "configs/FartsysAss");
    CreateDirectory(basePath);
    char subFolder[PLATFORM_MAX_PATH];
    Format(subFolder, sizeof(subFolder), "%s/Files", basePath);
    CreateDirectory(subFolder);
    Format(subFolder, sizeof(subFolder), "%s/Weather", basePath);
    CreateDirectory(subFolder);
    Format(subFolder, sizeof(subFolder), "%s/BulkFire", basePath);
    CreateDirectory(subFolder);
    Format(subFolder, sizeof(subFolder), "%s/Core", basePath);
    CreateDirectory(subFolder);
    PrintToServer("FartsysAss folder structure created!");
  }
  void init_post() {
    CPrintToChatAll("{gray}[Server]: Reloading!");
    AssLogger(LOGLVL_INFO, "[Server]: Reloading!");
    char _path_WM[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, _path_WM, sizeof(_path_WM), "configs/FartsysAss/Weather/WeatherManager.cfg");
    if (!FileExists(_path_WM)) { GenerateDefaultConfig(2, _path_WM); }
    strcopy(this.configDefault, sizeof(this.configDefault),"addons/sourcemod/configs/FartsysAss/Core/WaveDefaults.ini");
    strcopy(this.configTacoBell, sizeof(this.configTacoBell),"addons/sourcemod/configs/FartsysAss/Core/WaveDefaults_TacoBell.ini");
    strcopy(this.configWaveNull, sizeof(this.configWaveNull),"addons/sourcemod/configs/FartsysAss/Core/WaveDefaults_WaveNull.ini");
    WaveSystem().SetActive(false);
    WaveSystem().SetWave(0);
    GlobalAudio.init();
    AssLogger(1, "Setting up bosses...");
    ass[0].name = "[10] Bath Salts";
    ass[1].name = "[20] Summon Goobbue or Kirby";
    ass[2].name = "[30] Robot Trap (Wave Fail-safe)";
    ass[3].name = "[40] Explosives Paradise";
    ass[4].name = "[50] Banish Tornadoes";
    ass[5].name = "[60] Ass Gas";
    ass[6].name = "[70] Instant Fat Man";
    ass[7].name = "[80] Meteorites";
    ass[8].name = "[90) 300,000 UbUp Cash";
    ass[9].name = "[100] Professor Fartsalot";
    for (int i = 0; i < sizeof(ass); i++) ass[i].price = 10 + (10 * i);
    for (int i = 0; i < sizeof(ass); i++) ass[i].purchase = 30 + i;
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
    HWBoss[6].fireCount=2;//Does this disable skelespawner after enabling it?...
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
    AssLogger(1, "[BULKFIRE] Setting up Crusader...");
    Handle confCoreData = OpenFile("addons/sourcemod/configs/FartsysAss/BulkFire/crusader.ini", "rt", false);
    char bufCoreData[256];
    int crusaderIndex = -1;
    if(confCoreData == INVALID_HANDLE) return;
    while (ReadFileLine(confCoreData, bufCoreData, sizeof(bufCoreData))){
      TrimString(bufCoreData);
      if (!StrContains(bufCoreData, "OnUser1")){
        crusaderIndex++;
        Format(crusaderAtk[crusaderIndex].fireStr, 256, bufCoreData);
      }
      if (IsEndOfFile(confCoreData)) break;
    }
    AssLogger(1, "[BULKFIRE] Setting up Lightning Flashes...");
    confCoreData = OpenFile("addons/sourcemod/configs/FartsysAss/BulkFire/lightning_flash.ini", "rt", false);
    int lightningFlashIndex = -1;
    if(confCoreData == INVALID_HANDLE) return;
    while (ReadFileLine(confCoreData, bufCoreData, sizeof(bufCoreData))){
      TrimString(bufCoreData);
      if (!StrContains(bufCoreData, "OnUser1")){
        lightningFlashIndex++;
        Format(lightningFlash[lightningFlashIndex].fireStr, 256, bufCoreData);
      }
      if (IsEndOfFile(confCoreData)) break;
    }
    AssLogger(1, "[BULKFIRE] Setting up Lightning Strikes...");
    confCoreData = OpenFile("addons/sourcemod/configs/FartsysAss/BulkFire/lightning_strike.ini", "rt", false);
    int lightningStrikeIndex = -1;
    if(confCoreData == INVALID_HANDLE) return;
    while (ReadFileLine(confCoreData, bufCoreData, sizeof(bufCoreData))){
      TrimString(bufCoreData);
      if (!StrContains(bufCoreData, "OnUser1")){
        lightningStrikeIndex++;
        Format(lightningStrike[lightningStrikeIndex].fireStr, 256, bufCoreData);
      }
      if (IsEndOfFile(confCoreData)) break;
    }
    AssLogger(1, "[CORE] Setting up Wave Defaults...");
    PrintToServer("confCoreData returned %s", OVERRIDE_AND_GET_COREDATA());
    confCoreData = OpenFile(OVERRIDE_AND_GET_COREDATA(), "rt", false);
    int defIndexHW, defIndexTornado, defIndexBGM, defIndexBS, defIndexBSM, defIndexSPM; // Start at 0 because we don't have a wave 0
    if(confCoreData == INVALID_HANDLE) return;
    while (ReadFileLine(confCoreData, bufCoreData, sizeof(bufCoreData))){
      TrimString(bufCoreData);
      if (!StrContains(bufCoreData, "Wave.defCanHWBoss:")){
        defIndexHW++;
        DefaultsArray[defIndexHW].defCanHWBoss = CharToBool(bufCoreData);
      }
      else if(!StrContains(bufCoreData, "Wave.defCanTornado:")){
        defIndexTornado++;
        DefaultsArray[defIndexTornado].defCanTornado = CharToBool(bufCoreData);
      }
      else if(!StrContains(bufCoreData, "Wave.defBGMIndex:")){
        defIndexBGM++;
        ReplaceString(bufCoreData, sizeof(bufCoreData), "Wave.defBGMIndex:", "", true);
        DefaultsArray[defIndexBGM].defBGMIndex = StringToInt(bufCoreData);
      }
      else if(!StrContains(bufCoreData, "Wave.defBombStatus:")){
        defIndexBS++;
        ReplaceString(bufCoreData, sizeof(bufCoreData), "Wave.defBombStatus:", "", true);
        DefaultsArray[defIndexBS].defBombStatus = StringToInt(bufCoreData);
      }
      else if(!StrContains(bufCoreData, "Wave.defBombStatusMax:")){
        defIndexBSM++;
        ReplaceString(bufCoreData, sizeof(bufCoreData), "Wave.defBombStatusMax:", "", true);
        DefaultsArray[defIndexBSM].defBombStatusMax = StringToInt(bufCoreData);
      }
      else if(!StrContains(bufCoreData, "Wave.defSacPointsMax:")){
        defIndexSPM++;
        ReplaceString(bufCoreData, sizeof(bufCoreData), "Wave.defSacPointsMax:", "", true);
        DefaultsArray[defIndexSPM].defSacPointsMax = StringToInt(bufCoreData);
      }
    }
    CloseHandle(confCoreData);
    AssLogger(1, "Core data setup complete!");
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

//Messages to be printed when the fail safe has been triggered.
char failsafe[][256] = {
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
float CircleAOEpos[][3] = {
  {2370.0, -1670.0, -645.0},
  {1050.0, -1790.0, -535.0},
  {1150.0, -1520.0, -535.0},
  {1830.0, -1550.0, -600.0},
  {1925.0, -1875.0, -580.0},
  {1485.0, -1460.0, -585.0},
  {1630.0, -1680.0, -590.0},
  {1370.0, -1360.0, -570.0},
  {1350.0, -1690.0, -570.0},
  {2270.0, -900.0, -400.0},
  {1920.0, -490.0, -400.0},
  {1825.0, -650.0, -400.0},
  {980.0, -560.0, -400.0},
  {650.0, -880.0, -400.0},
  {1690.0, -870.0, -500.0},
  {1400.0, -870.0, -500.0},
  {1050.0, -980.0, -500.0},
  {1500.0, -480.0, -480.0},
  {800.0, -1100.0, -480.0},
  {520.0, -1400.0, -470.0},
  {550.0, -1730.0, -500.0},
  {1030.0, -1740.0, -530.0}
};

//Return position
float Return[3] = {
  -3730.0,
  67.0,
  -252.0
};

//Bomb paths
char BOMBPATH[][32] = {
  "bombpath_right_prefer_flankers",
  "bombpath_left_prefer_flankers"
}

//Idle advertisements in chat
char AdvMessage[][256] = {
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
  "CAM.Rear",
  "CAM.Spawn"
};
//Easter eggs
char SpecEnt[][24] = {
  "Spec.Goobbue",
  "Spec.Waffle",
  "Spec.Burrito",
  "Spec.Shroom",
  "Spec.BlueBall",
  "Spec*"
};
//test code
char TBGM0[16] = "test/bgm0.mp3";
char TBGM1[16] = "test/bgm1.mp3";
char TBGM2[16] = "test/bgm2.mp3";
char TBGM3[16] = "test/bgm3.mp3";
char TBGM4[16] = "test/bgm4.mp3";
char TBGM5[16] = "test/bgm5.mp3";
char TBGM6[16] = "test/bgm6.mp3";

//Register our commands.
void RegisterAllCommands(){
  AssLogger(1, "Registering commands...");
  RegServerCmd("fb_operator", Command_Operator, "Serverside only. Does nothing when executed as client.");
  RegAdminCmd("sm_music", Command_Music, ADMFLAG_RESERVATION, "Set music to be played for the next wave");
  RegConsoleCmd("sm_bombstatus", Command_FBBombStatus, "Check bomb status");
  RegConsoleCmd("sm_song", Command_GetCurrentSong, "Get current song name");
  RegConsoleCmd("sm_stats", Command_MyStats, "Print current statistics");
  RegConsoleCmd("sm_return", Command_Return, "Return to Spawn");
  RegConsoleCmd("sm_sacpoints", Command_SacrificePointShop, "Fartsy's Annihilation Supply Shop");
  RegConsoleCmd("sm_discord", Command_Discord, "Join our Discord server!");
  RegConsoleCmd("sm_sounds", Command_Sounds, "Toggle sounds on or off via menu");
  RegConsoleCmd("sm_aoe", Command_AOE, "[TESTING] Dispatch AOE Effect");
  RegConsoleCmd("sm_fogsetdensity", Command_SetFogDensity, "[TESTING] Set fog density");
  RegConsoleCmd("sm_fogstartendupdate", Command_SetFogSEU, "[TESTING] Set fog start end with optional flag");
}

void RegisterAndPrecacheAllFiles(){
  PrecacheSound(TBGM0, true);
  PrecacheSound(TBGM2, true);
  PrecacheSound(TBGM3, true);
  PrecacheSound(TBGM4, true);
  PrecacheSound(TBGM5, true);
  PrecacheSound(TBGM6, true);
  PrintToServer("PRECACHING THESE SOUNDS SO EARLY IS NOT INTENTIONAL, PLEASE FIX THIS FARTSY.");
  PrecacheSound("fartsy/misc/brawler/fartsy_dll/intro.mp3", true);
  PrecacheSound("fartsy/misc/brawler/fartsy_dll/terror.mp3", true);
  PrecacheSound("fartsy/music/tcmusic/humanhypothesis.wav", true);
  AssLogger(0, "Loading EVERYTHING to RAM...");
  int bgmIndex;
  int sfxIndex;
  PrintToServer("Precaching BGMs...");
  char _path_BGM[PLATFORM_MAX_PATH];
  char _path_SFX[PLATFORM_MAX_PATH];
  BuildPath(Path_SM, _path_BGM, sizeof(_path_BGM), "configs/FartsysAss/Files/music.cfg");
  BuildPath(Path_SM, _path_SFX, sizeof(_path_SFX), "configs/FartsysAss/Files/sfx.cfg");
  if (!FileExists(_path_BGM)) { GenerateDefaultConfig(0, _path_BGM); }
  if (!FileExists(_path_SFX)) { GenerateDefaultConfig(1, _path_SFX); }
  AssLogger(LOGLVL_DEBUG, "Loading BGMs...");
  LoadConfig(0);
  AssLogger(LOGLVL_DEBUG, "Loading SFX...");
  LoadConfig(1);
  for (int i = 0; i < sizeof(BGMArray); i++) {
   if (!StrEqual(BGMArray[i].realPath, "")) {
    bgmIndex++;
    AssLogger(LOGLVL_DEBUG, "Precaching BGM: %s (%s), Duration %f seconds, Start Loop Point %i seconds, Volume %i   @ %i", BGMArray[i].songName, BGMArray[i].realPath, BGMArray[i].loopSeconds, BGMArray[i].introSeconds, BGMArray[i].SNDLVL, i);
    PrecacheSound(BGMArray[i].realPath, true);
   }
  }
  for (int i = 0; i < sizeof(SFXArray); i++) {
    if (!StrEqual(SFXArray[i].realPath, "")) {
      sfxIndex++;
      AssLogger(LOGLVL_DEBUG, "Precaching SFX: %s in position %i", SFXArray[i].realPath, i);
      PrecacheSound(SFXArray[i].realPath, true);
    }
  }
  AssLogger(LOGLVL_INFO, "Done! BGM Count: %i, SFX Count: %i, Total: %i files loaded to RAM.", bgmIndex, sfxIndex, bgmIndex + sfxIndex);
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
  "OnUser1 Classic_Mode_Intel1,Enable,,0.0,1", //enable intel 1
  "OnUser1 Classic_Mode_Intel2,Enable,,0.0,1", //enable intel 2
  "OnUser1 Classic_Mode_Intel3,Enable,,0.0,1",
  "OnUser1 Classic_Mode_Intel4,Enable,,0.0,1",
  "OnUser1 Classic_Mode_Intel5,Enable,,0.0,1",
  "OnUser1 Classic_Mode_Intel6,Enable,,0.0,1"
};

//Gets config file based on the game mode, we use the struct because returning differing array sizes is impossible otherwise.
char[] OVERRIDE_AND_GET_COREDATA() {
  switch (WaveSystem().GetType()) {
    case 0: {
      return core.configDefault;
    }
    case 1: {
      return core.configTacoBell;
    }
    case 2: {
      return core.configWaveNull;
    }
    default: {
      LogError("Gamemode %i out of bounds. Reverting to defaults.", WaveSystem().GetType());
      return core.configDefault;
    }
  }
}

// Generates an example config, for version 9 specifically
public bool GenerateDefaultConfig(const int type, const char[] path)
{
  Handle file = OpenFile(path, "w");
  if (file == null)
  {
    LogError("Failed to create config file: %s", path);
    return false;
  }
  switch(type) {
    // BGM
    case 0: {
      WriteFileLine(file, "\"Music\"");
      WriteFileLine(file, "{");
      for (int i = 0; i < sizeof(BGMArray); i++)
      {
        WriteFileLine(file, "\t\"%d\"", i);
        WriteFileLine(file, "\t{");
        WriteFileLine(file, "\t\t\"path\"\t\t\"\"");
        WriteFileLine(file, "\t\t\"name\"\t\t\"Example Song %d\"", i);
        WriteFileLine(file, "\t\t\"duration\"\t\"120.0\"");
        WriteFileLine(file, "\t\t\"intro\"\t\t\"0.0\"");
        WriteFileLine(file, "\t\t\"loop\"\t\t\"0.0\"");
        WriteFileLine(file, "\t\t\"volume\"\t\"80\"");
        WriteFileLine(file, "\t}");
      }
    }
    // SFX
    case 1: {
      WriteFileLine(file, "\"SFX\"");
      WriteFileLine(file, "{");
      for (int i = 0; i < sizeof(SFXArray); i++)
      {
        WriteFileLine(file, "\t\"%d\"", i);
        WriteFileLine(file, "\t{");
        WriteFileLine(file, "\t\t\"path\"\t\t\"\"");
        WriteFileLine(file, "\t\t\"name\"\t\t\"Example SFX %d\"", i);
        WriteFileLine(file, "\t\t\"volume\"\t\"80\"");
        WriteFileLine(file, "\t}");
      }
    }
    // Weather
    case 2: {
      WriteFileLine(file, "\"WeatherManager\"");
      WriteFileLine(file, "{");
      WriteFileLine(file, "\t\"defFogStartDist\"\t\"300\"");
      WriteFileLine(file, "\t\"defFogEndDist\"\t\"3000\"");
      WriteFileLine(file, "\t\"defFogDensity\"\t\"0.55\"");
      WriteFileLine(file, "\t\"fogChangeRate\"\t\"0.001\"");
      WriteFileLine(file, "\t\"fogChangeRateRGB\"\t\"0.25\"");
    }
  }
  WriteFileLine(file, "}");
  CloseHandle(file);
  LogMessage("Generated example config: %s", path);
  return true;
}

// Loads various config files
void LoadConfig(int type)
{
  Handle kv = CreateKeyValues(type == 0 ? "Music" : type == 1 ? "SFX" : "WeatherManager");
  char path[PLATFORM_MAX_PATH];
  BuildPath(Path_SM, path, sizeof(path), type == 0 ? "configs/FartsysAss/Files/Music.cfg" : type == 1 ? "configs/FartsysAss/Files/SFX.cfg" : "configs/FartsysAss/Weather/WeatherManager.cfg");
  if (!FileToKeyValues(kv, path))
  {
    CloseHandle(kv);
    SetFailState("Failed to read %s", path);
    return;
  }
  char index[32];
  int idx;
  switch (type) {
    // BGM
    case 0: {
      if (KvGotoFirstSubKey(kv)) {
        do {
          KvGetSectionName(kv, index, sizeof(index));
          idx = StringToInt(index);
          KvGetString(kv, "path", BGMArray[idx].realPath, sizeof(BGMArray[idx].realPath));
          char fsPath[PLATFORM_MAX_PATH];
          Format(fsPath, sizeof(fsPath), "sound/%s", BGMArray[idx].realPath);
          if (!FileExists(fsPath, true) && !StrEqual(BGMArray[idx].realPath, "")) { SetFailState("File does not exist: sound/%s (%s, field %d). Aborting Plugin Load.", BGMArray[idx].realPath, path, idx); }
          KvGetString(kv, "name", BGMArray[idx].songName, sizeof(BGMArray[].songName));
          BGMArray[idx].introSeconds = KvGetFloat(kv, "intro", 0.0);
          BGMArray[idx].loopSeconds  = KvGetFloat(kv, "duration", 0.0);
          BGMArray[idx].SNDLVL       = KvGetNum(kv, "volume", 80);
        } while (KvGotoNextKey(kv));
        CloseHandle(kv);
      }
    }
    // SFX
    case 1: {
      if (KvGotoFirstSubKey(kv)) {
        do {
          KvGetSectionName(kv, index, sizeof(index));
          idx = StringToInt(index);
          KvGetString(kv, "path", SFXArray[idx].realPath, sizeof(SFXArray[idx].realPath));
          char fsPath[PLATFORM_MAX_PATH];
          Format(fsPath, sizeof(fsPath), "sound/%s", SFXArray[idx].realPath);
          if (!FileExists(fsPath, true) && !StrEqual(SFXArray[idx].realPath, "")) { SetFailState("File does not exist: sound/%s (%s, field %d). Aborting Plugin Load.", SFXArray[idx].realPath, path, idx); }
          //KvGetString(kv, "name", SFXArray[idx].sfxName, sizeof(SFXArray[].sfxName));
          SFXArray[idx].SNDLVL       = KvGetNum(kv, "volume", 80);
        } while (KvGotoNextKey(kv));
        CloseHandle(kv);
      }
    }
    // Weather
    case 2: {
      KvGotoFirstSubKey(kv);
      KvGetString(kv, "defFogStartDist", WeatherManager.defFogStartDist, sizeof(WeatherManager.defFogStartDist), "300");
      KvGetString(kv, "defFogEndDist", WeatherManager.defFogEndDist, sizeof(WeatherManager.defFogEndDist), "3000");
      WeatherManager.defFogDensity = FloatMin(KvGetFloat(kv, "defFogDensity", 0.55), 1.0);
      WeatherManager.fogChangeRate = FloatMin(KvGetFloat(kv, "fogChangeRate", 0.001), 1.0);
      WeatherManager.fogChangeRateRGB = FloatMin(KvGetFloat(kv, "fogChangeRateRGB", 0.25), 1.0);
      WeatherManager.hasTornado = false;
      CloseHandle(kv);
    }
  }
}