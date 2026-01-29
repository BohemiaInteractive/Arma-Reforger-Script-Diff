[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_LogoOverlayEditorAttribute: SCR_MenuOverlayEditorAttribute
{
	//Disable respawn time if respawning is disabled
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		//Set sub labels
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_LogoOverlayPositionEditorAttribute);
		}
			
		manager.SetAttributeEnabled(SCR_LogoOverlayPositionEditorAttribute, var && var.GetInt() != 0);
	}
};