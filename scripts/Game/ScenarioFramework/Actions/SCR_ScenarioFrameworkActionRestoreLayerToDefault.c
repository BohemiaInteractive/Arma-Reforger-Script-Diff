[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRestoreLayerToDefault : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Layer to be restored to default (Optional if action is attached on layer that is supposed to be restored to default)")]
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;
	
	[Attribute(defvalue: "true", desc: "If checked, it will also restore child layers to default state as well.")]
	bool m_bIncludeChildren;
	
	[Attribute(desc: "If checked, it will reinit the layer after the restoration")]
	bool m_bReinitAfterRestoration;
	
	[Attribute(defvalue: "true", desc: "If checked, it will also clear randomization and re-randomize it again instead of using cached elements from initial randomization")]
	bool m_bAffectRandomization;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}

		layer.RestoreToDefault(m_bIncludeChildren, m_bReinitAfterRestoration, m_bAffectRandomization);
	}
}