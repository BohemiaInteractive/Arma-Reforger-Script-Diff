[EntityEditorProps(category: "GameScripted/DataCollection", description: "Data Collector client cheat menu")]
class SCR_DataCollectorDiagComponentClass : ScriptComponentClass
{
}

class SCR_DataCollectorDiagComponent : ScriptComponent
{
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_DATA_COLLECTION, "Data Collection", "Statistics");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_DATA_COLLECTION_ENABLE_DIAG, "", "Enable debug menu", "Data Collection");
	}
#endif
}
