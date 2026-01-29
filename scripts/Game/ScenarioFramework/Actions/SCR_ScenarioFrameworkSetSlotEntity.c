[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkSetSlotEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target slot getter")];
	ref SCR_ScenarioFrameworkGet m_SlotGetter;

	[Attribute(desc: "Entity getter")];
	ref SCR_ScenarioFrameworkGet m_EntityGetter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		IEntity targetSlotEntity;
		if (!ValidateInputEntity(object, m_SlotGetter, targetSlotEntity))
			return;
		
		IEntity entity;
		if (!ValidateInputEntity(object, m_EntityGetter, entity))
			return;
		
		if (entity == targetSlotEntity)
		{
			PrintFormat("ScenarioFramework SCR_ScenarioFrameworkSetSlotEntity: Both target and new entity are same for action %1.", this, level:LogLevel.ERROR);
			return;
		}
		
		SCR_ScenarioFrameworkSlotBase slot = SCR_ScenarioFrameworkSlotBase.Cast(targetSlotEntity.FindComponent(SCR_ScenarioFrameworkSlotBase));
		if (!slot)
		{
			PrintFormat("ScenarioFramework SCR_ScenarioFrameworkSetSlotEntity: Target slot entity doesn't have SCR_ScenarioFrameworkSlotBase or inherited component for %1.", this, level:LogLevel.ERROR);
			return;
		}
		
		slot.SetEntity(entity);
		SCR_ScenarioFrameworkSlotAI slotAI = SCR_ScenarioFrameworkSlotAI.Cast(slot);
		if (slotAI)
			slotAI.m_AIGroup = SCR_AIGroup.Cast(entity);
	}
}