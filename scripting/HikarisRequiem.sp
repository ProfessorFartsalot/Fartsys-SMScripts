/*             WELCOME TO HIKARI'S REQUIEM ROTTENBURG.
 *   A FEW THINGS TO KNOW: ONE.... THIS IS INTENDED TO BE USED WITH UBERUPGRADES, AND IS DEVELOPED BY THE HYDROGEN HOSTING.ORG TEAM.
 *   TWO..... SERVER OPERATORS MAY UPLOAD MUSIC TO BE USED WITH THIS PLUGIN. WE ARE NOT RESPONSIBLE FOR WHAT USERS UPLOAD TO THEIR SERVERS.
 *   THREE..... THE DURATION OF MUSIC TIMERS SHOULD BE SET DEPENDING WHAT SONG IS USED. SET THIS USING THE CONFIG FILES. SONG DUR IN SECONDS / 0.0151515151515 = REFIRE TIME.
 *   FOUR.... TIPS AND TRICKS MAY BE ADDED TO THE TIMER, SEE PerformAdverts(Handle timer);
*/
public char PLUGIN_VERSION[8] = "10.2.0";
#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <tf2_stocks>
#include <hikari/tf2_damagebits>
#include <hikari/newcolors>
#include <hikari/photon>
#include <hikari/lightlogger>
#include <hikari/hr_discord>
#include <hikari/hr_database>
#include <hikari/hr_serverutils>
#include <hikari/hr_triggers>
#include <hikari/hr_enhancer>
#include <hikari/hr_asshop>
#include <hikari/hr_bombstate>
#include <hikari/hr_bosshandler>
#include <hikari/hr_emergency>
#include <hikari/hr_configsystem>
#include <hikari/hr_helper>
#include <hikari/hr_commands>
#include <hikari/hr_entities>
#include <hikari/hr_events>
#include <hikari/hr_sudo>
#include <hikari/hr_wavesystem>
#include <tf2attributes>
#pragma newdecls required
#pragma semicolon 1
public Plugin myinfo = {
  name = "Hikari's MvM Framework",
  author = "Hikari",
  description = "Framework for Hikari's Requiem (MvM Mods)",
  version = PLUGIN_VERSION,
  url = "https://wiki.hydrogenhosting.org"
};
// Check if extensions are loaded, send startup log
public void OnPluginStart() {
  if (GetExtensionFileStatus("smjansson.ext") != 1) { SetFailState("Required extension (smjansson) is not loaded!"); }
  LightLogger(LOGLVL_INFO, "Starting up Hikari's Framework! Waiting for Map Start...");
}
// Begin executing IO when ready
public void OnPhotonReady() {
  LightLogger(LOGLVL_INFO, "####### FASTFIRE2 IS READY! INITIATE STARTUP SEQUENCE... PREPARE FOR THE END TIMES #######");
  core.init_pre();
  RegisterAndPrecacheAllFiles();
  RegisterMiscCommands();
  HookAllEvents();
  WaveSystem().update();
  if (WaveSystem().IsDefault()) core.init_post();
  CPrintToChatAll("{hikarired}Plugin Reloaded. If you do not hear music, please do !sounds and configure your preferences.");
  AudioManager.Reset(true);
  WeatherManager.Reset();
  LightLogger(LOGLVL_INFO, "####### STARTUP COMPLETE (v%s) #######", PLUGIN_VERSION);
}
// Process ticks and requests in real time
public void OnGameFrame() {
  AudioManager.TickGlobal();
  BossHandler.Tick();
  WaveSystem().Tick();
  WeatherManager.Tick();
  TickAllTriggers();
  SlotMachine.Tick();
}