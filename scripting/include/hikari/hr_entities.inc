// Hikari's Entity Hooks / Outputs v1.2.0 

// Hook all spawned entities, automatically unhooking them when they poof.
public void OnEntityCreated(int ent, const char[] classname) { SDKHookEx(ent, SDKHook_SpawnPost, OnEntitySpawned); }

// A wild CBaseEntity appeared!
public void OnEntitySpawned(int ent) {
  char classname[64]; GetEntityClassname(ent, classname, sizeof(classname)); // Who's that Entity???
  if (strcmp(classname, "func_physbox") == 0) { HookSingleEntityOutput(ent, "OnDamaged", OnEntityDamaged, false); } // func_physbox damaged
  if (strcmp(classname, "path_track") == 0) { HookSingleEntityOutput(ent, "OnPass", OnPassTrack, false); } // path_track passed
}

// Called when entities are damaged
public void OnEntityDamaged(const char[] output, int caller, int activator, float delay) {
  // Falling Star did a thing
  if (Photon.MatchEntName(caller, "FallingStar")) {
    CustomSoundEmitterEX(SFXArray[3].realPath, _, _, 65, false, 0, 1.0, 100);
    Photon.Fire("FallingStar", "Break");
    Photon.Fire("FallingStarSprite", "Kill");
    Photon.Fire("FrontMapShake", "Amplitude", "16");
    Photon.Fire("FrontMapShake", "Frequency", "160");
    Photon.Fire("StarExplosion", "Explode");
    Photon.Fire("FrontMapShake", "StartShake");
    Photon.Fire("StarExplosion", "Kill", _, 0.1);
  }
  // Mini Meteor made landfall!
  if (Photon.MatchEntName(caller, "FB.MiniMeteor")) OnMeteoriteImpact();
  // Goobbue or Blue Kirby slammed into the planet!
  if (Photon.MatchEntName(caller, "GiantGoobPhys") || Photon.MatchEntName(caller, "BlueBall")) {
    CustomSoundEmitterEX(SFXArray[35].realPath, _, _, SFXArray[35].SNDLVL, false, 0, 1.0, 100);
    Photon.Fire("FrontMapShake", "Amplitude", "16");
    Photon.Fire("FrontMapShake", "Frequency", "100");
    Photon.Fire("FrontMapShake", "StartShake");
    Photon.Fire("GiantGoob*", "Kill");
    SetupAttackZone(777777.777, (isGoobbue ? DMG_ACID : DMG_PARALYZE), g_FrontRottenburg, _, GetRandomInt(35, 75), 0.1, 2.1);
  }
}

// Called when a path_track has been passed
public void OnPassTrack(const char[] output, int caller, int activator, float delay) {
  // Meteorite made landfall!
  if (Photon.MatchEntName(caller, "MeteoritePath02")) OnMeteoriteImpact();
}

// The dinosaurs are extinct! (again)
public void OnMeteoriteImpact() {
  Photon.Fire("ShakeGlobal", "StartShake"); Photon.Fire("ShakeGlobal", "StopShake", _, 0.5);
  CustomSoundEmitterEX(SFXArray[124].realPath, _, _, SFXArray[124].SNDLVL, false, 0, 1.0, 100);
  Photon.Fire("FB.Meteorite00", "Kill"); Photon.Fire("FB.MiniMeteor", "Kill");
  Photon.Fire("MeteoritePath*", "Kill");
  Photon.Fire("FB.MeteoriteTrain", "Kill");
  SetupAttackZone(50000.0, DMG_CRUSH, g_FrontRottenburg, _, 3, _, 0.5);
}

Zone g_SlotMachine;
methodmap SLOTMACHINE {
  public void Init() { SetZoneExtents(g_SlotMachine, -3450.0, -56.0, -206.0, -3465.0, -120.0, -280.0); }
  public void Tick() {
    if (g_SlotMachine.canTick()) {
      for (int i = 1; i <= MaxClients; i++) {
        if (IsClientInvalidForTrigger(i)) continue;
        float dest[3]; GetClientAbsOrigin(i, dest);
        if (g_SlotMachine.isTriggeredByEntityAt(dest)) { this.OnStart(); }
      }
    }
  }
  // Someone's gambling!
  public void OnStart() {
    CustomSoundEmitterEX(SFXArray[125].realPath, Photon.Find("SlotMachine.Display02", "func_brush"), _, 75, false, 0, 1.0, 100, {-3418.0, -89.0, -220.0});
  }
}
SLOTMACHINE SlotMachine;