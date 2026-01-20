Database Ass_Database;
Database Ass_Database_ServerMetrics;
//Connect to database
public void OnConfigsExecuted() {
  if (!Ass_Database) Database.Connect(Database_OnConnect, "ass");
  if (!Ass_Database_ServerMetrics) Database.Connect(Database_OnMetricsConnect, "ass_servermetrics");
}

// Database Fastquery Manager
public void Database_FastQuery(Database db, DBResultSet results,
  const char[] error, any data) {
  if (!results) LogError("Failed to query database: %s", error);
}

// Database Error Handler
public void Database_MergeDataError(Database db, any data, int numQueries,
  const char[] error, int failIndex, any[] queryData) {
  LogError("Failed to query database (transaction): %s", error);
}

//Format database if needed
public void Database_OnConnect(Database db, char[] error, any data) {
  if (!db) {
    AssLogger(3, error);
    return;
  }
  char buffer[64];
  db.Driver.GetIdentifier(buffer, sizeof(buffer));
  if (!StrEqual(buffer, "mysql", false)) {
    delete db;
    AssLogger(3, "Could not connect to the database: expected mysql database.");
    return;
  }
  Ass_Database = db;
  Ass_Database.Query(Database_FastQuery, "CREATE TABLE IF NOT EXISTS ass_activity(name TEXT, steamid INT UNSIGNED, date DATE, seconds INT UNSIGNED DEFAULT '0', class TEXT DEFAULT 'na', health TEXT DEFAULT '-1', maxHealth INT UNSIGNED DEFAULT '0', damagedealt INT UNSIGNED DEFAULT '0', damagedealtsession INT UNSIGNED DEFAULT '0', kills INT UNSIGNED DEFAULT '0', killssession INT UNSIGNED DEFAULT '0', deaths INT UNSIGNED DEFAULT '0', deathssession INT UNSIGNED DEFAULT '0', bombsreset INT UNSIGNED DEFAULT '0', bombsresetsession INT UNSIGNED DEFAULT '0', sacrifices INT UNSIGNED DEFAULT '0', sacrificessession INT UNSIGNED DEFAULT '0', lastkilledname TEXT DEFAULT 'na', lastweaponused TEXT DEFAULT 'na', killedbyname TEXT DEFAULT 'na', killedbyweapon TEXT DEFAULT 'na', soundprefs INT UNSIGNED DEFAULT '3', PRIMARY KEY (steamid));");
  if (GetClientCount(true) > 0){
    for (int i = 0; i <= MaxClients; i++){
      if (!IsValidClient(i)) continue;
      char query[256];
      Format(query, sizeof(query), "SELECT soundprefs FROM ass_activity WHERE steamid = '%i';", GetSteamAccountID(i));
      Ass_Database.Query(SQL_SNDPrefs, query, i);
    }
  }
}

//Connect to metrics db
public void Database_OnMetricsConnect(Database db, char[] error, any data) {
  if (!db) {
    AssLogger(3, error);
    return;
  }
  char buffer[64];
  db.Driver.GetIdentifier(buffer, sizeof(buffer));
  if (!StrEqual(buffer, "mysql", false)) {
    delete db;
    AssLogger(3, "Could not connect to the database: expected mysql database.");
    return;
  }
  Ass_Database_ServerMetrics = db;
}