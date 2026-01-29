//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionExecuteFunction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Object the method will be called")]
	ref SCR_ScenarioFrameworkGet		m_ObjectToCallTheMethodFrom;

	[Attribute(desc: "Method to call")]
	string			m_sMethodToCall;

	[Attribute(desc: "Parameter1 to pass (string only)")]
	string		m_sParameter;

	[Attribute(desc: "Parameter2 to pass (string only)")]
	string		m_sParameter2;

	[Attribute(desc: "Parameter3 to pass (string only)")]
	string		m_sParameter3;

	[Attribute(desc: "Parameter4 to pass (string only)")]
	string		m_sParameter4;

	[Attribute(desc: "Parameter5 to pass (string only)")]
	string		m_sParameter5;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_ObjectToCallTheMethodFrom.Get());
		if (!entityWrapper)
			return;

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkArea));
		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkLayerBase));
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entityWrapper.GetValue().FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (layer)
			GetGame().GetCallqueue().CallByName(layer, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else if (layerBase)
			GetGame().GetCallqueue().CallByName(layerBase, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else if (area)
			GetGame().GetCallqueue().CallByName(area, m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
		else
			GetGame().GetCallqueue().CallByName(entityWrapper.GetValue(), m_sMethodToCall, m_sParameter, m_sParameter2, m_sParameter3, m_sParameter4, m_sParameter5);
	}
}
//---- REFACTOR NOTE END ----