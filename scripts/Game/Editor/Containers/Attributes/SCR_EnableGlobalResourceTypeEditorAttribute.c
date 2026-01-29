[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableGlobalResourceTypeEditorAttribute : SCR_BaseEditorAttribute
{
	[Attribute(EResourceType.SUPPLIES.ToString(), desc: "Which resource is set enabled/disabled globally", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceToSetEnabled;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(item);
		if (!gameMode)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(gameMode.IsResourceTypeEnabled(m_eResourceToSetEnabled));
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		SCR_BaseEditorAttributeVar respawnEnabledVar;
		manager.GetAttributeVariable(SCR_RespawnEnabledEditorAttribute, respawnEnabledVar);
		bool respawnEnabled = true;
		
		if (respawnEnabledVar)
			respawnEnabled = respawnEnabledVar.GetBool();
		
		manager.SetAttributeEnabled(SCR_LoadoutSupplyCostMultiplierEditorAttribute, var && var.GetBool() && respawnEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(item);
		if (!gameMode)
			return;
		
		gameMode.SetResourceTypeEnabled(var.GetBool(), m_eResourceToSetEnabled, playerID);
	}
}
