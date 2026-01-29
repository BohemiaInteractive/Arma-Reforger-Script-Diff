[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetEntityScale : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to be scaled.")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;

	[Attribute(defvalue: "1", desc: "Scale of the entity, where 1 is default size.")]
	float m_fEntityScale;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_EntityGetter, entity))
			return;

		entity.SetScale(m_fEntityScale);
	}
}