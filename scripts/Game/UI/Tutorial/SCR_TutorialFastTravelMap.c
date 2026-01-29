class SCR_TutorialFastTravelMapElement : SCR_MapUIElement
{
	protected vector m_vPosition;
	protected IEntity m_TargetEntity;
	
	//------------------------------------------------------------------------------
	void SetFastTravelEntity(string entName)
	{
		m_TargetEntity = GetGame().GetWorld().FindEntityByName(entName);
	}
	
	//------------------------------------------------------------------------------
	void SetFastTravelName(string name)
	{
		TextWidget textWidget = TextWidget.Cast(m_wRoot.FindAnyWidget("Name"));
		if (textWidget)
			textWidget.SetText(name);
		
	}
	
	//------------------------------------------------------------------------------
	override vector GetPos()
	{
		return m_vPosition;
	}
	
	// ------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		SCR_TutorialGamemodeComponent m_TutorialComponent = SCR_TutorialGamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_TutorialGamemodeComponent));
		if (!m_TutorialComponent)
			return false;
		
		if (!m_TargetEntity)
			return false;
		
		MenuManager menuMan = GetGame().GetMenuManager();
		menuMan.FindMenuByPreset(ChimeraMenuPreset.TutorialFastTravel).Close();
		
		m_TutorialComponent.FastTravel(m_TargetEntity);
		return false;
	}
}