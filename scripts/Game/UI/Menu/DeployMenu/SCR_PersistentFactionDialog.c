class SCR_PersistentFactionDialog : SCR_ConfigurableDialogUi
{
	protected int m_iRequestedFactionIndex;
	//protected SCR_ButtonCheckerComponent m_Checker;

	//------------------------------------------------------------------------------------------------
	static SCR_PersistentFactionDialog CreatePersistentFactionDialog(int factionIndex)
	{
		SCR_PersistentFactionDialog dialogUI = new SCR_PersistentFactionDialog();
		SCR_ConfigurableDialogUi.CreateFromPreset("{FC62FADA1444750B}Configs/DeployMenu/DeployMenuDialogPreset.conf", "PERSISTENT_FACTION", dialogUI);
		dialogUI.m_iRequestedFactionIndex = factionIndex;
		
		return dialogUI;
	}
	
	int GetRequestedFactionIndex()
	{
		return m_iRequestedFactionIndex;
	}	

	/*
	todo@lk: uncomment once user settings saving works correctly on ds
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		Widget btn = w.FindAnyWidget("DontShowButton");
		m_Checker = SCR_ButtonCheckerComponent.Cast(btn.FindHandler(SCR_ButtonCheckerComponent));
		GetGame().GetInputManager().AddActionListener("MenuDeploy", EActionTrigger.PRESSED, Toggle);
	}

	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(W);
		
		GetGame().GetInputManager().RemoveActionListener("MenuDeploy", EActionTrigger.PRESSED, Toggle);
	}
	
	protected override void OnConfirm()
	{
		if (m_Checker.IsToggled())
		{
			UserSettings userSettings = GetGame().GetGameUserSettings();
			if (userSettings)
			{
				BaseContainer container = userSettings.GetModule("SCR_DeployMenuSettings");
				if (container)
				{
					container.Set("m_bShowPersistentFactionWarning", false);
					GetGame().UserSettingsChanged();
					GetGame().SaveUserSettings();
				}
			}
		}

		super.OnConfirm();
	}
	
	protected void Toggle()
	{
		m_Checker.SetToggled(!m_Checker.IsToggled());
	}

	static bool CanCreateDialog()
	{
		UserSettings userSettings = GetGame().GetGameUserSettings();
		if (!userSettings)
			return false;

		bool showWarning;
		BaseContainer container = userSettings.GetModule("SCR_DeployMenuSettings");
		if (container)
			container.Get("m_bShowPersistentFactionWarning", showWarning);

		return showWarning;	
	}
	*/	
};