 //Bomb states (note to self, any additions made to this array for custom things like shop purchases or boss events should be at the END of the array, prioritize actual bombs first so as not to break anything in the event we add more bombs.. and remember to adjust bombstate[x].explode(false) in the .sp file)
int g_BombIndex; // TODO: Use this for the bombstate index instead of having 9 copies of it in the array below lmao
enum struct BOMBREGISTRY {
  bool canHindenburg;
  bool canSENTStars;
  bool canSENTShark;
  bool isMoving;
  bool isNuclear;
  bool isReady;
  bool freezeBots;
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
  int stateMax;
}
BOMBREGISTRY BombRegistry[9];

methodmap BOMBCONTROLLER {
  // Returns the current bomb id
  public int getCurBombID() { return RoundToFloor(g_BombIndex / 8.0); }
  
  // Returns a copy of the current BombRegistry object
  public BOMBREGISTRY getCurBomb() { return BombRegistry[this.getCurBombID()]; }
  
  // Returns the max bomb state for the current bomb
  public int getUpperLimit() { return (this.getCurBomb().stateMax); }
  
  /* Explodes a bomb
  * @param index The index of the bomb to detonate (max sizeof(BombRegistry))
  * @param realExplosion Whether the bomb was legitimately deployed, or if it was activated by any other means (sacpoints shop, bosses, etc)
  */
  public void explode(int index, bool realExplosion) {
    BOMBREGISTRY bomb; bomb = BombRegistry[index];
    AssLogger(LOGLVL_DEBUG, "Exploding a %s - isRealExplosion: %d canHinden: %d, canStars: %d, canShark: %d, isNuclear: %d, shouldFreezeBots: %d, explEnt: %s, explShake: %s, explSnd: %s, identifier: %s, explDly: %f, explShakeDly: %f, fadeDly: %f, NukeStart: %f, sacVal: %i...", bomb.name, realExplosion, bomb.canHindenburg, bomb.canSENTStars, bomb.canSENTShark, bomb.isNuclear, bomb.freezeBots, bomb.explEnt, bomb.explShake, bomb.explSnd, bomb.identifier, bomb.explDly, bomb.explShakeDly, bomb.FadeDly, bomb.NukeStart, bomb.sacVal);
    //Freeze bots?
    if (bomb.freezeBots) {
      ServerCommand("sm_freeze @blue 10");
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] Bots frozen by Bath Salts for 10 seconds.");
    }
    //Summon star storm
    if (bomb.canSENTStars) {
      CreateTimer(1.0, SENTStarTimer);
      CreateTimer(60.0, TimedOperator, 14);
    }
    //Holy jesus, it's a nuke!
    if (bomb.isNuclear) {
      FastFire2("LargeExplosionSND", "PlaySound", "", bomb.NukeStart);
      FastFire2("NukeAll", "Enable", "", bomb.NukeStart);
      FastFire2("HurtAll", "AddOutput", "damagetype 262144", bomb.NukeStart);
      FastFire2("HurtAll", "AddOutput", "damage 2000000", bomb.NukeStart);
      FastFire2("HurtAll", "Enable", "", bomb.NukeStart + 0.1);
      FastFire2("FB.Fade", "Fade", "", bomb.FadeDly);
      FastFire2("NukeAll", "Disable", "", 3.0);
      FastFire2("HurtAll", "Disable", "", 3.0);
      if (g_BombIndex == 64) {
        CreateTimer(15.0, SpecTimer);
        this.Reset();
      }
      if (bomb.canHindenburg) {
        FastFire2("DeliveryBurg", "Unlock");
        CPrintToChatAll("The {red}HINDENBURG {forestgreen}is now ready for flight!");
      }
    }
    FastFire2(bomb.explEnt, "Explode", "", bomb.explDly);
    CustomSoundEmitter(bomb.explSnd, 65, false, 0, 1.0, 100);
    FastFire2(bomb.explShake, "StartShake", "", bomb.explShakeDly);
    if (realExplosion) {
      CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] %s {forestgreen}successfully deployed! {white}({limegreen}+%i pts{white})", bomb.name, bomb.sacVal);
      ass_sacPoints += bomb.sacVal;
      CreateTimer(3.0, BombStatusAddTimer);
      CustomSoundEmitter(SFXArray[6].realPath, 65, false, 0, 1.0, 100);
      FastFire2("RareSpells", "Enable");
    }
  }
  public void Reset() {
    g_BombIndex = 0;
    BombRegistry[0].stateMax = 10;
  }
}
BOMBCONTROLLER Bombs;

//Track g_BombIndex and update entities every 0.1 seconds
public Action BombStatusUpdater(Handle timer) {
  if (WaveSystem().IsActive()) {
    CreateTimer(0.1, BombStatusUpdater);
    if (g_BombIndex < BombRegistry[0].stateMax) {
      switch (g_BombIndex) {
        case 8, 16, 24, 32, 40, 48, 56, 64: {
          int curBomb = Bombs.getCurBombID();
          BombRegistry[0].stateMax = g_BombIndex;
          BombRegistry[0].isReady = true;
          FastFire2("Bombs*", "Disable");
          FastFire2("Delivery", "Unlock");
          FastFire2(BombRegistry[curBomb].identifier, "Enable");
          CustomSoundEmitter(SFXArray[56].realPath, 65, false, 0, 1.0, 100);
          CPrintToChatAll("{darkviolet}[{forestgreen}CORE{darkviolet}] {forestgreen}Your team's %s {forestgreen}is now available for deployment!", BombRegistry[curBomb].name);
        }
      }
    } else if (g_BombIndex > BombRegistry[0].stateMax)g_BombIndex = Bombs.getUpperLimit();
  }
  return Plugin_Stop;
}

void SetupBombSystem() {
  BombRegistry[8].canHindenburg = true;
  BombRegistry[4].canSENTStars = true;
  BombRegistry[6].canSENTShark = true;
  BombRegistry[5].explDly = 1.7;
  BombRegistry[1].explEnt = "SmallExplosion";
  BombRegistry[2].explEnt = "MediumExplosion";
  BombRegistry[3].explEnt = "MediumExplosion";
  BombRegistry[4].explEnt = "MediumExplosion";
  BombRegistry[5].explEnt = "LargeExplosion";
  BombRegistry[6].explEnt = "LargeExplosion";
  BombRegistry[7].explEnt = "LargeExplosion";
  BombRegistry[8].explEnt = "LargeExplosion";
  BombRegistry[1].explShake = "SmallExploShake";
  BombRegistry[2].explShake = "MedExploShake";
  BombRegistry[3].explShake = "MedExploShake";
  BombRegistry[4].explShake = "MedExploShake";
  BombRegistry[5].explShake = "LargeExploShake";
  BombRegistry[6].explShake = "LargeExploShake";
  BombRegistry[7].explShake = "LargeExploShake";
  BombRegistry[8].explShake = "LargeExploShake";
  BombRegistry[5].explShakeDly = 1.7;
  BombRegistry[1].explSnd = SFXArray[1].realPath;
  BombRegistry[2].explSnd = SFXArray[2].realPath;
  BombRegistry[3].explSnd = SFXArray[2].realPath;
  BombRegistry[4].explSnd = SFXArray[2].realPath;
  BombRegistry[5].explSnd = SFXArray[4].realPath;
  BombRegistry[6].explSnd = SFXArray[3].realPath;
  BombRegistry[7].explSnd = SFXArray[35].realPath;
  BombRegistry[8].explSnd = SFXArray[35].realPath;
  BombRegistry[1].identifier = "Bombs.FreedomBomb";
  BombRegistry[2].identifier = "Bombs.ElonBust";
  BombRegistry[3].identifier = "Bombs.BathSalts";
  BombRegistry[4].identifier = "Bombs.FallingStar";
  BombRegistry[5].identifier = "Bombs.MajorKong";
  BombRegistry[6].identifier = "Bombs.SharkTorpedo";
  BombRegistry[7].identifier = "Bombs.FatMan";
  BombRegistry[8].identifier = "Bombs.Hydrogen";
  BombRegistry[0].isMoving = false;
  BombRegistry[5].isNuclear = true;
  BombRegistry[7].isNuclear = true;
  BombRegistry[8].isNuclear = true;
  BombRegistry[5].NukeStart = 1.7;
  BombRegistry[0].isReady = false;
  BombRegistry[1].name = "{red}FREEDOM BOMB{white}";
  BombRegistry[2].name = "{red}ELON BUST{white}";
  BombRegistry[3].name = "{orange}BATH SALTS{white}";
  BombRegistry[4].name = "{gold}FALLING STAR{white}";
  BombRegistry[5].name = "{red}MAJOR KONG{white}";
  BombRegistry[6].name = "{aqua}SHARK{white}";
  BombRegistry[7].name = "{orange}FAT MAN{white}";
  BombRegistry[8].name = "{red}HYDROGEN{white}";
  BombRegistry[1].sacVal = 5;
  BombRegistry[2].sacVal = 5;
  BombRegistry[3].sacVal = 5;
  BombRegistry[4].sacVal = 10;
  BombRegistry[5].sacVal = 15;
  BombRegistry[6].sacVal = 20;
  BombRegistry[7].sacVal = 25;
  BombRegistry[8].sacVal = 50;
  BombRegistry[3].freezeBots = true;
  g_BombIndex = 0;
  BombRegistry[0].stateMax = 0;
}