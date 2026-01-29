class SCR_NoGameMasterStickyNotificationUIComponent : SCR_StickyNotificationUIComponent
{
	protected SCR_PlayerDelegateEditorComponent m_DelegateManager;
	
	//------------------------------------------------------------------------------------------------
	protected void CheckShowNotification(bool isInit = false)
	{
		//Check if there is at least one GM. If not, set the sticky active else hide the sticky
		SetStickyActive(!m_DelegateManager.HasPlayerWithUnlimitedEditor(), !isInit);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLimitedEditorChanged()
	{
		CheckShowNotification();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnButton()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		
		if (menuManager)
			menuManager.OpenDialog(ChimeraMenuPreset.PlayerListMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{			
		super.OnInit(notificationLog);
		
		//Never show if everybody is GM or nobody is GM. 
		SCR_GameModeEditor gameMode = SCR_GameModeEditor.Cast(GetGame().GetGameMode());
		if (gameMode && gameMode.GetGameMasterTarget() == EGameModeEditorTarget.VOTE)
		{
			m_DelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
			if (m_DelegateManager)
			{
				m_DelegateManager.GetOnLimitedEditorChanged().Insert(OnLimitedEditorChanged);
				CheckShowNotification(true);
				return;
			}
		}

		SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDestroy()
	{
		if (m_DelegateManager)
			m_DelegateManager.GetOnLimitedEditorChanged().Remove(OnLimitedEditorChanged);
	}
}
