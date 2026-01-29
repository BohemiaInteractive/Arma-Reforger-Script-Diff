// Script File 
/**
Group Faction Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AIGroupFactionEditorAttribute : SCR_BaseFactionEditableAttribute
{	
	protected override bool ValidEntity(GenericEntity entity)
	{
		SCR_AIGroup aIGroup = SCR_AIGroup.Cast(entity);
		if (!aIGroup || aIGroup.GetPlayerCount() > 0 || aIGroup.IsSlave()) 
			return false;
		
		return true;
	}
	
	protected override Faction GetFaction(GenericEntity entity)
	{
		return SCR_AIGroup.Cast(entity).GetFaction();
	}
	
	protected override void SetFaction(GenericEntity entity, Faction faction)
	{
		SCR_AIGroup aIGroup = SCR_AIGroup.Cast(entity);
		if (!aIGroup || aIGroup.GetPlayerCount() > 0) 
			return;
		
		aIGroup.SetFaction(faction);
	}
};