//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_CaptureBasesContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		IEntity owner = null;
		int selectedEntitiesCount = 0;
		
		if (selectedEntities)
			selectedEntitiesCount = selectedEntities.Count();
		
		// No entity to perform the action on
		if (!hoveredEntity && selectedEntitiesCount == 0)
			return false;
		
		if (hoveredEntity)
		{
			owner = hoveredEntity.GetOwner();
			if (!owner.IsInherited(SCR_CampaignMilitaryBaseComponent))
					return false;
		}
		
		for (int i = 0; i < selectedEntitiesCount; i++)
		{
			owner = selectedEntities[i].GetOwner();
			if (!owner.IsInherited(SCR_CampaignMilitaryBaseComponent))
				return false;
		}
		
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		array<Faction> factions = new array<Faction>();
		array<int> factionIDs = new array<int>();
		factionManager.GetFactionsList(factions);
		
		if (!factions)
			return;
		
		int factionsCount = factions.Count();
		
		if (factionsCount == 0)
			return;
		
		foreach (Faction f: factions)
			if (SCR_Faction.Cast(f).IsPlayable())
				factionIDs.Insert(factionManager.GetFactionIndex(f));
		
		int factionIDsCount = factionIDs.Count();
		
		if (factionIDsCount == 0)
			return;
		
		if (hoveredEntity)
			PerformOn(hoveredEntity, factionIDs, factionIDsCount);
		
		if (!selectedEntities)
			return;
		
		int selectedEntitiesCount = selectedEntities.Count();
		for (int i = 0; i < selectedEntitiesCount; i++)
		{
			if (selectedEntities[i] == hoveredEntity)
				continue;
			
			PerformOn(selectedEntities[i], factionIDs, factionIDsCount);
		}
	}
	
	void PerformOn(SCR_EditableEntityComponent entity, array<int> factionIDs, int factionIDsCount)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(entity.GetOwner().FindComponent(SCR_CampaignMilitaryBaseComponent));
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (!base)
			return;
		
		int index = factionIDs.Find(factionManager.GetFactionIndex(base.GetFaction())) + 1;
		
		if (index >= factionIDsCount)
			index = 0;
		
		PlayerController pc = GetGame().GetPlayerController();
		
		if (!pc)
			return;
		
		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
		
		if (!campaignNetworkComponent)
			return;
		
		campaignNetworkComponent.CaptureBaseGM(base, factionIDs[index]);
	}
};
