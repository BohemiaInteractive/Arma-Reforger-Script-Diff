// Script File 
/**
A Generic entity faction changer. Will ignore arsenals
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EntityFactionEditorAttribute : SCR_BaseFactionEditableAttribute
{			
	protected override bool ValidEntity(GenericEntity entity)
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(entity.FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return false;

		Vehicle vehicle = Vehicle.Cast(entity);
		if (vehicle)
			return false;

		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(entity);
		if (arsenalComponent)
			return false;

		return true;
	}
	
	protected override Faction GetFaction(GenericEntity entity)
	{
		return SCR_FactionAffiliationComponent.Cast(entity.FindComponent(SCR_FactionAffiliationComponent)).GetAffiliatedFaction();
	}
	
	protected override void SetFaction(GenericEntity entity, Faction faction)
	{
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(entity.FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent)
			return;
		
		factionComponent.SetAffiliatedFaction(faction);
	}
};