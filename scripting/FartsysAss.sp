/*                         WELCOME TO FARTSY'S ASS ROTTENBURG.
 *   A FEW THINGS TO KNOW: ONE.... THIS IS INTENDED TO BE USED WITH UBERUPGRADES.
 *   TWO..... THE MUSIC USED WITH THIS MOD MAY OR MAY NOT BE COPYRIGHTED. WE HAVE NO INTENTION ON INFRINGEMENT. THIS PROJECT IS PURELY NON PROFIT AND JUST FOR FUN. SHOULD COPYRIGHT HOLDERS WISH THIS PROJECT BE TAKEN DOWN, I (Fartsy) SHALL OBLIGE WITHOUT HESITATION.
 *   THREE..... THIS MOD IS INTENDED FOR USE ON THE HYDROGENHOSTING SERVERS ONLY.
 *   FOUR..... THE DURATION OF MUSIC TIMERS SHOULD BE SET DEPENDING WHAT SONG IS USED. SET THIS USING THE CONFIG FILES. SONG DUR IN SECONDS / 0.0151515151515 = REFIRE TIME.
 *   FIVE..... TIPS AND TRICKS MAY BE ADDED TO THE TIMER, SEE PerformAdverts(Handle timer);
 *              IF IT'S WAR THAT YOU WANT, THEN I'M READY TO PLAY. GLHF!
 */

#include <sourcemod>
#include <sdktools>
#include <tf2_stocks>
#include <fartsy/newcolors>
#include <fartsy/fastfire2>
#include <fartsy/ass_enhancer>
#include <fartsy/ass_helper>
#include <fartsy/ass_bosshandler>
#include <fartsy/ass_commands>
#include <fartsy/ass_events>
#include <fartsy/ass_sudo>
#include <fartsy/ass_serverutils>
#include <fartsy/ass_wavesystem>
#pragma newdecls required
#pragma semicolon 1
char[] GET_PLUGIN_VERSION() {
  return PLUGIN_VERSION;
}

public Plugin myinfo = {
  name = "Fartsy's Ass - Framework",
  author = "Fartsy",
  description = "Framework for Fartsy's Ass (MvM Mods)",
  version = PLUGIN_VERSION,
  url = "https://wiki.hydrogenhosting.org"
};

public void OnPluginStart() {
  AssLogger(LOGLVL_INFO, "Starting up Fartsy's Framework! Waiting for Map Start...");
}

//Begin executing IO when ready
public void OnFastFire2Ready() {
  AssLogger(LOGLVL_INFO, "####### FASTFIRE2 IS READY! INITIATE STARTUP SEQUENCE... PREPARE FOR THE END TIMES #######");
  RegisterAndPrecacheAllFiles();
  RegisterAllCommands();
  HookAllEvents();
  WaveSystem().update();
  if (WaveSystem().IsDefault()) SetupCoreData();
  UpdateAllHealers();
  CreateTimer(1.0, UpdateMedicHealing);
  CPrintToChatAll("{fartsyred}Plugin Reloaded. If you do not hear music, please do !sounds and configure your preferences.");
  cvarSNDDefault = CreateConVar("sm_fartsysass_sound", "3", "Default sound for new users, 3 = Everything, 2 = Sounds Only, 1 = Music Only, 0 = Nothing");
  AssLogger(LOGLVL_INFO, "####### STARTUP COMPLETE (v%s) #######", PLUGIN_VERSION);
  CreateTimer(15.0, StatsTracker);
  GlobalAudio.Reset();
  WeatherManager.Reset();
  CreateTimer(1.0, SelectAdminTimer);
  sudo(1002);
}

//Process ticks and requests in real time
public void OnGameFrame() {
  if(WeatherManager.TornadoWarning) WeatherManager.TickSiren();
  if (GlobalAudio.shouldTick) GlobalAudio.Tick();
  if (BossHandler.shouldTick) BossHandler.Tick();
  if (BossHandler.tickBusterNuclear) BossHandler.TickBusterNuclear();
  if (WaveSystem().IsWaveNull() && WaveSystem().IsActive) WaveSystem().run_bodycheck();
  WeatherManager.TickFog();
}