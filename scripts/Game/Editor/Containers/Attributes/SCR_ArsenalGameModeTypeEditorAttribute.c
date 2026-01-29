[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalGameModeTypeEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!IsGameMode(item))
			return null;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return null;
		
		//~ No entries in the UI data for Arsenal Game mode types
		array<SCR_ArsenalGameModeUIData> arsenalGameModeTypeUIInfoList = {};
		if (arsenalManager.GetArsenalGameModeUIInfoList(arsenalGameModeTypeUIInfoList) <= 0)
			return null;

		return SCR_BaseEditorAttributeVar.CreateInt(arsenalManager.GetArsenalGameModeType());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		if (!IsGameMode(item))
			return;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;
		
		arsenalManager.SetArsenalGameModeType_S(var.GetInt(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return 0;
		
		array<SCR_ArsenalGameModeUIData> arsenalGameModeTypeUIInfoList = {};
		arsenalManager.GetArsenalGameModeUIInfoList(arsenalGameModeTypeUIInfoList);
		
		m_aValues.Clear();	
		SCR_EditorAttributeFloatStringValueHolder value;
		
		foreach (SCR_ArsenalGameModeUIData data : arsenalGameModeTypeUIInfoList)
		{
			value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetWithUIInfo(data.m_UIInfo, data.m_eArsenalGameModeType);
			m_aValues.Insert(value);
		}
		
		return super.GetEntries(outEntries);
	}
};
