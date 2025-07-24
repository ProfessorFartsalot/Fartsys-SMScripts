#include <sourcemod>
public Plugin myinfo = {
  name = "Fartsy's Random Bullshit Go",
  author = "Fartsy",
  description = "Fuck you Brawler",
  version = "1.0.0",
  url = "https://wiki.hydrogenhosting.org"
};

public void OnPluginStart(){
    RegServerCmd("sm_fuckyoubrawler", Command_RandomThing, "yes");
}

public Action Command_RandomThing(int args){
    if (GetRandomInt(0,1) == 1) ServerCommand("sm_forcertd @all");
    return Plugin_Handled;
}