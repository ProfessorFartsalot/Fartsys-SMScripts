// Hikari's Light Logger
char LL_VER[8] = "1.3.0";
enum {
  LOGLVL_DEBUG = 0,
  LOGLVL_INFO = 1,
  LOGLVL_WARN = 2,
  LOGLVL_ERROR = 3
};

int LL_Threshold = LOGLVL_INFO;
void LightLogger(int logLevel, const char[] logData, any...) {
  if (logLevel < LL_Threshold) return;
  char buffer[512]; VFormat(buffer, sizeof(buffer), logData, 3);
  switch (logLevel) {
    case LOGLVL_DEBUG: { LogMessage("[DEBUG]: %s", buffer); }
    case LOGLVL_INFO: { LogMessage("[INFO]: %s", buffer); }
    case LOGLVL_WARN: { LogMessage("[WARN]: %s", buffer); }
    case LOGLVL_ERROR: { LogMessage("[ERROR]: %s", buffer); }
  }
}

public void InitLightLogger(const char[] initMsg) {
  LightLogger(LOGLVL_INFO, "[LightLogger v%s] %s", LL_VER, initMsg);
  RegAdminCmd("sm_ll_level", Command_LLLevel, ADMFLAG_ROOT, "Sets the log level for Hikari's LightLogger");
}

public Action Command_LLLevel(int client, int args){
  char arg1[32];
  GetCmdArg(1, arg1, sizeof(arg1));
  int threshold = StringToInt(arg1);
  LL_Threshold = threshold < 4 && threshold > -1 ? threshold : 1;
  LightLogger(1, "[LightLogger v%s] LogLevel changed to %i", LL_VER, LL_Threshold);
  return Plugin_Handled;
}