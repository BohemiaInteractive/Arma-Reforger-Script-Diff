//! Arsenal Faction Attribute for getting and setting varriables in Editor Attribute window
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalFactionEditorAttribute : SCR_BaseFactionEditableAttribute
{	
	//------------------------------------------------------------------------------------------------
	protected override bool ValidEntity(GenericEntity entity)
	{
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(entity);
		if (!arsenalComponent)
			return false;
		
		//~ Not allowed to change faction if arsenal items are being overwritten as this would not do anything
		if (arsenalComponent.GetOverwriteArsenalConfig())
			return false;
		
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(entity.FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override Faction GetFaction(GenericEntity entity)
	{
		return SCR_FactionAffiliationComponent.Cast(entity.FindComponent(SCR_FactionAffiliationComponent)).GetAffiliatedFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void SetFaction(GenericEntity entity, Faction faction)
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(entity.FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return;
		
		factionComponent.SetAffiliatedFaction(faction);
	}
}
