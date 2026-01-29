//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SupplyBasesContextAction : SCR_SelectedEntitiesContextAction
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
			SCR_CampaignMilitaryBaseComponent militaryBase = SCR_CampaignMilitaryBaseComponent.Cast(owner.FindComponent(SCR_CampaignMilitaryBaseComponent));
			if (!militaryBase)
				return false;
			
			if (militaryBase.GetType() == SCR_ECampaignBaseType.RELAY)
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
		if (hoveredEntity)
			PerformOn(hoveredEntity);
		
		if (!selectedEntities)
			return;
		
		int selectedEntitiesCount = selectedEntities.Count();
		for (int i = 0; i < selectedEntitiesCount; i++)
		{
			if (selectedEntities[i] == hoveredEntity)
				continue;
			
			PerformOn(selectedEntities[i]);
		}
	}
	
	void PerformOn(SCR_EditableEntityComponent entity)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(entity.GetOwner().FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (!base)
			return;
		
		PlayerController pc = GetGame().GetPlayerController();
		
		if (!pc)
			return;
		
		SCR_CampaignNetworkComponent comp = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
		
		if (!comp)
			return;
		
		comp.AddSuppliesFromContextMenu(base, 1000);
	}
};
