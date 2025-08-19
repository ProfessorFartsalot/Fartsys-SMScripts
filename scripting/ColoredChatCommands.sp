#include <sourcemod>
#include <morecolors>
public Plugin myinfo = {
  name = "Colored Chat Comms",
  author = "Fartsy",
  description = "Allows the server to post colored chat",
  version = "1.0.0",
  url = "https://panel.hydrogenhosting.org"
};

public void OnPluginStart(){
    RegServerCmd("sm_clsay", Command_ClSay, "Prints a colored chat to the chat box");
}

public Action Command_ClSay(int args){
    char text[512];
    GetCmdArgString(text, sizeof(text));
    CPrintToChatAll(text);
    return Plugin_Handled;
}