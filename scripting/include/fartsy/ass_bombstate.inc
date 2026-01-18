//Bomb states (note to self, any additions made to this array for custom things like shop purchases or boss events should be at the END of the array, prioritize actual bombs first so as not to break anything in the event we add more bombs.. and remember to adjust bombstate[x].explode(false) in the .sp file)
enum struct BOMBSTATE {
  bool canHindenburg;
  bool canSENTStars;
  bool canSENTShark;
  bool isMoving;
  bool isNuclear;
  bool isReady;
  bool shouldFreezeBots;
  char explEnt[64];
  char explShake[64];
  char explSnd[64];
  char identifier[64];
  char name[64];
  float explDly;
  float explShakeDly;
  float FadeDly;
  float NukeStart;
  int sacVal;
  int state;
  int stateMax;
  int getCurBomb() {
    return RoundToFloor(this.state / 8.0);
  }
  int upperLimit() {
    return (this.state - 4);
  }
  void explode(bool realExplosion){
    AssLogger(LOGLVL_DEBUG, "Exploding a %s - isRealExplosion: %d canHinden: %d, canStars: %d, canShark: %d, isNuclear: %d, shouldFreezeBots: %d, explEnt: %s, explShake: %s, explSnd: %s, identifier: %s, explDly: %f, explShakeDly: %f, fadeDly: %f, NukeStart: %f, sacVal: %i...", this.name, realExplosion, this.canHindenburg, this.canSENTStars, this.canSENTShark, this.isNuclear, this.shouldFreezeBots, this.explEnt, this.explShake, this.explSnd, this.identifier, this.explDly, this.explShakeDly, this.FadeDly, this.NukeStart, this.sacVal);
    //Freeze bots?
    if(this.shouldFreezeBots){
      ServerCommand("sm_freeze @blue 10");
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] Bots frozen by Bath Salts for 10 seconds.");
    }
    //Summon star storm
    if(this.canSENTStars){
      CreateTimer(1.0, SENTStarTimer);
      CreateTimer(60.0, TimedOperator, 14);
    }
    //Holy jesus, it's a nuke!
    if(this.isNuclear){
      FastFire2("LargeExplosionSND", "PlaySound", "", this.NukeStart, false);
      FastFire2("NukeAll", "Enable", "", this.NukeStart, false);
      FastFire2("HurtAll", "AddOutput", "damagetype 262144", this.NukeStart, false);
      FastFire2("HurtAll", "AddOutput", "damage 2000000", this.NukeStart, false);
      FastFire2("HurtAll", "Enable", "", this.NukeStart+0.1, false);
      FastFire2("FB.Fade", "Fade", "", this.FadeDly, false);
      FastFire2("NukeAll", "Disable", "", 3.0, false);
      FastFire2("HurtAll", "Disable", "", 3.0, false);
      if(this.state == 64){
        CreateTimer(15.0, SpecTimer);
        ResetBombState();
      }
      if(this.canHindenburg){
        FastFire2("DeliveryBurg", "Unlock", "", 0.0, false);
        CPrintToChatAll("The {red}HINDENBURG {forestgreen}is now ready for flight!");
      }
    }
    FastFire2(this.explEnt, "Explode", "", this.explDly, false);
    CustomSoundEmitter(this.explSnd, 65, false, 0, 1.0, 100);
    FastFire2(this.explShake, "StartShake", "", this.explShakeDly, false);
    if(realExplosion){
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] %s {forestgreen}successfully deployed! {white}({limegreen}+%i pts{white})", this.name, this.sacVal);
      ass_sacPoints += this.sacVal;
      CreateTimer(3.0, BombStatusAddTimer);
      CustomSoundEmitter(SFXArray[6].realPath, 65, false, 0, 1.0, 100);
      FastFire2("RareSpells", "Enable", "", 0.0, false);
    }
  }
}
BOMBSTATE bombState[9];
void ResetBombState(){
  bombState[0].state = 0;
  bombState[0].stateMax = 10;
}