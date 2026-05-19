// Hikari's Scene Enhancer (Inspired by Mrbt0907/Weather2Remastered)
char HSE_VER[8] = "2.8.1";

// Used to make lasers
enum struct LaserData {
  char tName[16];
  float sPos[3];
  float ePos[3];
}
LaserData WBLasers[3][28];

enum struct LaserColors {
  char cRGB[16];
}
LaserColors WBLaserColors[3];

// All background music
enum struct BGM {
  char realPath[64];
  char songName[64];
  float introSeconds;
  float loopSeconds;
  int SNDLVL;
}
BGM BGMArray[48];

// All sound effects
enum struct SFXARRAY {
  char realPath[64];
  int SNDLVL;
}
SFXARRAY SFXArray[132];

// Sound preference menu
char sndPrefs[][128] = {
  "Sounds are currently DISABLED",
  "Sounds are currently MUSIC ONLY",
  "Sounds are currently SOUND EFFECTS ONLY",
  "Sounds are currently ALL ON",
  "Somehow your sound preference was stored as non-existent 5... Please configure your sounds."
};
Handle cvarSNDDefault = INVALID_HANDLE;
Handle cvarSirenEnabledDefault = INVALID_HANDLE;
Handle cvarSirenMutesMusicDefault = INVALID_HANDLE;
int soundPreference[MAXPLAYERS + 1];
bool g_SirenMutesMusic[MAXPLAYERS + 1];
bool g_SirenEnabled[MAXPLAYERS + 1];

//Get client sound prefs
public void SQL_SNDPrefs(Database db, DBResultSet results, const char[] error, int client) {
  if (!results) { LogError("Failed to query database: %s", error); return; }
  if (!IsValidClient(client)) return;
  if (results.FetchRow()) { soundPreference[client] = results.FetchInt(0); g_SirenEnabled[client] = results.FetchInt(1) == 1 ? true : false; g_SirenMutesMusic[client] = results.FetchInt(2) == 1 ? true : false; }
}

// Music system rewrite for the 9th time.
/* I have rewritten this stupid thing 9 times now. It now exists in its final form. Aka, perfect alexander.
 * This is the audio manager. It plays music. It plays music PER CLIENT. It is good at playing MUSIC.
 * It syncs music. It tracks the EXACT MILLISECOND POSITION of the currently playing song.
 * It is... PERFECT.
*/
int g_chanBGM;
int g_indexBGM;
enum struct AUDIOSTATE {
  bool bgmPlaying;
  bool eventMode;
  bool shouldTick;
  bool stopBGM;
  bool hasTimeOffset;
  bool clientIsFresh;
  char cachedPath[128];
  char songName[128];
  float timeSeconds;
  float savedTimeSeconds;
  float loopSeconds;
  float savedLoopSeconds;
  float introSeconds;
  float savedIntroSeconds;
  int client;
  int savedIndexBGM;
  int indexBGM;
  int loops;
  int ticks;
  int savedLoops;
  int VIPBGM;
  int VIPIndex;
  int volume;
  int volume_sfx;
  float volume_real;
  float volume_real_sfx;
}
AUDIOSTATE AudioState[MAXPLAYERS+1];

methodmap AUDIOMANAGER {
  // Check if client is in event mode
  public bool isClientInEventMode(int client) { return AudioState[client].eventMode; }

  /** Gets the engine time for a client, accounting for intro offset seconds **/
  public float engineSecondsAdjusted(int client) {
    return GetEngineTime() + AudioState[client].introSeconds;
  }

  // Gets the default BGM
  public int GetDefaultBGM() {
    return WaveSystem().IsActive() ? (GetCoreData().sephiroth ? 16 : GetDefaultsManager().GetDefaults(WaveSystem().GetWave()).defBGMIndex) : GetRandomInt(1, 4);
  }

  // Initialise all AudioStates
  public void init() {
    LightLogger(LOGLVL_INFO, "Initializing Audio System...");
    g_chanBGM = 6;
    for (int i = 1; i < MaxClients; ++i) {
      AudioState[i].bgmPlaying = false;
      AudioState[i].stopBGM = false;
      AudioState[i].shouldTick = false;
      AudioState[i].cachedPath = "null";
      AudioState[i].songName = "null";
      AudioState[i].indexBGM = 0;
      AudioState[i].loopSeconds = 0.0;
      AudioState[i].clientIsFresh = true;
      AudioState[i].VIPIndex = -1;
      if (IsValidClient(i)) {
        AudioState[i].client = i;
        for (int x = 0; x < sizeof(BGMArray); ++x) StopSound(i, g_chanBGM, BGMArray[x].realPath);
      }
    }
    g_indexBGM = GetRandomInt(1, 4);
    this.UpdateBGM();
    CreateTimer(1.0, EnableAudio);
  }

  /* Reset all AudioStates
  * @param fullReset - Reset globally? true/false
  * @param client - client index to reset for
  */
  public void Reset(bool fullReset, int client = 0) {
    if (fullReset) {
      g_indexBGM = !WaveSystem().IsWaveNull() ? GetRandomInt(1, 4) : 28;
      for (int i = 0; i < MaxClients; ++i) this.Reset(false, i);
      LightLogger(LOGLVL_DEBUG, "All AudioStates have been reset!");
    }
    else {
      AudioState[client].stopBGM = true;
      AudioState[client].loops = 0;
      AudioState[client].indexBGM = 0;
      AudioState[client].timeSeconds = 0.0;
      if (IsValidClient(client)) for (int s = AudioState[client].indexBGM; s < sizeof(BGMArray); s++) StopSound(client, g_chanBGM, BGMArray[s].realPath);
      LightLogger(LOGLVL_DEBUG, "AudioState %i has been reset!", client);
    }
  }

  /* Changes the current music globally.
   * @param bgm - The index of the music to be played
   * @param instant - True = stops music NOW and changes, False = play current song then change
  */
  public void setBGM_Global(int bgm, bool instant) {
    for (int i = 0; i < MaxClients; ++i) if (IsValidClient(i)) { this.setBGM(i, bgm, instant); }
  }

  /* Changes the current music for <client>.
   * @param client - The client index to change music for
   * @param bgm - The index of the music to be played
   * @param instant - True = stops music NOW and changes, False = play current song then change
  */
  public void setBGM(int client, int bgm, bool instant) {
    //g_indexBGM = bgm == 0 ? GetRandomInt(1, 4) : bgm;
    if (IsValidClient(client)) PrintToChat(client, "Your BGM was changed. %i = %i > 0 ? %i : %i == 0 ? %i : %i, playing at sndlvl %i volume %f", AudioState[client].indexBGM, AudioState[client].VIPBGM, AudioState[client].VIPBGM, bgm, g_indexBGM, bgm, GetSNDLVL(client, true, BGMArray[AudioState[client].indexBGM].SNDLVL), GetVolume(client, true, 1.0));
    AudioState[client].indexBGM = AudioState[client].VIPBGM > 0 ? AudioState[client].VIPBGM : bgm == 0 ? g_indexBGM : bgm;
    LightLogger(LOGLVL_DEBUG, "[Enhancer @ v%s] Set %i for %N", HSE_VER, AudioState[client].indexBGM, client);
    if (instant) { AudioState[client].timeSeconds = 0.0; }
    AudioState[client].shouldTick = true;
    AudioState[client].hasTimeOffset = BGMArray[AudioState[client].indexBGM].introSeconds > 0 ? true : false;
    AudioState[client].stopBGM = !StrEqual(AudioState[client].cachedPath, BGMArray[AudioState[client].indexBGM].realPath) && !this.isClientInEventMode(client) ? true : false;
    AudioState[client].bgmPlaying = true;
  }

  /* Sets the VIP's chosen song for all clients */
  public void setBGM_VIP(int bgm) {
    bgm = bgm <= 0 ? this.GetDefaultBGM() : bgm;
    for (int i = 0; i < MaxClients; ++i)  if (IsValidClient(i) && !this.isClientInEventMode(i)) { AudioState[i].VIPBGM = bgm; }
  }

  /** Sets event mode for a client
   * @param client - The client index to set event mode for
   * @param bgm - The bgm index to set them to
   * @param activate - true or false (active/inactive)
   */
  public void setEventMode(int client, int bgm, int activate) {
    int curBGM = AudioState[client].indexBGM;
    if (activate == 1) {
      PrintToChat(client, "Saving %i for you", AudioState[client].indexBGM);
      AudioState[client].VIPBGM = 0;
      AudioState[client].savedIndexBGM = AudioState[client].indexBGM;
      AudioState[client].savedTimeSeconds = AudioState[client].timeSeconds;
      AudioState[client].savedLoopSeconds = AudioState[client].loopSeconds;
      AudioState[client].savedIntroSeconds = AudioState[client].introSeconds;
      AudioState[client].savedLoops = AudioState[client].loops;
      AudioState[client].eventMode = true;
      CSEClientEX(client, BGMArray[curBGM].realPath, -2, g_chanBGM, BGMArray[curBGM].SNDLVL, true, 1, 0.05, 100); // Set old bgm to low volume
      LightLogger(LOGLVL_WARN, "[Enhancer @ v%s] Entering event mode with song %s", HSE_VER, BGMArray[bgm].songName);
      this.setBGM(client, bgm, true);
    }
    else {
      StopSound(client, g_chanBGM, BGMArray[curBGM].realPath);
      AudioState[client].indexBGM = AudioState[client].savedIndexBGM;
      AudioState[client].loops = AudioState[client].savedLoops;
      AudioState[client].introSeconds = AudioState[client].savedIntroSeconds;
      AudioState[client].loopSeconds = AudioState[client].savedLoopSeconds;
      AudioState[client].timeSeconds = AudioState[client].savedTimeSeconds;
      int new_bgm = AudioState[client].indexBGM;
      CSEClientEX(client, BGMArray[new_bgm].realPath, -2, g_chanBGM, BGMArray[new_bgm].SNDLVL, true, 1, 1.0, 100);
      AudioState[client].eventMode = false;
      LightLogger(LOGLVL_WARN, "[Enhancer @ v%s] Exiting event mode with song %s", HSE_VER, BGMArray[new_bgm].songName);
    }
  }

  // Stops all music
  public void StopAll() { for (int i = 0; i < MaxClients; ++i) this.Stop(i); }

  // Stops for a client
  public void Stop(int client) {
    StopSound(client, g_chanBGM, BGMArray[AudioState[client].indexBGM].realPath);
    StopSound(client, g_chanBGM, AudioState[client].cachedPath);
    AudioState[client].shouldTick = false;
    AudioState[client].bgmPlaying = false;
    AudioState[client].indexBGM = 0;
    AudioState[client].loops = 0;
    AudioState[client].stopBGM = true;
    AudioState[client].timeSeconds = 0.0;
  }

  // Tick audio if a client should be ticking
  public void TickGlobal() { for (int i = 0; i < MaxClients; ++i) if (IsValidClient(i) && AudioState[i].shouldTick) this.TickClient(i); }

  // Tick audio for a client
  public void TickClient(int client) {
    if (!IsValidClient(client) || !AudioState[client].shouldTick) { this.Stop(client); return; }
    if (this.isClientInEventMode(client)) {
      if (AudioState[client].savedIndexBGM != g_indexBGM && AudioState[client].VIPBGM <= 0) AudioState[client].savedIndexBGM = g_indexBGM;
      if (this.engineSecondsAdjusted(client) > AudioState[client].savedTimeSeconds + AudioState[client].savedLoopSeconds) {
        AudioState[client].savedTimeSeconds = GetEngineTime();
        AudioState[client].savedLoops++;
        AudioState[client].savedIntroSeconds = AudioState[client].savedLoops >= 1 ? BGMArray[AudioState[client].savedIndexBGM].introSeconds : 0.0;
        AudioState[client].savedLoopSeconds = BGMArray[AudioState[client].savedIndexBGM].loopSeconds;
      }
    }
    AudioState[client].indexBGM = (!this.isClientInEventMode(client) && AudioState[client].VIPBGM > 0) ? AudioState[client].VIPBGM : (AudioState[client].indexBGM == 0 ? g_indexBGM : AudioState[client].indexBGM);
    if (this.engineSecondsAdjusted(client) > AudioState[client].timeSeconds + AudioState[client].loopSeconds) {
      LightLogger(LOGLVL_DEBUG, "[Enhancer @ v%s] Playing %i for %N", HSE_VER, AudioState[client].indexBGM, client);
      AudioState[client].timeSeconds = GetEngineTime();
      AudioState[client].loops++;
      if (AudioState[client].stopBGM) {
        LightLogger(LOGLVL_DEBUG, "[Enhancer @ v%s] Stopping %i for %N", HSE_VER,  AudioState[client].cachedPath);
        StopSound(client, g_chanBGM, AudioState[client].cachedPath);
        AudioState[client].loops = 0;
      }
      int index = AudioState[client].indexBGM;
      AudioState[client].stopBGM = StrContains(BGMArray[index].realPath, "mp3", false) != -1;
      CSEClientEX(client, BGMArray[index].realPath, -2, g_chanBGM, BGMArray[index].SNDLVL, true, 1, 1.0, 100);
      strcopy(AudioState[client].cachedPath, sizeof(AudioState[client].cachedPath), BGMArray[index].realPath);
      strcopy(AudioState[client].songName, sizeof(AudioState[client].songName), BGMArray[index].songName);
      AudioState[client].introSeconds = AudioState[client].loops >= 1 ? BGMArray[index].introSeconds : 0.0;
      AudioState[client].loopSeconds = BGMArray[index].loopSeconds;
      LightLogger(LOGLVL_DEBUG, "[AudioManager for %N]: We are on wave %i, now playing: %s (from %s) for %f seconds. It will start looping at %f seconds after the first cycle.", client, WaveSystem().GetWave(), AudioState[client].songName, AudioState[client].cachedPath, AudioState[client].loopSeconds, BGMArray[index].introSeconds);
    }
  }

  // Sets EVERYONE'S BGM in sync! Unless they're in an event...
  public void UpdateBGM() {
    for (int i = 0; i < MaxClients; ++i) {
      if (IsValidClient(i)) {
        if (this.isClientInEventMode(i) || !IsValidClient(i)) { PrintToServer("Client %N valid? %s Event Mode? %s", i, IsValidClient(i) ? "true" : "false", this.isClientInEventMode(i) ? "true" : "false"); continue; }
        this.setBGM(i, AudioState[i].VIPBGM > 0 && !this.isClientInEventMode(i) ? AudioState[i].VIPBGM : AudioState[i].indexBGM == 0 ? g_indexBGM : AudioState[i].indexBGM, true);
        AudioState[i].shouldTick = GetWeatherManager().hasTornadoWarning() && g_SirenMutesMusic[i] ? false : true;
      }
    }
  }

  // Enables AudioStates
  public void SetEnabled(bool state) { for (int i = 0; i < MaxClients; i++) { AudioState[i].shouldTick = state; } }

  /* When a client joins the server, set this audio manager to theirs */
  public void OnClientConnected(int client) {
    if (!IsValidClient(client) || IsFakeClient(client)) { PrintToServer("Invalid client %N", client); return; }
    AudioState[client].client = client;
    AudioState[client].clientIsFresh = true;
  }

  /* Handles client disconnects */
  public void OnClientDisconnect(int client) {
    PrintToServer("[AudioState @ %i] Client %N disconnected, shutting down gracefully!", client, client);
    AudioState[client].client = -1;
  }

  /* When a client respawns */
  public void OnClientRespawned(int client) {
    if (AudioState[client].clientIsFresh && IsValidClient(client)) {
      if (soundPreference[client] == 1 || soundPreference[client] == 3) CPrintToChat(client, "[AudioState v%s] Welcome! We are listening to %s", HSE_VER, BGMArray[g_indexBGM].songName);
      this.setBGM(client, g_indexBGM, true);
      AudioState[client].clientIsFresh = false;
    }
  }

  /* When a tornado warning is issued */
  public void OnTornadoWarning(bool issued) {
    for (int i = 0; i < MaxClients; i++) if (IsValidClient(i) && g_SirenEnabled[i] && g_SirenMutesMusic[i]) issued ? this.Stop(i) : this.setBGM(i, g_indexBGM, true);
  }
  
  /* When a client sets the volume */
  public void SetVolume(int client, int vol) { AudioState[client].volume = vol; PrintToChat(client, "Set your volume to %i", AudioState[client].volume); }

  /* When a client sets the volume */
  public void SetVolumeSFX(int client, int vol) { AudioState[client].volume_sfx = vol; PrintToChat(client, "Set your volume to %i", AudioState[client].volume_sfx); }

  /* This should be obvious by now */
  public void SetVolumeReal(int client, float vol) { AudioState[client].volume_real = vol; PrintToChat(client, "Set your volume to %f%", AudioState[client].volume_real * 100); }

  /* This should be obvious by now */
  public void SetVolumeRealSFX(int client, float vol) { AudioState[client].volume_real_sfx = vol; PrintToChat(client, "Set your volume to %f%%", AudioState[client].volume_real_sfx * 100); }

  public void OnStartup(int event, const char[] path) {
    char finalPath[PLATFORM_MAX_PATH]; strcopy(finalPath, sizeof(finalPath), path);
    switch (event) {
      // Create Configs
      case 0: {
        LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] AudioManager is starting up...", HSE_VER);
        FormatEx(finalPath, sizeof(finalPath), "%s/Files", finalPath); if (!DirExists(finalPath)) CreateDirectory(finalPath, 711);
        char MusicPath[PLATFORM_MAX_PATH];
        FormatEx(MusicPath, sizeof(MusicPath), "%s/Music.json", finalPath);
        if (!FileExists(MusicPath)) { 
          Handle file = OpenFile(MusicPath, "w");
          if (file == null) { LightLogger(LOGLVL_ERROR, "[Enhancer @ v%s] AudioManager failed to open config file: %s", HSE_VER, MusicPath); CloseHandle(file); return; }
          WriteFileLine(file, "{");
          WriteFileLine(file, "  \"Music\": {");
          for (int i = 0; i < sizeof(BGMArray); i++) {
            WriteFileLine(file, "    \"%d\": {", i);
            WriteFileLine(file, "      \"path\": \"\",");
            WriteFileLine(file, "      \"name\": \"Example Song %d\",", i);
            WriteFileLine(file, "      \"duration\": 120.0,");
            WriteFileLine(file, "      \"intro\": 0.0,");
            WriteFileLine(file, "      \"volume\": 80");
            WriteFileLine(file, i < sizeof(BGMArray) - 1 ? "    }," : "    }");
          }
          WriteFileLine(file, "  }");
          WriteFileLine(file, "}");
          CloseHandle(file);
          LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] AudioManager generated Default Config: %s", HSE_VER, MusicPath);
        }
        this.OnStartup(1, MusicPath);
        char SFXPath[PLATFORM_MAX_PATH];
        FormatEx(SFXPath, sizeof(SFXPath), "%s/SFX.json", finalPath);
        if (!FileExists(SFXPath)) {
          Handle file = OpenFile(SFXPath, "w");
          if (file == null) { LightLogger(LOGLVL_ERROR, "[Enhancer @ v%s] AudioManager failed to open config file: %s", HSE_VER, SFXPath); CloseHandle(file); return; }
          WriteFileLine(file, "{");
          WriteFileLine(file, "  \"SFX\": {");
          for (int i = 0; i < sizeof(SFXArray); i++) {
            WriteFileLine(file, "    \"%d\": {", i);
            WriteFileLine(file, "      \"path\": \"\",");
            WriteFileLine(file, "      \"name\": \"Example Sound %d\",", i);
            WriteFileLine(file, "      \"volume\": 80");
            WriteFileLine(file, i < sizeof(SFXArray) - 1 ? "    }," : "    }");
          }
          WriteFileLine(file, "  }");
          WriteFileLine(file, "}");
          CloseHandle(file);
          LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] AudioManager generated Default Config: %s", HSE_VER, SFXPath);
        }
        this.OnStartup(1, SFXPath);
        RegAdminCmd("sm_music", Command_Music, ADMFLAG_RESERVATION, "Set music to be played for the next wave");
        RegConsoleCmd("sm_song", Command_GetCurrentSong, "Get current song name");
        RegConsoleCmd("sm_sounds", Command_Sounds, "Toggle sounds on or off via menu");
        RegConsoleCmd("sm_volume", Command_SetVolume, "Set the music SNDLVL to whatever you want (up to 130)");
        RegConsoleCmd("sm_volume_sfx", Command_SetVolumeSFX, "Set the sfx SNDLVL to whatever you want (up to 130)");
        RegConsoleCmd("sm_setvolumereal", Command_SetVolumeReal, "Set the real volume for sfx/music. Ex: sm_setvolumereal sfx 100, sm_setvolumereal music 100");
        cvarSNDDefault = CreateConVar("hr_sound", "3", "Default sound for new users, 3 = Everything, 2 = Sounds Only, 1 = Music Only, 0 = Nothing");
      }
      // Load Configs
      case 1: {
        // Setup json loader
        Handle jsHandle = json_load_file(finalPath); if (jsHandle == INVALID_HANDLE) { CloseHandle(jsHandle); SetFailState("Failed to load json %s", finalPath); }
        LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] AudioManager is Loading Config %s...", HSE_VER, finalPath);

        // Attempt to locate headers
        bool isMusic = (StrContains(finalPath, "Music") != -1);
        char header[32]; strcopy(header, 32, isMusic ? "Music" : "SFX");
        Handle jsHeader = json_object_get(jsHandle, header);
        if (jsHeader == INVALID_HANDLE) { CloseHandle(jsHandle); CloseHandle(jsHeader); SetFailState("Failed to locate header, missing json root object \"%s\" in file %s", header, finalPath); }
        Handle jsIter = json_object_iter(jsHeader); char index[32]; int idx;
        while (jsIter != INVALID_HANDLE)
        {
          json_object_iter_key(jsIter, index, sizeof(index)); idx = StringToInt(index);
          Handle _jsKey = json_object_iter_value(jsIter);
          // It's Music
          if (isMusic) {
            json_object_get_string(_jsKey, "path", BGMArray[idx].realPath, sizeof(BGMArray[idx].realPath));
            json_object_get_string(_jsKey, "name", BGMArray[idx].songName, sizeof(BGMArray[idx].songName));
            BGMArray[idx].introSeconds = json_object_get_float(_jsKey, "intro");
            BGMArray[idx].loopSeconds = json_object_get_float(_jsKey, "duration");
            BGMArray[idx].SNDLVL = json_object_get_int(_jsKey, "volume");
            if (!StrEqual(BGMArray[idx].realPath, "")) {
              char fsPath[PLATFORM_MAX_PATH];
              Format(fsPath, sizeof(fsPath), "sound/%s", BGMArray[idx].realPath);
              if (!FileExists(fsPath, true)) SetFailState("File does not exist: %s (field %d)", fsPath, idx);
            }
          }
          else {
            json_object_get_string(_jsKey, "path", SFXArray[idx].realPath, sizeof(SFXArray[idx].realPath));
            SFXArray[idx].SNDLVL = json_object_get_int(_jsKey, "volume");
            if (!StrEqual(SFXArray[idx].realPath, "")) {
              char fsPath[PLATFORM_MAX_PATH];
              Format(fsPath, sizeof(fsPath), "sound/%s", SFXArray[idx].realPath);
              if (!FileExists(fsPath, true)) SetFailState("File does not exist: %s (field %d)", fsPath, idx);
            }
          }
          jsIter = json_object_iter_next(jsHeader, jsIter);
        }
        CloseHandle(jsIter); CloseHandle(jsHeader); CloseHandle(jsHandle);
        int bgmIndex;
        for (int i = 0; i < sizeof(BGMArray); i++) {
          if (!StrEqual(BGMArray[i].realPath, "")) {
            bgmIndex++;
            LightLogger(LOGLVL_DEBUG, "[Enhancer @ v%s] AudioManager is Precaching BGM: %s (%s), Duration %f seconds, Start Loop Point %f seconds, Volume %i @ %i", HSE_VER, BGMArray[i].songName, BGMArray[i].realPath, BGMArray[i].loopSeconds, BGMArray[i].introSeconds, BGMArray[i].SNDLVL, i);
            PrecacheSound(BGMArray[i].realPath, true);
          }
        }
        int sfxIndex;
        for (int i = 0; i < sizeof(SFXArray); i++) {
          if (!StrEqual(SFXArray[i].realPath, "")) {
            sfxIndex++;
            LightLogger(LOGLVL_DEBUG, "[Enhancer @ v%s] AudioManager is Precaching SFX: %s @ %i", HSE_VER, SFXArray[i].realPath, i);
            PrecacheSound(SFXArray[i].realPath, true);
          }
        }
        if (!StrEqual(BGMArray[1].realPath, "") && (!StrEqual(SFXArray[0].realPath, ""))) LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] AudioManager is Ready! BGM Count: %i, SFX Count: %i, Total: %i files precached.", HSE_VER, bgmIndex, sfxIndex, bgmIndex + sfxIndex);
      }
    }
  }
}
AUDIOMANAGER AudioManager;

public Action EnableAudio(Handle timer) {
  AudioManager.SetEnabled(true);
  return Plugin_Stop;
}

// Custom sound emitter, I don't know how many f-ing times I've rewritten this! +3! See potato.sp
// @param sndName The path to the sound to be played
// @param entity The entity to play from
// @param channel The channel to play to
// @param TSNDLVL The soundlevel to use
// @param isBGM Is it bgm?
// @param flags
//  `SND_NOFLAGS (0)` Nothing,
//  `SND_CHANGEVOL (1)` Changes volume,
//  `SND_CHANGEPITCH (2)` Changes pitch,
//  `SND_STOP (3)` Stops the sound,
//  `SND_SPAWNING (4)` Used in some cases for ambients,
//  `SND_DELAY (5)`, Sound has an initial delay,
//  `SND_STOPLOOPING (6)` Stop looping all sounds on the entity,
//  `SND_SPEAKER (7)` Being played by a mic through a speaker,
//  `SND_SHOULDPAUSE (8)` Pause if game is paused.
// @param vol Float 0.0 to 1.0 representing the volume
// @param pitch Int 0 to 255 representing the pitch
// @param pos pos[3] x y z where to play the sound
void CustomSoundEmitterEX(char[] sndName, int entity = -2, int channel = SNDCHAN_AUTO, int TSNDLVL, bool isBGM, int flags, float vol, int pitch, float pos[3] = NULL_VECTOR) {
  for (int i = 1; i <= MaxClients; i++) {
    if (!IsValidClient(i)) continue;
    if (isBGM && (soundPreference[i] == 1 || soundPreference[i] == 3) || !isBGM && soundPreference[i] >= 2) EmitSoundToClient(i, sndName, entity, isBGM ? g_chanBGM : channel, GetSNDLVL(i, isBGM, TSNDLVL), flags, GetVolume(i, isBGM, vol), pitch, _, pos, _, _, _);
  }
}

// Play sound to client. Originally ripped straight out of potato. Now supports source entities, channels, and positions.
void CSEClientEX(int client, char[] sndName, int entity = -2, int channel = SNDCHAN_AUTO, int TSNDLVL, bool isBGM, int flags, float vol, int pitch, float pos[3] = NULL_VECTOR) {
  if (!IsValidClient(client)) return;
  if (isBGM && (soundPreference[client] == 1 || soundPreference[client] == 3) || !isBGM && soundPreference[client] >= 2) EmitSoundToClient(client, sndName, entity, channel, GetSNDLVL(client, isBGM, TSNDLVL), flags, GetVolume(client, isBGM, vol), pitch, _, pos, _, _, _);
}

// Gets the client's preferred sound level - see the Valve Developer Wiki for valid SNDLVL indexes.
public int GetSNDLVL(int client, bool isBGM, int _default) { return isBGM ? (AudioState[client].volume != 0 ? AudioState[client].volume : _default) : !isBGM ? (AudioState[client].volume_sfx != 0 ? AudioState[client].volume_sfx : _default) : _default; }

// Gets the client's preferred REAL sound level - between 0.0 and 1.0;
public float GetVolume(int client, bool isBGM, float _default) { return isBGM ? (AudioState[client].volume_real != 0.0 ? AudioState[client].volume_real : _default) : !isBGM ? (AudioState[client].volume_real_sfx != 0.0 ? AudioState[client].volume_real_sfx : _default) : _default; }

// VIP Music Menu
public Action Command_Music(int client, int args) {
  int steamID = GetSteamAccountID(client);
  if (!steamID || steamID <= 10000) return Plugin_Handled;
  else ShowHikariMusicMenu(client);
  return Plugin_Handled;
}

// VIP Music Menu
public void ShowHikariMusicMenu(int client) {
  Menu menu = new Menu(MenuHandlerHikariMusic, MENU_ACTIONS_DEFAULT);
  char buffer[100];
  menu.SetTitle("Hikari's Music Menu");
  for (int i = 0; i < sizeof(BGMArray); i++) menu.AddItem(buffer, BGMArray[i].songName);
  menu.Display(client, MENU_TIME_FOREVER);
  menu.ExitButton = true;
}

// VIP Music Menu Handler
public int MenuHandlerHikariMusic(Menu menu, MenuAction action, int client, int bgm) {
  if (action == MenuAction_Select) {
    LightLogger(LOGLVL_WARN, "WE SHOULDNT BE USING TACO BELL BGM INDEX ANYMORE DUE TO NEW CONFIGS!!!");
    CPrintToChat(client, (bgm == 0 ? "{darkgreen}[CORE] Confirmed. Next song set to {aqua}Default{darkgreen}." : "{limegreen}[CORE] Confirmed. Next song set to {aqua}%s{limegreen}."), BGMArray[bgm].songName);
    AudioManager.setBGM_VIP(bgm == 0 ? -1 : bgm);
    for (int i = 0; i < MaxClients; ++i) { AudioState[i].stopBGM = (!StrEqual(AudioState[i].cachedPath, BGMArray[bgm].realPath) ? true : false); AudioState[i].VIPIndex = client; }
  } else if (action == MenuAction_End) CloseHandle(menu);
  return 0;
}

//Set Hikari Sound menu
public void HikarisSNDSelected(int client, MenuAction action, any info, char[] buffer, int maxlen) {
  if (action == MenuAction_Select) ShowHikariMenu(client);
}

// Get clients sound preferences then send them the menu
public Action Command_Sounds(int client, int args) {
  int steamID = GetSteamAccountID(client);
  if (!steamID || steamID <= 10000) return Plugin_Handled;
  else {
    char queryID[256];
    Format(queryID, sizeof(queryID), "SELECT soundprefs, sirenenabled, sirenmutesmusic FROM ass_activity WHERE steamid = '%d';", steamID);
    Ass_Database.Query(SQL_SNDPrefs, queryID, client);
    ShowHikariMenu(client);
    PrintToChat(client, "%s | Tornado Siren Enabled: %s | Tornado Siren Mutes Music: %s.", sndPrefs[soundPreference[client]], g_SirenEnabled[client] ? "True" : "False", g_SirenMutesMusic[client] ? "True" : "False");
    return Plugin_Handled;
  }
}

//Send client sound menu
public void ShowHikariMenu(int client) {
  Menu menu = new Menu(MenuHandlerHikari, MENU_ACTIONS_DEFAULT);
  char buffer[100];
  menu.SetTitle("Hikari's Sound Menu");
  menu.AddItem(buffer, "Disable ALL");
  menu.AddItem(buffer, "Music Only");
  menu.AddItem(buffer, "Sound Effects Only");
  menu.AddItem(buffer, "Enable ALL");
  menu.AddItem(buffer, g_SirenEnabled[client] ? "Disable Tornado Siren" : "Enable Tornado Siren")
  menu.AddItem(buffer, g_SirenMutesMusic[client] ? "Siren Mutes Music" : "Siren Does NOT Mute Music");
  menu.Display(client, 20);
  menu.ExitButton = true;
}

// Handle client choices for sound preference
public int MenuHandlerHikari(Menu menu, MenuAction action, int param1, int param2) {
  if (action == MenuAction_Select) {
    char query[256];
    int steamID = GetSteamAccountID(param1);
    if (!Ass_Database || !steamID) return 0;
    if (param2 < 4) {
      Format(query, sizeof(query), "UPDATE ass_activity SET soundprefs = '%i' WHERE steamid = '%d';", param2, steamID);
      soundPreference[param1] = param2;
    }
    if (param2 == 4) { Format(query, sizeof(query), "UPDATE ass_activity SET sirenenabled = CASE sirenenabled WHEN 0 THEN 1 ELSE 0 END WHERE steamid = '%d';", steamID); g_SirenEnabled[param1] = !g_SirenEnabled[param1] ? true : false; }
    if (param2 == 5) { Format(query, sizeof(query), "UPDATE ass_activity SET sirenmutesmusic = CASE sirenmutesmusic WHEN 0 THEN 1 ELSE 0 END WHERE steamid = '%d';", steamID); g_SirenMutesMusic[param1] = !g_SirenMutesMusic[param1] ? true : false; }
    Ass_Database.Query(Database_FastQuery, query);
    Command_Sounds(param1, 0);
  }
  else if (action == MenuAction_End) CloseHandle(menu);
  return 0;
}

//Light Entities
bool g_PowerOutage;
enum struct MAPLIGHTING {
  bool hasCustomPattern;
  bool isBroken;
  char arcs[32];
  char beam[32];
  char buzz[32];
  char explosion[35];
  char light[32];
  char status[32];
  char stun[32];
  char zap[32];
  void Explode() {
    if (this.isBroken) return;
    float dest[3]; GetEntPropVector(Photon.Find(this.light, "light_spot"), Prop_Send, "m_vecAbsOrigin", dest);
    dest[2] = dest[2] - 8.0;
    TeleportEntity(Photon.Find("MapLighting.StreetLamp.Explosion", "env_explosion"), dest, _, _);
    LightLogger(LOGLVL_WARN, "[Enhancer:584] MAPLIGHTING SHOULD NOT BE A CONSTANT HERE."); // In favor of configs
    Photon.Fire(this.arcs, "StartSpark", "", 0.0);
    Photon.Fire(this.arcs, "StartSpark", "", 0.1);
    Photon.Fire(this.beam, "LightOff", "", 0.0);
    Photon.Fire(this.buzz, "PlaySound", "", 0.0);
    Photon.Fire(this.explosion, "Explode", "", 0.0);
    Photon.Fire(this.light, "TurnOff", "", 0.0);
    Photon.Fire(this.status, "Color", "255 0 0", 0.0);
    Photon.Fire(this.stun, "Enable", "", 0.0);
    Photon.Fire(this.zap, "Enable", "", 0.0);
    this.isBroken = true;
  }
  void Repair() {
    this.isBroken = false;
    Photon.Fire(this.arcs, "StopSpark", "", 0.0);
    Photon.Fire(this.buzz, "StopSound", "", 0.25);
    Photon.Fire(this.status, "Color", "0 255 0", 0.0);
    Photon.Fire(this.stun, "Disable", "", 0.0);
    Photon.Fire(this.zap, "Disable", "", 0.0);
    if (!g_PowerOutage) {
      Photon.Fire(this.light, "SetPattern", "", 0.0);
      Photon.Fire(this.light, "TurnOff", "", 0.0);
      Photon.Fire(this.light, "TurnOn", "", 0.1);
      Photon.Fire(this.light, "SetPattern", "", 0.1);
      Photon.Fire(this.beam, "LightOff", "", 0.1);
      Photon.Fire(this.beam, "LightOn", "", 0.2);
    }
  }
  void RestorePower() {
    if (!this.isBroken || strcmp(this.light, "MapLighting.StreetLamp0C.light") == 0) {
      Photon.Fire(this.light, "TurnOn", "", 0.0);
      Photon.Fire(this.beam, "LightOn", "", 0.0);
      Photon.Fire(this.beam, "LightOn", "", 1.0);
      Photon.Fire(this.beam, "LightOn", "", 3.0);
      Photon.Fire(this.light, "SetPattern", "", 0.0);
      this.hasCustomPattern = false;
    }
  }
}
MAPLIGHTING MapLighting[13];

//Ghosties
methodmap GHOSTHANDLER {
  public void SpawnRandom() {
    Photon.Fire("GhostTemplate", "ForceSpawn");
    switch (GetRandomInt(0, 3)) {
      case 0: { Photon.Fire("gTrain", "TeleportToPathTrack", "gPath00_0"); }
      case 1: { Photon.Fire("gTrain", "TeleportToPathTrack", "gPath01_0"); }
      case 2: { Photon.Fire("gTrain", "TeleportToPathTrack", "gPath02_0"); }
      case 3: { Photon.Fire("gTrain", "TeleportToPathTrack", "gPath03_0"); }
    }
  }
  public void Kill() {
    Photon.Fire("gTelefect", "Start", "", 3.0);
    Photon.Fire("gTelefect", "Stop", "", 6.0);
    Photon.Fire("gTrain", "TeleportToPathTrack", "gPathHome", 3.0);
    Photon.SetEntityLife("gTrain", 7.0);
    Photon.SetEntityLife("gTrainTriggerStun", 7.0);
    Photon.SetEntityLife("gGhost", 7.0);
    Photon.SetEntityLife("gTelefect", 7.0);
    Photon.SetEntityLife("gTrainSnd*", 7.0);
    Photon.SetEntityLife("gPath0*", 7.0);
  }
}
GHOSTHANDLER GhostHandler;

// Weather Stuffs
int sirenRef;
PHOTON_BULK lightningStrike[16];
PHOTON_BULK lightningFlash[17];
enum struct WEATHERDATA {
  bool canTornado;
  bool hasTornado;
  bool powerSurging;
  bool reset;
  bool sirenExploded;
  bool tickWeather;
  bool TornadoTimerActive;
  bool TornadoWarning;
  char defRainDensity;
  char rainDensity;
  float fogDensity;
  float defFogDensity;
  float defFogEndDist;
  float defFogStartDist;
  float fogTarget;
  float fogChangeRate;
  float fogChangeRateRGB;
  float sirenPitch;
  float sirenPitchRate;
  float sirenPitchTarget;
  float fogColorR;
  float fogColorG;
  float fogColorB;
  float fogColorRTarget;
  float fogColorGTarget;
  float fogColorBTarget;
  float sirenPos[3];
  float skyHeight;
  int intensity;
  int mIntensity;
  int iStormMin;
  int iStormMax;
  int ticks;
}
WEATHERDATA WDATA;

methodmap WEATHERMANAGER {
  public bool CanFormTornado() { return WaveSystem().IsActive() && WDATA.canTornado && !WDATA.hasTornado; }
  public void Activate() {
    this.Reset();
    if (!WaveSystem().IsWaveNull()) WDATA.tickWeather = true; // Start weather system ONLY IF NOT IN GAMEMODE 2.
  }
  public void doLightning() {
    for (int i = 0; i < GetRandomInt(1, 4); i++) {
      int ent = CreateEntityByName("env_beam");
      if (ent != -1) {
        float sPos[3];
        sPos[0]= GetRandomFloat(g_WeatherZone.min[0], g_WeatherZone.max[0]);
        sPos[1] = GetRandomFloat(g_WeatherZone.min[1], g_WeatherZone.max[1]);
        sPos[2] = WDATA.skyHeight;
        float ePos[3];
        ePos[0]= GetRandomFloat(g_WeatherZone.min[0], g_WeatherZone.max[0]);
        ePos[1]= GetRandomFloat(g_WeatherZone.min[1], g_WeatherZone.max[1]);
        ePos[2] = -192.0;
        LightLogger(LOGLVL_WARN, "[Enhancer:708] ePos[2] should be configurable, as the lowest point where lightning can strike.");
        TeleportEntity(ent, sPos, NULL_VECTOR, NULL_VECTOR);
        SetEntityModel(ent, "sprites/laserbeam.spr");
        SetEntPropVector(ent, Prop_Data, "m_vecEndPos", ePos);
        DispatchKeyValue(ent, "targetname", "LightningStrike");
        DispatchKeyValue(ent, "rendercolor", "255 255 255");
        DispatchKeyValue(ent, "renderamt", "255");
        DispatchKeyValue(ent, "NoiseAmplitude", "32.0");
        DispatchKeyValue(ent, "TextureScroll", "25");
        DispatchKeyValue(ent, "framestart", "1");
        DispatchKeyValue(ent, "texture", "sprites/laserbeam.spr");
        DispatchKeyValue(ent, "framerate", "100");
        DispatchKeyValue(ent, "BoltWidth", "16.0");
        DispatchKeyValue(ent, "damage", "1000000");
        DispatchKeyValue(ent, "dissolvetype", "0");
        DispatchSpawn(ent);
        SetEntPropFloat(ent, Prop_Data, "m_fWidth", 16.0);
        SetEntPropFloat(ent, Prop_Data, "m_fEndWidth", 16.0);
        SetEntPropFloat(ent, Prop_Data, "m_fAmplitude", 32.0);
        ActivateEntity(ent);
        AcceptEntityInput(ent, "TurnOn");
        TeleportEntity(Photon.Find("MapLighting.StreetLamp.Explosion", "env_explosion"), ePos);
        Photon.Fire("MapLighting.StreetLamp.Explosion", "Explode");
        for (int c = 1; c <= MaxClients; c++) {
          if (!IsClientInGame(c) || !IsPlayerAlive(c)) continue;
          float entPos[3]; GetEntPropVector(c, Prop_Data, "m_vecAbsOrigin", entPos);
          if (GetVectorDistance(entPos, ePos) <= 300.0) AttackEntity(i, 10000000.0, DMG_ENERGYBEAM);
        }
        Photon.SetEntityLife("LightningStrike", GetRandomFloat(0.15, 0.30));
      }
    }
    sudo(1003);
    //Photon.Fire(lightningStrike[GetRandomInt(0, sizeof(lightningStrike) - 1)].fireStr);
    //Photon.Fire("Weather.LightningHurt*", "Disable", "", 0.07);
    CustomSoundEmitterEX(SFXArray[GetRandomInt(27, 34)].realPath, _, _, 65, false, 0, 1.0, 100);
    // Affect lights
    int i = GetRandomInt(WDATA.intensity, 15);
    LightLogger(LOGLVL_DEBUG, "Doing light interactions with %i", i);
    switch (i) {
      case 2, 3, 5: {
        int index = GetRandomInt(0, 12);
        float f = GetRandomFloat(0.5, 3.0);
        Photon.Fire(MapLighting[index].light, "TurnOff");
        Photon.Fire(MapLighting[index].beam, "LightOff");
        Photon.Fire(MapLighting[index].light, "TurnOn", "", f);
        Photon.Fire(MapLighting[index].beam, "LightOn", "", f);
      }
      case 11, 15: {
        g_PowerOutage = true;
        if (!WDATA.hasTornado) CreateTimer(GetRandomFloat(5.0, 30.0), RestorePower, GetRandomInt(0, 3));
        Photon.Fire("MapLighting.*", "LightOff");
        Photon.Fire("MapLighting.*", "TurnOff");
        Photon.Fire("MapLighting.*", "LightOff", "", 1.0);
        Photon.Fire("MapLighting.*", "LightOff", "", 2.0);
        Photon.Fire("MapLighting.*", "LightOff", "", 3.0);
        Photon.Fire("MapLighting.*", "LightOff", "", 4.0);
        Photon.Fire("MapLighting.*", "LightOff", "", 5.0);
      }
    }
  }
  public void Dissipate() {
    WDATA.sirenPitchTarget = 0.0;
    WDATA.sirenPitchRate = 0.25;
    WDATA.fogTarget = WDATA.defFogDensity;
    g_PowerOutage = false;
    WDATA.hasTornado = false;
    WDATA.mIntensity = WaveSystem().IsActive() ? 5 : 0;
    WDATA.intensity = WaveSystem().IsActive() ? 7 : 0;
    if (WDATA.sirenExploded) { WDATA.sirenPitch = 0.0; WDATA.sirenExploded = false; }
    WDATA.TornadoWarning = false;
    SetZoneActive(g_WeatherZone, false);
    CreateTimer(GetRandomFloat(45.0, 135.0), ResetTornado);
    for (int i = 0; i < sizeof(MapLighting); i++) if (!MapLighting[i].isBroken)Photon.Fire(MapLighting[i].light, "TurnOn");
    for (int i = 0; i < MaxClients; i++) { if (IsValidClient(i)) SetEntityGravity(i, 1.0); }
    Photon.Fire("Weather.FogOutdoor", "SetStartDistLerp", "%f", _, _, WDATA.defFogStartDist);
    Photon.Fire("Weather.FogOutdoor", "SetEndDistLerp", "%f", _, _, WDATA.defFogEndDist);
    Photon.Fire("Weather.FogOutdoor", "StartFogTransition");
    Photon.Fire("tornadof1", "Stop");
    Photon.Fire("tornadof1wind", "Disable");
    Photon.Fire("tornadowindf1", "StopSound");
    Photon.Fire("shaketriggerf1", "Disable");
    Photon.Fire("tornadobutton", "Unlock", "", 30.0);
    Photon.Fire("FB.FakeTankTank01", "Kill");
    Photon.Fire("FB.FakeTankPhys01", "Kill");
    Photon.Fire("Weather.EASScreen", "Disable");
    Photon.Fire("Weather.Siren.ExplodeParticle", "Stop");
    Photon.Fire("Weather.Siren", "StopSound", "", 5.0);
    AudioManager.OnTornadoWarning(false);
  }
  // Emits storm sounds
  public void EmitStormSounds() {
    switch (GetRandomInt(WDATA.mIntensity, WDATA.intensity)) {
        case 0: { CustomSoundEmitterEX(SFXArray[108].realPath, _, _, 65, false, 1, 0.1, 100); } // 0-3: Rain, no wind, reduced volume
        case 1: { CustomSoundEmitterEX(SFXArray[108].realPath, _, _, 65, false, 1, 0.1, 100); }
        case 2: { CustomSoundEmitterEX(SFXArray[108].realPath, _, _, 65, false, 1, 0.1, 100); }
        case 3: { CustomSoundEmitterEX(SFXArray[108].realPath, _, _, 65, false, 1, 0.1, 100); }
        case 4: {
          CustomSoundEmitterEX(SFXArray[108].realPath, _, _, 45, false, 1, 1.0, 100); //Rain, no wind, increased volume
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.1, 100); //Rain, wind, decreased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.01, 100); //Extra wind
        }
        case 5: {
          CustomSoundEmitterEX(SFXArray[108].realPath, _, _, 65, false, 1, 0.1, 100); //Rain, no wind, decreased volume
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 45, false, 1, 0.15, 100); //Rain, wind
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.05, 100); //Extra wind
        }
        case 6: {
          CustomSoundEmitterEX(SFXArray[108].realPath, _, _, 65, false, 1, 0.05, 100); //Rain, no wind, decreased volume
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.21, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.125, 100); //Extra wind
        }
        case 7: {
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 0);
          CustomSoundEmitterEX(SFXArray[108].realPath, _, _, 65, false, 1, 0.05, 100); //Rain, no wind, decreased volume
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.26, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.25, 100); //Extra wind
        }
        case 8: {
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 1);
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.37, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.35, 100); //Extra wind
        }
        case 9: {
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 2);
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.41, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.42, 100); //Extra wind
        }
        case 10: {
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 3);
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.45, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.56, 100); //Extra wind
        }
        case 11: {
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 4);
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.63, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.69, 100); //Extra wind
        }
        case 12: {
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 5);
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.72, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.76, 100); //Extra wind
        }
        case 13: {
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 6);
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.85, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.83, 100); //Extra wind
        }
        case 14: {
          Photon.Fire("rain", "Alpha", "255");
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 7);
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 0.9, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 0.92, 100); //Extra wind
        }
        case 15: {
          Photon.Fire("rain", "Alpha", "255");
          CreateTimer(GetRandomFloat(0.1, 1.25), GetRandomWind, 7);
          CustomSoundEmitterEX(SFXArray[107].realPath, _, _, 65, false, 1, 1.0, 100); //Rain, wind, increased volume
          //CustomSoundEmitterEX(SFXArray[109].realPath, 45, false, 1, 1.0, 100); //Extra wind
        }
      }
  }
  // Gets a safe tick delay to prevent divide by 0 errors / stalls
  public int getTickDelay() { return GetRandomInt(WDATA.iStormMin < 1 ? 1 : WDATA.iStormMin, WDATA.iStormMax < 2 ? 2 : WDATA.iStormMax); }
  public void IssueTornadoWarning() {
    if (WaveSystem().IsActive() && WDATA.canTornado && !WDATA.sirenExploded) {
      WDATA.TornadoWarning = true;
      WDATA.sirenPitchTarget = 100.0;
      WDATA.sirenPitchRate = 0.1;
      Photon.Fire("Weather.EASScreen", "Enable");
      Photon.Fire("Weather.EAS", "PlaySound");
      AudioManager.OnTornadoWarning(true);
    }
  }
  public void Reset() {
    LightLogger(LOGLVL_DEBUG, "[Enhancer @ v%s] WeatherManager has been reset!", HSE_VER);
    g_IndoorZones[0].tickrate = 77;
    g_IndoorZones[0].shouldTick = true;
    WDATA.fogDensity = 0.50;
    WDATA.fogColorRTarget = 55.0;
    WDATA.fogColorGTarget = 47.0;
    WDATA.fogColorBTarget = 47.0;
    this.Dissipate();
    WDATA.reset = true;
    WDATA.tickWeather = false;
    Photon.Fire("rain", "Alpha", "0");
    Photon.Fire("Weather.Sky", "Enable");
    Photon.Fire("Weather.Sky", "Skin", "0");
    Photon.Fire("Weather.FogSky", "Enable", "", 2.0);
    for (int i = 0; i < sizeof(MapLighting); i++) { MapLighting[i].Repair(); SetZoneActive(g_Lights[i], false); }
    WDATA.sirenPos = {-2305.0, 2095.0, 1102.0};
    int siren = Photon.Create("info_target", "WeatherSiren");
    if (!IsValidEntity(siren)) SetFailState("Could not inject Siren Entity, aborting!!!");
    TeleportEntity(siren, WDATA.sirenPos, NULL_VECTOR, NULL_VECTOR);
    sirenRef = EntIndexToEntRef(siren);
    LightLogger(LOGLVL_INFO, "Located info_target WeatherSiren!");
  }
  public void SetFogStartQueued(int fsq) { Photon.Fire("Weather.FogOutdoor", "SetStartDistLerpTo", "%i", _, _, fsq); }
  public void SetFogEndQueued(int feq) { Photon.Fire("Weather.FogOutdoor", "SetEndDistLerpTo","%i", _, _, feq); }
  public void StartFogTransition() { Photon.Fire("Weather.FogOutdoor", "StartFogTransition"); }
  public void StartTornadoTimer(float t) {
    CreateTimer(t - 20.0, TimedOperator, 40);
    CreateTimer((t + GetRandomFloat(5.0, 25.0)), TimedOperator, 41);
  }
  public void Tick() {
    if (WDATA.TornadoWarning) this.TickSiren();
    // Update intensity / queue wind sounds
    if (++WDATA.ticks % this.getTickDelay() == 0) {
      if (!WDATA.tickWeather) return;
      LightLogger(LOGLVL_DEBUG, "[Hikari's Enhancer] Storm Intensity @ %f% (%i)", (WDATA.intensity / 15.0), WDATA.intensity);
      this.doLightning();
      this.updateIntensity();
      WDATA.mIntensity = WDATA.intensity - 1;
      this.SetFogStartQueued(RoundToNearest(float (1000 - (WDATA.intensity * ((1000 - 150) / 15)))));
      this.SetFogEndQueued(RoundToNearest(float (3000 - (WDATA.intensity * ((3000 - 250) / 15)))));
      WDATA.fogTarget = 0.4 + (0.5 * (WDATA.intensity / 15));
      WDATA.iStormMin = RoundToNearest(float (1000 - (WDATA.intensity * ((1000 - 300) / 15))));
      WDATA.iStormMax = RoundToNearest(float (3000 - (WDATA.intensity * ((3000 - 700) / 15))));
      Photon.Fire("rain", "Alpha", "%i", _, _, RoundToNearest(255 * (WDATA.intensity / 15.0)));
      if (!WDATA.TornadoTimerActive && WDATA.intensity > 13 && this.CanFormTornado()) {
        WDATA.TornadoTimerActive = true;
        this.StartTornadoTimer(WaveSystem().GetWave() == 4 ? GetRandomFloat(30.0, 90.0) : GetRandomFloat(90.0, 210.0));
      }
      this.StartFogTransition();
      this.EmitStormSounds();
    }
    // Every 240 ticks: Assign player fog controllers.
    if (WDATA.ticks % 240 == 0) for (int i = 1; i < MaxClients; i++) { if (!IsClientInGame(i) || !IsClientConnected(i)) continue; else { SetVariantString(g_IsPlayerIndoors[i] ? "Weather.FogIndoor" : "Weather.FogOutdoor"); AcceptEntityInput(i, "SetFogController", i, i); }}
    // Every tick: Update fog
    if (WDATA.fogDensity != WDATA.fogTarget || WDATA.reset) {
      char targetAlpha[4];
      char targetDensity[24];
      WDATA.fogDensity = (WDATA.fogDensity < WDATA.fogTarget) ? Maths().FloatMin(Maths().Adjust(WDATA.fogDensity, WDATA.fogChangeRate), WDATA.fogTarget) : Maths().FloatMax(Maths().Adjust(WDATA.fogDensity, -WDATA.fogChangeRate), WDATA.fogTarget);
      IntToString(RoundFloat(Maths().FloatMin(255.0 * (WDATA.fogDensity * 1.20), 255.0)), targetAlpha, sizeof(targetAlpha));
      FloatToString(WDATA.fogDensity, targetDensity, sizeof(targetDensity));
      Photon.Fire("Weather.FogOutdoor", "SetMaxDensity", targetDensity);
      Photon.Fire("Weather.FogSky", "Alpha", targetAlpha);
    }
    if (WDATA.fogColorR != WDATA.fogColorRTarget || WDATA.fogColorG != WDATA.fogColorGTarget || WDATA.fogColorB != WDATA.fogColorBTarget || WDATA.reset) {
      char target[24];
      WDATA.fogColorR = (WDATA.fogColorR < WDATA.fogColorRTarget) ? Maths().FloatMin(WDATA.fogColorR += WDATA.fogChangeRateRGB, WDATA.fogColorRTarget) : Maths().FloatMax(WDATA.fogColorR -= WDATA.fogChangeRateRGB, WDATA.fogColorRTarget);
      WDATA.fogColorG = (WDATA.fogColorG < WDATA.fogColorGTarget) ? Maths().FloatMin(WDATA.fogColorG += WDATA.fogChangeRateRGB, WDATA.fogColorGTarget) : Maths().FloatMax(WDATA.fogColorG -= WDATA.fogChangeRateRGB, WDATA.fogColorGTarget);
      WDATA.fogColorB = (WDATA.fogColorB < WDATA.fogColorBTarget) ? Maths().FloatMin(WDATA.fogColorB += WDATA.fogChangeRateRGB, WDATA.fogColorBTarget) : Maths().FloatMax(WDATA.fogColorB -= WDATA.fogChangeRateRGB, WDATA.fogColorBTarget);
      Format(target, sizeof(target), "%i %i %i", RoundFloat(WDATA.fogColorR), RoundFloat(WDATA.fogColorG), RoundFloat(WDATA.fogColorB));
      Photon.Fire("Weather.FogIndoor", "SetColor", target);
      Photon.Fire("Weather.FogIndoor", "SetColorSecondary", target);
      Photon.Fire("Weather.FogOutdoor", "SetColor", target);
      Photon.Fire("Weather.FogOutdoor", "SetColorSecondary", target);
      Photon.Fire("Weather.FogSky", "Color", target);
      WDATA.reset = false;
    }
    if (WDATA.hasTornado && (WDATA.ticks % GetRandomInt(30, 60) == 0)) { this.TickTornado(); }
  }
  public void TickSiren() {
    if (WDATA.TornadoWarning) {
      if (!WDATA.sirenExploded && (WDATA.sirenPitch > 165.0)) {
        Photon.Fire("Weather.Siren.ExplodeParticle", "Start");
        CustomSoundEmitterEX(SFXArray[121].realPath, _, _, SFXArray[121].SNDLVL, false, 0, 1.0, 100);
        for (int i = 0; i < MaxClients; i++) if (IsValidClient(i) && g_SirenEnabled[i]) {
          StopSound(i, 10, SFXArray[119].realPath);
          StopSound(i, 10, SFXArray[120].realPath);
          StopSound(i, 3, SFXArray[119].realPath);
          StopSound(i, 3, SFXArray[120].realPath);
          StopSound(i, 2, SFXArray[119].realPath);
          StopSound(i, 2, SFXArray[120].realPath);
          StopSound(i, 6, SFXArray[119].realPath);
          StopSound(i, 6, SFXArray[120].realPath);
        }
        Photon.Fire("Weather.Siren.ExplodeParticle", "Stop", "", 5.0);
        Photon.Fire("Weather.Siren.ExplodeParticle", "Start", "", 5.1);
        WDATA.sirenExploded = true;
      }
      if (WDATA.sirenPitch != WDATA.sirenPitchTarget && !WDATA.sirenExploded) {
        WDATA.sirenPitch = (WDATA.sirenPitch < WDATA.sirenPitchTarget) ? Maths().FloatMin(WDATA.sirenPitch += WDATA.sirenPitchRate, WDATA.sirenPitchTarget) : Maths().FloatMax(WDATA.sirenPitch -= WDATA.sirenPitchRate, WDATA.sirenPitchTarget);
        int siren = EntRefToEntIndex(sirenRef);
        for (int i = 0; i < MaxClients; i++) if (IsValidClient(i) && g_SirenEnabled[i]) {
          CSEClientEX(i, SFXArray[119].realPath, siren, 10, SNDLEVEL_RAIDSIREN, false, 1026, 1.0, RoundFloat(WDATA.sirenPitch), WDATA.sirenPos); 
          CSEClientEX(i, SFXArray[120].realPath, siren, 10, SNDLEVEL_RAIDSIREN, false, 1026, 1.0, RoundFloat(WDATA.sirenPitch), WDATA.sirenPos);
          CSEClientEX(i, SFXArray[119].realPath, siren, 3, SNDLEVEL_RAIDSIREN, false, 1026, 1.0, RoundFloat(WDATA.sirenPitch), WDATA.sirenPos); 
          CSEClientEX(i, SFXArray[120].realPath, siren, 3, SNDLEVEL_RAIDSIREN, false, 1026, 1.0, RoundFloat(WDATA.sirenPitch), WDATA.sirenPos);
          CSEClientEX(i, SFXArray[119].realPath, siren, 2, SNDLEVEL_RAIDSIREN, false, 1026, 1.0, RoundFloat(WDATA.sirenPitch), WDATA.sirenPos); 
          CSEClientEX(i, SFXArray[120].realPath, siren, 2, SNDLEVEL_RAIDSIREN, false, 2, 1.0, RoundFloat(WDATA.sirenPitch), WDATA.sirenPos);
          CSEClientEX(i, SFXArray[119].realPath, siren, 6, SNDLEVEL_RAIDSIREN, false, 2, 0.7, RoundFloat(WDATA.sirenPitch), WDATA.sirenPos); 
          CSEClientEX(i, SFXArray[120].realPath, siren, 6, SNDLEVEL_RAIDSIREN, false, 2, 1.0, RoundFloat(WDATA.sirenPitch), WDATA.sirenPos);
        }
      }
      if (WDATA.ticks % GetRandomInt(70, 224) == 0) {
        if (WDATA.TornadoWarning && !WDATA.sirenExploded) {
          WDATA.sirenPitchTarget = WDATA.powerSurging ? GetRandomFloat(115.0, 210.0) : GetRandomFloat(65.0, 105.0);
          WDATA.sirenPitchRate = WDATA.powerSurging ? GetRandomFloat(0.0525, 0.25) : GetRandomFloat(0.01, 0.125);
        }
      }
    }
  }
  public void TickTornado() {
    float tornadoPos[3];
    bool found = false;
    int tornado = -1;
    while ((tornado = Photon.Find("tornadotrain", "func_tracktrain")) != -1) { GetEntPropVector(tornado, Prop_Send, "m_vecAbsOrigin", tornadoPos); found = true; break; }
    if (!found) { PrintToServer("No tornado found."); return; }
    float dest[3];
    dest[0] = GetRandomFloat(g_WeatherZone.min[0], g_WeatherZone.max[0]);
    dest[1] = GetRandomFloat(g_WeatherZone.min[1], g_WeatherZone.max[1]);
    dest[2] = 192.0;
    float dest2[3];
    dest2[0] = GetRandomFloat(g_WeatherZone.min[0], g_WeatherZone.max[0]);
    dest2[1] = GetRandomFloat(g_WeatherZone.min[1], g_WeatherZone.max[1]);
    dest2[2] = dest[2];
    TeleportEntity(Photon.Find("TorTrack0x00", "path_track"), dest);
    TeleportEntity(Photon.Find("TorTrack0x01", "path_track"), dest2);
    Photon.Fire("tornadotrain", tornadoPos[0] == dest[0] ? "StartBackward" : "StartForward");
    int maxEntities = GetMaxEntities();
    for (int i = 0; i <= maxEntities; i++) if (IsValidEntity(i)) if (HasEntProp(i, Prop_Send, "m_vecAbsOrigin")) {
      if (i < MaxClients) if (IsValidClient(i)) if (IsClientObserver(i)) continue;
      float entPos[3]; GetEntPropVector(i, Prop_Send, "m_vecAbsOrigin", entPos);
      float dx = tornadoPos[0] - entPos[0];
      float dy = tornadoPos[1] - entPos[1];
      float distance2D = SquareRoot(dx*dx + dy*dy);
      float str = 1.0 - ((distance2D / 2150.0) * (distance2D / 2150.0));
      if (distance2D <= 1700.0) {
        float v[3]; GetEntPropVector(i, Prop_Data, "m_vecVelocity", v);
        v[2] = (3000.0 * str);
        TeleportEntity(i, NULL_VECTOR, NULL_VECTOR, v);
        char classname[32]; GetEntityClassname(i, classname, sizeof(classname));
        if (StrEqual(classname, "tank_boss")) { AttackEntity(i, 10000000.0, DMG_GENERIC); Photon.Fire("FB.FakeTankSpawner", "ForceSpawn"); }
      }
      SetEntityGravity(i, distance2D <= 2150.0 ? str : 1.0);
    }
  }
  public void TryFormTornado() {
    if (this.CanFormTornado() && Photon.CanCreate(7)) {
      WDATA.hasTornado = true;
      WDATA.mIntensity = 15;
      WDATA.intensity = 15;
      float f = GetRandomFloat(60.0, 120.0);
      SetupAttackZone(9000000.0, DMG_FALL, g_WeatherZone, false, 32, _, f);
      CreateTimer(f, TimedOperator, 42);
      CreateTimer(1.5, TornadoShaker);
      Photon.Fire("tornado", "ForceSpawn");
      Photon.Fire("tornadobutton", "Lock");
      Photon.Fire("tornadof1", "Start", "", 20.0);
      Photon.Fire("shaketriggerf1", "Enable", "", 20.0);
      Photon.Fire("tornadowindf1", "PlaySound", "", 20.0);
      Photon.Fire("tornadof1wind", "Enable", "", 21.50);
      Photon.SetEntityLife("tornadotrain", f);
      Photon.SetEntityLife("TorTrack*", f);
      Photon.SetEntityLife("tornadotrain", f);
      Photon.SetEntityLife("tornadof1", f);
      Photon.SetEntityLife("tornadoshake_f1", f);
      Photon.SetEntityLife("tornadowindf1", f);  
    }
  }
  public void updateIntensity() {
    switch (GetRandomInt(0, 10)) {
      case 0, 4, 8: {
        if (WDATA.intensity > 9) GhostHandler.SpawnRandom();
        if (WDATA.intensity > 0 && (WDATA.intensity - 1 > WDATA.mIntensity)) WDATA.intensity--;
      }
      case 1, 2, 3, 5, 6, 7, 9, 10: { if (WDATA.intensity < 15) WDATA.intensity++; }
    }
  }
  public void OnStartup(int event, const char[] path) {
    char finalPath[PLATFORM_MAX_PATH]; strcopy(finalPath, sizeof(finalPath), path);
    switch (event) {
      // Create Configs
      case 0: {
        LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] WeatherManager is starting up...", HSE_VER);
        FormatEx(finalPath, sizeof(finalPath), "%s/Weather", finalPath); if (!DirExists(finalPath)) CreateDirectory(finalPath, 711);
        FormatEx(finalPath, sizeof(finalPath), "%s/WeatherManager.json", finalPath);
        if (!FileExists(finalPath)) { 
          Handle file = OpenFile(finalPath, "w");
          if (file == null) { LightLogger(LOGLVL_ERROR, "Failed to create config file: %s", finalPath); CloseHandle(file); return; }
          WriteFileLine(file, "{");
          WriteFileLine(file, "  \"WeatherManager\": {");
          WriteFileLine(file, "    \"defFogStartDist\": 300.0,");
          WriteFileLine(file, "    \"defFogEndDist\": 3000.0,");
          WriteFileLine(file, "    \"defFogDensity\": 0.55,");
          WriteFileLine(file, "    \"fogChangeRate\": 0.001,");
          WriteFileLine(file, "    \"fogChangeRateRGB\": 0.25,");
          WriteFileLine(file, "    \"skyHeight\": 1200.0,");
          char out[40];
          for (int i = 0; i < sizeof(MapLighting); i++) {
            Format(out, sizeof(out), "    \"MapLighting%iArcs\": \" \",", i); WriteFileLine(file, out);
            Format(out, sizeof(out), "    \"MapLighting%iBeam\": \" \",", i); WriteFileLine(file, out);
            Format(out, sizeof(out), "    \"MapLighting%iBuzz\": \" \",", i); WriteFileLine(file, out);
            Format(out, sizeof(out), "    \"MapLighting%iDamage\": 7000,", i); WriteFileLine(file, out);
            Format(out, sizeof(out), "    \"MapLighting%iLight\": \" \",", i); WriteFileLine(file, out);
            Format(out, sizeof(out), "    \"MapLighting%iStatusSprite\": \" \",", i); WriteFileLine(file, out);
            Format(out, sizeof(out), "    \"MapLighting%iStatusEffect\": \"TFCond_MVMBotRadiowave\",", i); WriteFileLine(file, out);
            Format(out, sizeof(out), i != sizeof(MapLighting) - 1 ? "    \"MapLighting%iZap\": \" \"," : "    \"MapLighting%iZap\": \" \"", i); WriteFileLine(file, out);
          }
          WriteFileLine(file, "  }");
          WriteFileLine(file, "}");
          LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] Generated Default Config: %s", HSE_VER, finalPath);
          CloseHandle(file);
        }
        RegConsoleCmd("sm_fogsetdensity", Command_SetFogDensity, "[TESTING] Set fog density");
        RegConsoleCmd("sm_fogstartendupdate", Command_SetFogSEU, "[TESTING] Set fog start end with optional flag");
        cvarSirenEnabledDefault = CreateConVar("hr_sirenenabled", "1", "Enables the tornado siren for clients to hear it by default (they can still opt in/out of it via !sounds). 1/0");
        cvarSirenMutesMusicDefault = CreateConVar("hr_sirenmutemusic", "1", "Allows the tornado siren to mute music for clients by default (they can still opt in/out of it via !sounds). 1/0");
        this.OnStartup(1, finalPath);
      }
      // Load Configs
      case 1 : {
        // Setup json loader
        Handle jsHandle = json_load_file(finalPath); if (jsHandle == INVALID_HANDLE) { CloseHandle(jsHandle); SetFailState("Failed to load json %s", finalPath); }
        LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] WeatherManager is Loading Config %s...", HSE_VER, finalPath);
      
        // Attempt to locate headers
        Handle jsHeader = json_object_get(jsHandle, "WeatherManager");
        if (jsHeader == INVALID_HANDLE) { CloseHandle(jsHandle); CloseHandle(jsHeader); SetFailState("Failed to locate header, missing json root object \"WeatherManager\" in file %s", finalPath); }
        Handle jsIter = json_object_iter(jsHeader);
        while (jsIter != INVALID_HANDLE) {
          this.setDefFogStartDist(json_object_get_float(jsHeader, "defFogStartDist"));
          this.setDefFogEndDist(json_object_get_float(jsHeader, "defFogEndDist"));
          this.setDefFogDensity(json_object_get_float(jsHeader, "defFogDensity"));
          this.setFogChangeRate(json_object_get_float(jsHeader, "fogChangeRate"));
          this.setFogChangeRateRGB(json_object_get_float(jsHeader, "fogChangeRateRGB"));
          this.setSkyHeight(json_object_get_float(jsHeader, "skyHeight"));
          for (int i = 0; i < sizeof(MapLighting); i++) {
            char key[40];
            Format(key, sizeof(key), "MapLighting%iArcs", i); json_object_get_string(jsHeader, key, MapLighting[i].arcs, sizeof(MapLighting[i].arcs));
            Format(key, sizeof(key), "MapLighting%iBeam", i); json_object_get_string(jsHeader, key, MapLighting[i].beam, sizeof(MapLighting[i].beam));
            Format(key, sizeof(key), "MapLighting%iBuzz", i); json_object_get_string(jsHeader, key, MapLighting[i].buzz, sizeof(MapLighting[i].buzz));
            Format(key, sizeof(key), "MapLighting%iDamage", i); g_Lights[i].dmg = json_object_get_float(jsHeader, key);
            Format(key, sizeof(key), "MapLighting%iLight", i); json_object_get_string(jsHeader, key, MapLighting[i].light, sizeof(MapLighting[i].light));
            Format(key, sizeof(key), "MapLighting%iStatusSprite", i); json_object_get_string(jsHeader, key, MapLighting[i].status, sizeof(MapLighting[i].status));
            //Format(key, sizeof(key), "MapLighting%iStatusEffect", i); //json_object_get_string(jsHeader, key, g_Lights[i].statuseffect, sizeof(g_Lights[i].statuseffect));
            Format(key, sizeof(key), "MapLighting%iZap", i); json_object_get_string(jsHeader, key, MapLighting[i].zap, sizeof(MapLighting[i].zap));
            if (StrContains(MapLighting[i].arcs, " ", false) != -1) ThrowError("WeatherManager MapLighting %i Arcs can not be null!", i);
            if (StrContains(MapLighting[i].beam, " ", false) != -1) ThrowError("WeatherManager MapLighting %i Beam can not be null!", i);
            if (StrContains(MapLighting[i].buzz, " ", false) != -1) ThrowError("WeatherManager MapLighting %i Buzz can not be null!", i);
            if (StrContains(MapLighting[i].light, " ", false) != -1) ThrowError("WeatherManager MapLighting %i Light can not be null!", i);
            if (StrContains(MapLighting[i].status, " ", false) != -1) ThrowError("WeatherManager MapLighting %i Status can not be null!", i);
            if (StrContains(MapLighting[i].zap, " ", false) != -1) ThrowError("WeatherManager MapLighting %i Zap can not be null!, i");
          }
          jsIter = json_object_iter_next(jsHeader, jsIter);
        }
        CloseHandle(jsIter); CloseHandle(jsHeader); CloseHandle(jsHandle);
        LightLogger(LOGLVL_INFO, "[Enhancer @ v%s] WeatherManager is Ready!", HSE_VER);
      }
    }
  }
  public void setCanTornado(bool c) { WDATA.canTornado = c; }
  public void setPowerSurging(bool p) { WDATA.powerSurging = p; }
  public void setTornadoTimerActive(bool t) { WDATA.TornadoTimerActive = t; }
  public bool canTornado() { return WDATA.canTornado; }
  public bool hasTornado() { return WDATA.hasTornado; }
  public bool hasTornadoWarning() { return WDATA.TornadoWarning; }
  public bool isPowerSurging() { return WDATA.powerSurging; }
  public void setFogDensity(float d) { WDATA.fogDensity = d; }
  public void setFogColorRTarget(float r) { WDATA.fogColorRTarget = r; }
  public void setFogColorGTarget(float g) { WDATA.fogColorRTarget = g; }
  public void setFogColorBTarget(float b) { WDATA.fogColorRTarget = b; }
  public void setFogTarget(float t) { WDATA.fogTarget = t; }
  public void setDefFogStartDist(float s) { WDATA.defFogStartDist = s; }
  public void setDefFogEndDist(float e) { WDATA.defFogEndDist = e; }
  public void setDefFogDensity(float d) { WDATA.defFogDensity = d; }
  public void setFogChangeRate(float r) { WDATA.fogChangeRate = r; }
  public void setFogChangeRateRGB(float rgb) { WDATA.fogChangeRateRGB = rgb; }
  public void setSkyHeight(float sH) { WDATA.skyHeight = sH; }
}
WEATHERMANAGER WeatherManager;
WEATHERMANAGER GetWeatherManager() { return WeatherManager; }

//Timers
public Action GetRandomWind(Handle timer, int intensity) {
  CustomSoundEmitterEX(SFXArray[GetRandomInt(113, intensity <  3 ? 114 : intensity < 6 ? 115 : 116)].realPath, _, _, intensity < 2 ? 35 : intensity < 4 ? 42 : intensity < 6 ? 45 : 50, true, 1, 0.125, 100);
  return Plugin_Stop;
}

public Action ResetTornado(Handle timer) {
  WeatherManager.setCanTornado(true);
  WeatherManager.setTornadoTimerActive(false);
  return Plugin_Stop;
}

public Action RestorePower(Handle timer, int surge) {
  g_PowerOutage = false;
  WeatherManager.setPowerSurging((surge == 1) ? true : false);
  if (surge == 1) { CreateTimer(1.0, SurgePower); }
  for (int i = 0; i < sizeof(MapLighting); i++) MapLighting[i].RestorePower();
  Photon.Fire("MapLighting.Indoor*", "TurnOn");
  Photon.Fire("MapLighting.Indoor*", "LightOn");
  return Plugin_Stop;
}

char LightingPatterns[][24] =  {
  /////Allistor/////
  "sduehrjkihwerte",
  "ihqopeiruhiqwer",
  "sadnpiudghsfiod",
  "kjahbfihkabweoi",
  "djfohoaeiufgawt",
  "ewrtyvghbvfczfr",
  "aesrergtafdcgvz",
  "aeradyjdghnyjxc",
  "oihaecpnefijanle",
  "oaihbewrpoijnae",
  //////Jeffy///////
  "gewsaadgfhgtfsr",
  "kuyijyterytdfsadfvgzs",
  "bvcxfgtertyaetr",
  "gyukkjtyasde",
  "bxcvmvbnhkgfj",
  "ewrhrtmhgjf",
  "bfgmghntjy",
  "afsdgdsfayrwte",
  "hfgjgfdiyet",
  "fsdbvxfgkytestry"
};

public Action SurgePower(Handle timer) {
  if (WeatherManager.isPowerSurging()) {
    int target = GetRandomInt(0, 12);
    if (MapLighting[target].isBroken) target = GetRandomInt(0, 12);
    if (!MapLighting[target].isBroken) {
      if (GetRandomInt(0, 9) >= 7) { MapLighting[target].Explode(); SetupAttackZone(9000.0, DMG_SHOCK, g_Lights[target], false, 20, 0.1, 9999999.0); }
      Photon.Fire(MapLighting[target].light, "SetPattern", LightingPatterns[GetRandomInt(0, 19)]);
      Photon.Fire(MapLighting[target].light, "TurnOn");
      Photon.Fire(MapLighting[target].beam, "LightOn");
      CreateTimer(GetRandomFloat(7.5, 15.0), RestorePower, 0);
      CreateTimer(GetRandomFloat(0.1, 3.0), SurgePower);
      CreateTimer(GetRandomFloat(1.5, 2.0), SurgePowerFX);
    }
  }
  return Plugin_Stop;
}

public Action SurgePowerFX(Handle timer) {
  for (int i = 0; i < sizeof(MapLighting); i++) {
    if (WeatherManager.isPowerSurging() && !MapLighting[i].isBroken) {
      Photon.Fire(MapLighting[i].light, "SetPattern", LightingPatterns[GetRandomInt(0, 19)]);
      Photon.Fire(MapLighting[i].beam, "LightOff");
      Photon.Fire(MapLighting[i].beam, "LightOn", "", GetRandomFloat(0.1, 0.35));
      Photon.Fire(MapLighting[i].beam, "LightOff", "", GetRandomFloat(0.4, 0.75));
      Photon.Fire(MapLighting[i].beam, "LightOn", "", GetRandomFloat(1.0, 1.35));
    }
  }
  return Plugin_Stop;
}

public Action TornadoShaker(Handle timer) {
  if (WeatherManager.hasTornado()) {
    Photon.Fire("tornadoshake_f1", "StartShake");
    CreateTimer(1.5, TornadoShaker);
  }
  return Plugin_Stop;
}

public Action Command_SetVolume(int client, int args) {
  char arg1[4];
  GetCmdArg(1, arg1, sizeof(arg1));
  int steamID = GetSteamAccountID(client);
  if (!Ass_Database || !steamID || steamID <= 10000) return Plugin_Stop;
  //char query[256];
  //Format(query, sizeof(query), "SELECT * from ass_activity WHERE steamid = %i;", steamID);
  //Ass_Database.Query(SetVolume, query, client);
  AudioManager.SetVolume(client, StringToInt(arg1));
  return Plugin_Continue;
}

public Action Command_SetVolumeSFX(int client, int args) {
  char arg1[4];
  GetCmdArg(1, arg1, sizeof(arg1));
  int steamID = GetSteamAccountID(client);
  if (!Ass_Database || !steamID || steamID <= 10000) return Plugin_Stop;
  //char query[256];
  //Format(query, sizeof(query), "SELECT * from ass_activity WHERE steamid = %i;", steamID);
  //Ass_Database.Query(SetVolume, query, client);
  AudioManager.SetVolumeSFX(client, StringToInt(arg1));
  return Plugin_Continue;
}

public Action Command_SetVolumeReal(int client, int args) {
  char arg1[8];
  char arg2[4];
  GetCmdArg(1, arg1, sizeof(arg1));
  GetCmdArg(2, arg2, sizeof(arg2));
  PrintToChat(client, "%s, %s", arg1, arg2);
  if (StringToInt(arg2) > 100 | StringToInt(arg2) < 0) { ReplyToCommand(client, "ERROR: Value must be between 0 and 100."); return Plugin_Handled; }
  if (StrContains(arg1, "music", false) > -1) { AudioManager.SetVolumeReal(client, (StringToInt(arg2) * 1.0) / 100.0); return Plugin_Handled; }
  if (StrContains(arg1, "sfx", false)> -1) { AudioManager.SetVolumeRealSFX(client, (StringToInt(arg2) * 1.0) / 100.0); return Plugin_Handled; }
  ReplyToCommand(client, "Usage: sm_setvolumereal <sfx / music> <int 0-100>"); return Plugin_Handled;
}

// Debug Commands
public Action Command_GetCurrentSong(int client, int args) {
  char buffer[16], tbuffer[16];
  int  sPos = RoundToFloor(AudioManager.engineSecondsAdjusted(client) - AudioState[client].timeSeconds);
  int  tPos = RoundToFloor(AudioState[client].loopSeconds);
  Format(buffer, 16, "%02d:%02d", sPos / 60, sPos % 60);
  Format(tbuffer, 16, "%02d:%02d", tPos / 60, tPos % 60);
  CPrintToChat(client, "The current song (%i) is: {limegreen}%s {orange}(%s / %s)", AudioState[client].indexBGM, AudioState[client].songName, buffer, tbuffer);
  return Plugin_Handled;
}

public Action Command_SetFogDensity(int client, int args) {
  char arg1[5]; GetCmdArg(1, arg1, sizeof(arg1));
  WeatherManager.setFogTarget(StringToFloat(arg1));
  PrintToChatAll("Changing fog density to %s", arg1);
  return Plugin_Handled;
}

public Action Command_SetFogSEU(int client, int args) {
  char arg01[5], arg02[5], arg03[5];
  GetCmdArg(1, arg01, sizeof(arg01)); GetCmdArg(2, arg02, sizeof(arg02)); GetCmdArg(3, arg03, sizeof(arg03));
  WeatherManager.SetFogStartQueued(StringToInt(arg01));
  WeatherManager.SetFogEndQueued(StringToInt(arg02));
  if (StringToInt(arg03) == 1) WeatherManager.StartFogTransition();
  PrintToChatAll("Received fog start: %s, fog end: %s, begin transition: %s", arg01, arg02, arg03);
  return Plugin_Handled;
}