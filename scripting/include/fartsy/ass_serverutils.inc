// ASS Utilities - Advanced server features
Database Ass_Database_ServerMetrics;
void Set_Ass_Database_ServerMetrics(Database db) {
  Ass_Database_ServerMetrics = db;
}
Database Get_Ass_Database_ServerMetrics() {
  return Ass_Database_ServerMetrics;
}
//Gets the server time as an int array, 0 = hour, 1 = minute, 2 = AM/PM(0/1 respectively)
void GetServerTime(int retVal[3]) {
    char buffer[8];
    FormatTime(buffer, sizeof(buffer), "%I", GetTime());
    int hour = StringToInt(buffer);
    FormatTime(buffer, sizeof(buffer), "%M", GetTime());
    int minute = StringToInt(buffer);
    FormatTime(buffer, sizeof(buffer), "%p", GetTime());
    int ampm = (StrEqual(buffer, "AM")) ? 0 : 1;
    retVal[0] = hour;
    retVal[1] = minute;
    retVal[2] = ampm;
}
//Gets the server stats and display it in the server
public Action StatsTracker(Handle timer) {
  int time[3];
  GetServerTime(time);
  char hour[4], minute[4], ampm[4];
  Format(hour, sizeof(hour), "%i", time[0]);
  Format(minute, sizeof(minute), "%i", time[1]);
  Format(ampm, sizeof(ampm), "%i", time[2]);
  FastFire2("FBMetric.TimeHH", "SetTextureIndex", hour, 0.0, false);
  FastFire2("FBMetric.TimeMM", "SetTextureIndex", minute, 0.0, false);
  FastFire2("FBMetric.TimeAMPM", "SetTextureIndex", ampm, 0.0, false);
  //PrintToChatAll("Current time is %s:%s %s", hour, minute, StringToInt(ampm) == 0 ? "AM" : "PM");
  CreateTimer(15.0, StatsTracker);
  if (!Get_Ass_Database()) return Plugin_Stop;
  char query[512];
  query[0] = '\0'; // Clear buffer
  StrCat(query, sizeof(query), "SELECT 'memory_used_gib' AS name, value FROM memory_used_gib WHERE id = 1 ");
  StrCat(query, sizeof(query), "UNION ALL SELECT 'memory_total_gib', value FROM memory_total_gib WHERE id = 1 ");
  StrCat(query, sizeof(query), "UNION ALL SELECT 'cpu_usage_percent', value FROM cpu_usage_percent WHERE id = 1 ");
  StrCat(query, sizeof(query), "UNION ALL SELECT 'cpu_frequency_x10', value FROM cpu_frequency_x10 WHERE id = 1 ");
  StrCat(query, sizeof(query), "UNION ALL SELECT 'cpu_temperature_c', value FROM cpu_temperature_c WHERE id = 1");
  SQL_TQuery(Ass_Database_ServerMetrics, OnMetricsFetch, query);
  return Plugin_Stop;
}

public void OnMetricsFetch(Database db, DBResultSet results, const char[] error, any data)
{
    if (results == null)
    {
        LogError("Stat fetch failed: %s", error);
        return;
    }

    while (results.FetchRow())
    {
        char name[32];
        int value;
        results.FetchString(0, name, sizeof(name));
        value = results.FetchInt(1);
        char val[32];
        if (StrEqual(name, "memory_used_gib")) {
            Format(val, sizeof(val), "%i", value);
            FastFire2("FBMetric.RAMUtil", "SetTextureIndex", val, 0.0, false);
        } else if (StrEqual(name, "memory_total_gib")) {
            Format(val, sizeof(val), "%i", value);
            FastFire2("FBMetric.RAMTotal", "SetTextureIndex", val, 0.0, false);
        } else if (StrEqual(name, "cpu_usage_percent")) {
            Format(val, sizeof(val), "%i", value);
            FastFire2("FBMetric.CPUUtil", "SetTextureIndex", val, 0.0, false);
            //PrintToServer("CPU Usage: %d%%", value);
        } else if (StrEqual(name, "cpu_frequency_x10")) {
            Format(val, sizeof(val), "%i", value);
            FastFire2("FBMetric.CPUFreq", "SetTextureIndex", val, 0.0, false);
            //PrintToServer("CPU Freq: %.1f GHz", value / 10.0);
        } else if (StrEqual(name, "cpu_temperature_c")) {
            Format(val, sizeof(val), "%i", value);
            FastFire2("FBMetric.CPUTemp", "SetTextureIndex", val, 0.0, false);
            //PrintToServer("CPU Temp: %d°C", value);
        }
    }
    FastFire2("FBMetrics.*", "Enable", "", 0.0, false);
}