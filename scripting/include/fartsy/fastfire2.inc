//Fartsy's FastFire2
char FF2_VER[7] = "1.2.0";
int ff2Ref;

/* Create Entities with optional data
* @param classname - The entity class name to be created
* @param targetname - The name of the created entity
*/
int CreateCustomEntity(const char[] classname, const char[] targetname) {
  int cne = FindEntityByClassname(-1, classname);
  if (!IsValidEntity(cne)) {
    cne = CreateEntityByName(classname);
    if (!IsValidEdict(cne)) { AssLogger(LOGLVL_ERROR, "Could not create %s, aborting!", classname); return -2; }
    DispatchSpawn(cne);
    DispatchKeyValue(cne, "targetname", targetname);
    ActivateEntity(cne);
  }
  return cne;
}

//Inject entity IO logic
public void OnMapStart() {
  AssLogger(LOGLVL_INFO, "OnMapStart Received: Injecting FastFire2! (v%s)", FF2_VER);
  int ff2 = CreateCustomEntity("info_target", "FastFire2")
  if (!IsValidEntity(ff2)) { SetFailState("Could not inject FastFire2, aborting!!!"); }
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