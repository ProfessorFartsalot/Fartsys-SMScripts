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
bool bSlotMachineRolling;
bool hasResult;
float flSlotMachineNext;
int iSlots[3];
int iSlotMachineEnt;
methodmap SLOTMACHINE {
  public void Init() {
    SetZoneExtents(g_SlotMachine, -3420.0, -56.0, -320.0, -3495.0, -120.0, -140.0);
    g_SlotMachine.tickrate = 12;
    g_SlotMachine.onlyPlayers = true;
    SetZoneActive(g_SlotMachine, true);
    iSlotMachineEnt = EntIndexToEntRef(Photon.Find("SlotMachine.Display02", "func_brush"));
    Photon.Fire("SlotMachine.Display0*", "Enable", _, 1.0);
  }
  public void Tick() {
    if (g_SlotMachine.canTick()) {
      float f = GetEngineTime();
      // Check if someone's gambling
      if (f > flSlotMachineNext) {
        for (int i = 1; i <= MaxClients; i++) {
          if (IsClientInvalidForTrigger(i)) continue;
          float dest[3]; GetClientAbsOrigin(i, dest);
          if (g_SlotMachine.isTriggeredByEntityAt(dest)) {
            Photon.Fire("SlotMachine", "Open");
            flSlotMachineNext = GetEngineTime() + 7.77;
            CustomSoundEmitterEX(SFXArray[125].realPath, EntRefToEntIndex(iSlotMachineEnt), _, 75, false, 0, 1.0, 100, {-3418.0, -89.0, -220.0});
          }
        }
      }
      if (f < flSlotMachineNext) {
        bool init = (f > (flSlotMachineNext - 6.9) && f < (flSlotMachineNext - 6.0));
        if (init) { bSlotMachineRolling = true; }
        // Roll
        if (bSlotMachineRolling) {
          for (int i = 0; i < sizeof(iSlots); i++) { int newInt = GetRandomInt(0,4); iSlots[i] = (newInt != iSlots[i]) ? (newInt) : GetRandomInt(0,4); if (GetRandomInt(0,1) == 0) iSlots[i] = 4; }
          Photon.Fire("SlotMachineDisp01", "SetTextureIndex", "%i", _, _, iSlots[0]);
          Photon.Fire("SlotMachineDisp02", "SetTextureIndex", "%i", init ? 0.0 : 0.5, _, iSlots[1]);
          Photon.Fire("SlotMachineDisp03", "SetTextureIndex", "%i", init ? 0.0 : 1.0, _, iSlots[2]);
          // Stop rolling and get a result
          if (f > (flSlotMachineNext - 4.1)) {
            PrintToChatAll("Not rolling anymore!");
            bSlotMachineRolling = false;
            for (int i = 2; i < 4; i++) { Photon.Fire("SlotMachine.Display0%i", "Disable", _, 0.07, _, i); Photon.Fire("SlotMachine.Display0%i", "Enable", _, i == 2 ? 0.5 : 1.0, _, i); }
            if (iSlots[0] == iSlots[1] && iSlots[1] == iSlots[2]) { hasResult = true; }
          }
        }
        if (hasResult && f > flSlotMachineNext - 3.1) {
          hasResult = false;
          PrintToServer("Got a result: %i", iSlots[0]);
          bool boomTime = (iSlots[0] == 4);
          switch(iSlots[0]) {
            case 4: {
              Photon.Fire("BoomTimeSpawner", "ForceSpawn");
              Photon.Fire("GrenadeSND", "PlaySound", _, 0.05);
              Photon.Fire("BoomTimeBoom", "Explode", _, 1.5);
              Photon.Fire("GrenadeBoomSND", "PlaySound", _, 1.5);
              Photon.Fire("BoomTimeGrenadePhys", "Break", _, 1.5);
            }
          }
          CustomSoundEmitterEX(SFXArray[boomTime ? 126 : 127].realPath, EntRefToEntIndex(iSlotMachineEnt), _, 75, false, 0, 1.0, 100, {-3418.0, -89.0, -220.0});
        }
      }
    }
  }
}
SLOTMACHINE SlotMachine;