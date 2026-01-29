//------------------------------------------------------------------------------------------------
//! Class that handles focusing of widgets for the Welcome and Debriefing screens
class SCR_WelcomeScreenFocusWidgetComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected string m_sTargetContent;
	
	[Attribute()]
	protected string m_sTargetHighlight;
	
	[Attribute()]
	protected string m_sDeployMenuClass;
	
	//------------------------------------------------------------------------------------------------
	override event bool OnFocus(Widget w, int x, int y)
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTargetContent) && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTargetHighlight))
			ToggleInteractions(true, w);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool OnFocusLost(Widget w, int x, int y)
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTargetContent) && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTargetHighlight))
			ToggleInteractions(false, w);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggles interactions for said widget
	void ToggleInteractions(bool enabled, Widget w)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode || !w)
			return;
		
		SCR_DeployMenuBaseScreenComponent deployMenuBase;
		if ("SCR_WelcomeScreenComponent" == m_sDeployMenuClass)
			deployMenuBase = SCR_WelcomeScreenComponent.Cast(gameMode.FindComponent(SCR_WelcomeScreenComponent));
		else if ("SCR_DebriefingScreenComponent" == m_sDeployMenuClass)
			deployMenuBase = SCR_DebriefingScreenComponent.Cast(gameMode.FindComponent(SCR_DebriefingScreenComponent));
		
		if (!deployMenuBase)
			return;
			
		SCR_DeployMenuBaseScreenLayout baseLayout = deployMenuBase.GetBaseLayout();
		if (!baseLayout)
			return;
			
		array<ref SCR_WelcomeScreenBaseContent> screenBaseContents = {};
		baseLayout.GetScreenBaseContents(screenBaseContents);
			
		foreach (SCR_WelcomeScreenBaseContent content : screenBaseContents)
		{
			if (content.GetContentName() != m_sTargetContent)
				continue;
			
			if (content.GetIsInteractible())
			{
				content.ToggleInteractions(enabled);
				
				if (enabled)
					w.FindAnyWidget(m_sTargetHighlight).SetOpacity(0.5);
				else
					w.FindAnyWidget(m_sTargetHighlight).SetOpacity(0);
			}
			else
			{
				w.SetFlags(WidgetFlags.NOFOCUS);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! When widget is initialised, we set the ToggleInteraction logic on false by default
	override event void HandlerAttached(Widget w)
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTargetContent) && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sTargetHighlight))
			GetGame().GetCallqueue().CallLater(ToggleInteractions, 100, false, false, w);
	}	
};
