//Fartsy's FastFire2
int ff2Ref;

//Inject entity IO logic
public void OnMapStart() {
  AssLogger(LOGLVL_INFO, "OnMapStart Received: Injecting FastFire2!");
  int ff2 = FindEntityByTargetname("FastFire2", "info_target");
  if (!IsValidEntity(ff2)) {
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
* @param target The target entity - if it contains OnUser1, this will become the entire io string. Add `,1` for refire count. Example: `FastFire2("OnUser1 Operator,Command,sm_example,0.69,1");`
* @param input The input to execute
* @param param Parameters if applicable
* @param delay The delay before executing
*/
void FastFire2(const char[] target, const char[] input = "", const char[] param = "", float delay = 0.0) {
  int ff2 = EntRefToEntIndex(ff2Ref);
  char FireStr[128];
  FormatEx(FireStr, sizeof(FireStr), (StrContains(target, "OnUser1", true) != -1 ? target : "OnUser1 %s,%s,%s,%f,1"), target, input, param, delay);
  SetVariantString(FireStr);
  AcceptEntityInput(ff2, "AddOutput");
  AcceptEntityInput(ff2, "FireUser1");
}

//Used for any time we need to fo a LOT of things in quick succession... Or when we want to do something at random.
enum struct BULKFIRE {
  char fireStr[256];
}