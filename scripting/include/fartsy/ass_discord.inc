// Sends events via SCR plugin
native void SCR_SendEvent(const char[] event, const char[] message, any ...);
// Checks if SCR is available
public bool SCR_IsAvailable() { return GetFeatureStatus(FeatureType_Native, "SCR_SendMessage") == FeatureStatus_Available; }