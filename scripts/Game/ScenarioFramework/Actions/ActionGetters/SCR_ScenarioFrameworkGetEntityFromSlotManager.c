[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetEntityFromSlotManager : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sLayerName;
	
	[Attribute(defvalue: "", desc: "Slot Name")]
	string m_sSlotName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sLayerName);
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;
		
		entity = layer.GetSpawnedEntity();
		if (!entity)
			return null;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(entity.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return null;
		
		EntitySlotInfo slotInfo = slotManager.GetSlotByName(m_sSlotName);
		if (!slotInfo)
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(slotInfo.GetAttachedEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	
}