[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAttachToSlotManager : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play animation on")]
	ref SCR_ScenarioFrameworkGet m_ActorGetter;
	
	[Attribute(desc: "Prop to be atteched")]
	ref SCR_ScenarioFrameworkGet m_PropToAttachGetter;
	
	[Attribute("")]
	string m_sSlotName;
	
	[Attribute("false", UIWidgets.Auto)]
	bool m_bDetachAndDelete;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_ActorGetter, entity))
			return;
		
		IEntity propEntity;		
		if (!ValidateInputEntity(object, m_PropToAttachGetter, propEntity))
			return;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(entity.FindComponent(SlotManagerComponent));
			
		if (!slotManager)
			return;
		
		EntitySlotInfo entityInSlot = slotManager.GetSlotByName(m_sSlotName);
		if(!entityInSlot)
			return;
		
		if (!m_bDetachAndDelete)
			entityInSlot.AttachEntity(propEntity);
		else
		{
			IEntity entityToDetach = entityInSlot.GetAttachedEntity();
			
			if (entityToDetach)
				SCR_EntityHelper.DeleteEntityAndChildren(entityToDetach);
		}
	}
}