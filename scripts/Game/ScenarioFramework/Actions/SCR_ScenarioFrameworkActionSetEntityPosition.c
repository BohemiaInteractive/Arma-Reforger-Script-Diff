[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetEntityPosition : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be teleported (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;

	[Attribute(defvalue: "0 0 0", desc: "Position that the entity will be teleported to")]
	vector 	m_vDestination;

	[Attribute(desc: "Name of the entity that above selected entity will be teleported to (Optional)")]
	ref SCR_ScenarioFrameworkGet m_DestinationEntityGetter;

	[Attribute(defvalue: "0 0 0", desc: "Position that will be used in relation to the entity for the position to teleport to (Optional)")]
	vector 	m_vDestinationEntityRelativePosition;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_EntityGetter, entity))
			return;

		if (!m_DestinationEntityGetter)
		{
			entity.SetOrigin(m_vDestination);
			entity.Update();
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> destinationEntityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_DestinationEntityGetter.Get());
		if (!destinationEntityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Destination Entity Getter has issues for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		IEntity destinationEntity = destinationEntityWrapper.GetValue();
		if (!destinationEntity)
		{
			Print(string.Format("ScenarioFramework Action: Destination Entity could not be found for action %1. Action won't do anything.", this), LogLevel.ERROR);
			return;
		}

		entity.SetOrigin(destinationEntity.GetOrigin() + m_vDestinationEntityRelativePosition);
		entity.Update();
	}
}