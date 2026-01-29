[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeLayerActivationType : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target Layer to change")]
	ref SCR_ScenarioFrameworkGetLayerBase		m_Getter;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType), category: "Activation")]
	SCR_ScenarioFrameworkEActivationType m_eActivationType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entityFrom = entityWrapper.GetValue();
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entityFrom.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not Layer Base for Action %1.", this), LogLevel.ERROR);
			return;
		}

		layer.SetActivationType(m_eActivationType);
	}
}