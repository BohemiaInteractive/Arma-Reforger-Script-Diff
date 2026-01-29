[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetFactionToCAHArea : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity of the CAH Area (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute()]
	FactionKey m_sFactionKey;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_CaptureArea captureArea = SCR_CaptureArea.Cast(entity);
		if (!captureArea)
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(factionManager.FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent) 
			m_sFactionKey = factionAliasComponent.ResolveFactionAlias(m_sFactionKey);
		
		Faction newFaction = factionManager.GetFactionByKey(m_sFactionKey);
		if (!newFaction)
			return;
		
		Faction previousFaction = captureArea.GetOwningFaction();
		captureArea.SetOwningFactionInternal(previousFaction, newFaction);
		captureArea.SetOwningFactionInternalAuthority(previousFaction, newFaction);
	}
}