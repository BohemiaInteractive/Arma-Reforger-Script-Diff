[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetRandomLayerBase : SCR_ScenarioFrameworkGet
{
	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "From this list, random layer will be selected")]
	ref array<string> 	m_aNameOfLayers;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (m_aNameOfLayers.IsEmpty())
			return null;

		string selectedLayer = m_aNameOfLayers.GetRandomElement();

		IEntity entity = FindEntityByName(selectedLayer);
		if (!entity)
			return null;

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
}