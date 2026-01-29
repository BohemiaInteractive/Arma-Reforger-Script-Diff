[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetEntityName : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", desc: "Name")]
	string m_sName;
	
	[Attribute(desc: "Entity to be renamed")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		entity.SetName(m_sName);
	}
}