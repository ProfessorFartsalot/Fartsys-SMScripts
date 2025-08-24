enum struct HOOKS {
  int ent;
  char output[64];
  EntityOutput callback;
}
HOOKS g_HookedEntities[128];
int g_HookedCount = 0;

// Hooks an entity by its index
void HookSingleEntity(int ent, const char[] output, EntityOutput callback)
{
    if (!IsValidEntity(ent) || g_HookedCount >= sizeof(g_HookedEntities)) return;
    for (int i = 0; i < g_HookedCount; i++) if (g_HookedEntities[i].ent == ent && StrEqual(g_HookedEntities[i].output, output)) return; // Avoid duplicates (same ent + output)
    HookSingleEntityOutput(ent, output, callback);
    g_HookedEntities[g_HookedCount].ent = ent;
    strcopy(g_HookedEntities[g_HookedCount].output, sizeof(g_HookedEntities[g_HookedCount].output), output);
    g_HookedEntities[g_HookedCount].callback = callback;
    PrintToServer("Hooking entity %i with output %s", ent, g_HookedEntities[g_HookedCount].output);
    g_HookedCount++;
}

public void OnEntityDestroyed(int ent){
    for (int i = g_HookedCount - 1; i >= 0; i--)
    {
        if (g_HookedEntities[i].ent == ent)
        {
            UnhookSingleEntityOutput(ent, g_HookedEntities[i].output, g_HookedEntities[i].callback);

            // Remove entry by shifting left
            for (int j = i; j < g_HookedCount - 1; j++)
            {
                g_HookedEntities[j] = g_HookedEntities[j + 1];
            }
            g_HookedCount--;
        }
    }
}
// Hook necessary entities for this plugin's operation
void HookAllEntities() {
    //PrintToServer("HOOKING ALL THE ENTITIES.");
    //HookSingleEntity(FindEntityByTargetname("Operator", "point_servercommand"), "Command", ass_relay_user1);
}

public void ass_relay_user1(const char[] output, int caller, int activator, float delay){
    //PrintToServer("Got output %s", output);
}
