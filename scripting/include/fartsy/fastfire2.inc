//Fartsy's FastFire2
int ff2Ref;

//Inject entity IO logic
public void OnMapStart() {
  InjectFastFire2();
}

void InjectFastFire2(){
  int ff2 = FindEntityByTargetname("FastFire2", "info_target");
  AssLogger(LOGLVL_INFO, "OnMapStart Received: Injecting FastFire2!");
  if (!IsValidEntity(ff2)){
    ff2 = CreateEntityByName("info_target");
    if (!IsValidEdict(ff2)) SetFailState("Could not inject FastFire2, aborting!!!");
    DispatchSpawn(ff2);
    DispatchKeyValue(ff2, "targetname", "FastFire2");
    ActivateEntity(ff2);
    AssLogger(LOGLVL_INFO, "Injected info_target FastFire2 to map!")
  }
  ff2Ref = EntIndexToEntRef(ff2);
  OnFastFire2Ready();
}

/* Fartsy's ent_fire interface
* @param target The target entity
* @param input The input to execute
* @param param Parameters if applicable
* @param delay The delay before executing
* @param isLegacy Whether this is legacy or not. isLegacy will use the target param as the entire fire string. Example: `FastFire2("OnUser1 Operator,Command,sm_example,0.69,1", "", "", 0.0, true);`
* OnUser1 is mandatory before the fire string when using legacy mode. Legacy mode also requries `,1` after the string so it only fires once instead of per function call.
*/
public void FastFire2(const char[] target, const char[] input, const char[] param, const float delay, bool isLegacy) {
  int ff2 = EntRefToEntIndex(ff2Ref);
  char FireStr[128];
  FormatEx(FireStr, sizeof(FireStr), (isLegacy ? target : "OnUser1 %s,%s,%s,%f,1"), target, input, param, delay);
  SetVariantString(FireStr);
  AcceptEntityInput(ff2, "AddOutput");
  AcceptEntityInput(ff2, "FireUser1");
}