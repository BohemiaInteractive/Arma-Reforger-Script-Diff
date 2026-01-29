class SCR_PlayerTeleportFeedbackUIComponent: ScriptedWidgetComponent
{
	[Attribute("0.5", desc: "Long fade delay when moved in and out of vehicles. In seconds.")]
	protected float m_fLongFadeDelay;
	
	[Attribute("TeleportingText", desc: "Teleport Text widget name")]
	protected string m_sTeleportTextName;
	
	protected SCR_FadeUIComponent m_FadeUIComponent;
	protected SCR_FadeUIComponent m_TeleportTextFadeUIComponent;
	protected Widget m_wRoot;
	
	protected void OnPlayerTeleported(bool editorIsOpen, bool longFade, SCR_EPlayerTeleportedReason teleportReason)
	{
		if (editorIsOpen)
			return;
		
		m_FadeUIComponent.CancelFade(false);
		m_wRoot.SetOpacity(1);
		m_wRoot.SetVisible(true);
		
		if (!longFade)
		{
			m_FadeUIComponent.FadeOut(false);
			
			if (m_TeleportTextFadeUIComponent)
				m_TeleportTextFadeUIComponent.GetFadeWidget().SetVisible(false);
		}
		else 
		{
			m_FadeUIComponent.DelayedFadeOut(m_fLongFadeDelay * 1000, false);
			
			if (m_TeleportTextFadeUIComponent)
				m_TeleportTextFadeUIComponent.FadeIn(true);
		}
			
	}
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_FadeUIComponent = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		
		if (!m_FadeUIComponent)
		{
			Print("'SCR_PlayerTeleportFeedbackUIComponent' could not find 'm_FadeUIComponent', make sure the fadecomponent is added before this component", LogLevel.ERROR);
			return;
		}
			
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerTeleportedFeedbackComponent playerTeleportedComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportedComponent)
		{
			Print("'SCR_PlayerTeleportFeedbackUIComponent' could not find 'SCR_PlayerTeleportedFeedbackComponent', make sure it is added to the player controller", LogLevel.ERROR);
			return;
		}
		
		Widget teleportTextWidget = w.FindAnyWidget(m_sTeleportTextName);
		if (teleportTextWidget)
		{
			m_TeleportTextFadeUIComponent = SCR_FadeUIComponent.Cast(teleportTextWidget.FindHandler(SCR_FadeUIComponent));
		}
		
			
		playerTeleportedComponent.GetOnPlayerTeleported().Insert(OnPlayerTeleported);
	}
	override void HandlerDeattached(Widget w)
	{
		if (!m_FadeUIComponent)
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SCR_PlayerTeleportedFeedbackComponent playerTeleportedComponent = SCR_PlayerTeleportedFeedbackComponent.Cast(playerController.FindComponent(SCR_PlayerTeleportedFeedbackComponent));
		if (!playerTeleportedComponent)
			return;
		
		playerTeleportedComponent.GetOnPlayerTeleported().Remove(OnPlayerTeleported);
		
	}
};
