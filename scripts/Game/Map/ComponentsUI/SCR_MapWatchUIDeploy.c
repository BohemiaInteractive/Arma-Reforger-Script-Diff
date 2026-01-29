// hack incoming! just for prototype to see what gadgets in deploy map will look like
class SCR_MapWatchUIDeploy : SCR_MapWatchUI
{
	//------------------------------------------------------------------------------------------------
	protected ResourceName GetGadgetForFaction()
	{
		SCR_PlayerFactionAffiliationComponent factionAffil = SCR_PlayerFactionAffiliationComponent.Cast(
			GetGame().GetPlayerController().FindComponent(SCR_PlayerFactionAffiliationComponent));
		
		ResourceName gadgetRes = "{78ED4FEF62BBA728}Prefabs/Items/Equipment/Watches/Watch_SandY184A.et";
		
		if (!factionAffil || !factionAffil.GetAffiliatedFaction())
			return gadgetRes;
		
		if (factionAffil.GetAffiliatedFaction().GetFactionKey() == "USSR")
			gadgetRes = "{6FD6C96121905202}Prefabs/Items/Equipment/Watches/Watch_Vostok.et";
		
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
