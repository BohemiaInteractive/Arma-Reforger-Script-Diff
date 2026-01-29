// hack incoming! just for prototype to see what gadgets in deploy map will look like
class SCR_MapCompassUIDeploy : SCR_MapCompassUI
{
	//------------------------------------------------------------------------------------------------
	protected ResourceName GetGadgetForFaction()
	{
		SCR_PlayerFactionAffiliationComponent factionAffil = SCR_PlayerFactionAffiliationComponent.Cast(
			GetGame().GetPlayerController().FindComponent(SCR_PlayerFactionAffiliationComponent));
		
		ResourceName gadgetRes = "{61D4F80E49BF9B12}Prefabs/Items/Equipment/Compass/Compass_SY183.et";
		
		if (!factionAffil || !factionAffil.GetAffiliatedFaction())
			return gadgetRes;
		
		if (factionAffil.GetAffiliatedFaction().GetFactionKey() == "USSR")
			gadgetRes = "{7CEF68E2BC68CE71}Prefabs/Items/Equipment/Compass/Compass_Adrianov.et";
		
		return gadgetRes;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetPrefabResource()
	{
		return GetGadgetForFaction();
	}

	//------------------------------------------------------------------------------------------------
	override IEntity FindRelatedGadget()
	{
		return m_MapEntity;
	}
}
