[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowWidget : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", desc: "Layout", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "layout")]
	ResourceName m_sLayout;
	
	[Attribute(defvalue: "0", desc: "Duration", params: "0 inf")]
	int m_iHideTimeDelay;
	
	[Attribute()]
	protected ref array<string> m_aLines;
	
	protected SCR_ScenarioInfoUI m_ScenarioInfoComponent;
	protected Widget m_wRoot;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(m_sLayout);
		m_ScenarioInfoComponent = SCR_ScenarioInfoUI.Cast(m_wRoot.FindHandler(SCR_ScenarioInfoUI));
		if (m_ScenarioInfoComponent)
			m_ScenarioInfoComponent.Show(m_aLines, this);
	}
	
	protected void OnDeactivate()
	{
		m_ScenarioInfoComponent.Hide();
	}
	
	void ShowAnimationEnded()
	{
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(OnDeactivate, m_iHideTimeDelay * 1000, false);
	}
	
	void HideAnimationEnded()
	{
		m_wRoot.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ScenarioFrameworkActionShowWidget()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy()
	}
}