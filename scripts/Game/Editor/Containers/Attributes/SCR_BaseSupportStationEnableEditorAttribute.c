[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseSupportStationEnableEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	//~ Overwrite this
	protected SCR_ESupportStationResupplyType GetSupportStationType()
	{
		Print("SCR_BaseSupportStationEnableEditorAttribute should never be used. Inherit instead!", LogLevel.ERROR);
		return SCR_ESupportStationResupplyType.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{					
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || !editableEntity.GetOwner())
			return null;

		SCR_BaseSupportStationComponent supportStation = SCR_BaseSupportStationComponent.FindSupportStation(editableEntity.GetOwner(), GetSupportStationType());
		if (!supportStation)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(supportStation.IsEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{						
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || !editableEntity.GetOwner())
			return;
		
		SCR_BaseSupportStationComponent supportStation = SCR_BaseSupportStationComponent.FindSupportStation(editableEntity.GetOwner(), GetSupportStationType());
		if (!supportStation)
			return;
		
		supportStation.SetEnabled(var.GetBool());
	}
};