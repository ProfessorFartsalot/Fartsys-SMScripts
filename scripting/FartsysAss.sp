/*             WELCOME TO FARTSY'S ASS ROTTENBURG.
 *   A FEW THINGS TO KNOW: ONE.... THIS IS INTENDED TO BE USED WITH UBERUPGRADES.
 *   TWO..... SERVER OPERATORS MAY UPLOAD MUSIC TO BE USED WITH THIS PLUGIN. WE ARE NOT RESPONSIBLE FOR WHAT USERS UPLOAD TO THEIR SERVERS.
 *   THREE..... THIS MOD IS INTENDED FOR USE ON THE HYDROGENHOSTING SERVERS ONLY.
 *   FOUR..... THE DURATION OF MUSIC TIMERS SHOULD BE SET DEPENDING WHAT SONG IS USED. SET THIS USING THE CONFIG FILES. SONG DUR IN SECONDS / 0.0151515151515 = REFIRE TIME.
 *   FIVE..... TIPS AND TRICKS MAY BE ADDED TO THE TIMER, SEE PerformAdverts(Handle timer);
 *        IF IT'S WAR THAT YOU WANT, THEN I'M READY TO PLAY. GLHF!
 */

public char PLUGIN_VERSION[8] = "9.4.5";
#include <sourcemod>
#include <sdktools>
#include <tf2_stocks>
#include <fartsy/tf2_damagebits>
#include <fartsy/newcolors>
#include <fartsy/fastfire2>
#include <fartsy/ass_discord>
#include <fartsy/ass_database>
#include <fartsy/ass_serverutils>
#include <fartsy/ass_enhancer>
#include <fartsy/ass_asshop>
#include <fartsy/ass_bombstate>
#include <fartsy/ass_bosshandler>
#include <fartsy/ass_emergency>
#include <fartsy/ass_configsystem>
#include <fartsy/ass_helper>
#include <fartsy/ass_commands>
#include <fartsy/ass_events>
#include <fartsy/ass_sudo>
#include <fartsy/ass_wavesystem>
#include <tf2attributes>
#pragma newdecls required
#pragma semicolon 1

public Plugin myinfo = {
  name = "Fartsy's Ass - Framework",
  author = "Fartsy",
  description = "Framework for Fartsy's Ass (MvM Mods)",
  version = PLUGIN_VERSION,
  url = "https://wiki.hydrogenhosting.org"
};

public void OnPluginStart() {
  if (GetExtensionFileStatus("smjansson.ext") != 1) { SetFailState("Required extension (smjansson) is not loaded!"); }
  AssLogger(LOGLVL_INFO, "Starting up Fartsy's Framework! Waiting for Map Start...");
}

//Begin executing IO when ready
public void OnFastFire2Ready() {
  AssLogger(LOGLVL_INFO, "####### FASTFIRE2 IS READY! INITIATE STARTUP SEQUENCE... PREPARE FOR THE END TIMES #######");
  core.init_pre();
  RegisterAndPrecacheAllFiles();
  RegisterAllCommands();
  HookAllEvents();
  WaveSystem().update();
  if (WaveSystem().IsDefault()) core.init_post();
  CPrintToChatAll("{fartsyred}Plugin Reloaded. If you do not hear music, please do !sounds and configure your preferences.");
  cvarSNDDefault = CreateConVar("sm_fartsysass_sound", "3", "Default sound for new users, 3 = Everything, 2 = Sounds Only, 1 = Music Only, 0 = Nothing");
  CreateTimer(15.0, StatsTracker);
  AudioManager.Reset(true);
  WeatherManager.Reset();
  CreateTimer(1.0, SelectAdminTimer);
  sudo(1002);
  AssLogger(LOGLVL_INFO, "####### STARTUP COMPLETE (v%s) #######", PLUGIN_VERSION);
}

//Process ticks and requests in real time
public void OnGameFrame() {
  if (WeatherManager.TornadoWarning) WeatherManager.TickSiren();
  AudioManager.TickGlobal();
  if (BossHandler.shouldTick) BossHandler.Tick();
  if (BossHandler.tickBusterNuclear) BossHandler.TickBusterNuclear();
  if (WaveSystem().IsWaveNull() && WaveSystem().IsActive()) WaveSystem().run_bodycheck();
  WeatherManager.TickFog();
}