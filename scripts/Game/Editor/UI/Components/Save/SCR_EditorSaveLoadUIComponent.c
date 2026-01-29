class SCR_EditorSaveLoadUIComponent : SCR_SaveDialogUIComponent
{
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm(SCR_InputButtonComponent button, string actionName)
	{
		// Save dialogs
		if (!m_bIsLoad)
		{
			string customName = m_Widgets.m_SaveNameInputComponent0.GetValue();
			/*if (!GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName))
			{
				//--- Creating a new file - save directly
				OnConfirmPrompt();
				return;
			}*/

			//--- Confirm prompt
			m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
			m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
			m_ConfirmPrompt.SetTitle(customName);
			return;
		}

		//--- Loading a file leads to restart, ask first
		
		Widget focusedEntry = GetGame().GetWorkspace().GetFocusedWidget();
		SCR_SaveLoadEntryComponent entry = m_mComponentEntries.Get(focusedEntry);
		if (!entry)
			return;

		SaveGame save = entry.GetSaveData();
		const string displayName = GetSaveDisplayName(save);
		if (!save.IsSavePointGameVersionCompatible())
		{
			// Warning - incompatible version
			m_LoadBadVersionPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadVersionPrompt);
			m_LoadBadVersionPrompt.m_OnConfirm.Insert(LoadEntry);
			m_LoadBadVersionPrompt.SetTitle(displayName);
			return;
		}

		if (!save.AreSavePointAddonsCompatible())
		{
			// Warning - incompatible addons
			m_LoadBadAddonsPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadAddonsPrompt);
			m_LoadBadAddonsPrompt.m_OnConfirm.Insert(LoadEntry);
			m_LoadBadAddonsPrompt.SetTitle(displayName);
			return;
		}

		//--- Confirm prompt
		m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
		m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
		m_ConfirmPrompt.SetTitle(displayName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void SaveEntry()
	{
		const string customName = m_Widgets.m_SaveNameInputComponent0.GetValue();
		
		ESaveGameRequestFlags flags;
		if (RplSession.Mode() == RplMode.None)
			flags = ESaveGameRequestFlags.BLOCKING;

		GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.MANUAL, customName, flags);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDeletePrompt()
	{
		if (m_SelectedSave)
			GetGame().GetSaveGameManager().Delete(m_SelectedSave, new SaveGameOperationCb(OnSaveDeleted, m_SelectedSave));
		
		super.OnDeletePrompt();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSaveDeleted(bool success, Managed context)
	{
		if (!success)
			return;
		
		auto save = SaveGame.Cast(context);
		if (save)
			RemoveSaveEntry(save);
	}

	//------------------------------------------------------------------------------------------------
	override protected void LoadEntry()
	{
		if (!m_SelectedSave)
			return;

		GetGame().GetSaveGameManager().Load(m_SelectedSave);

		/*
		SCR_ServerSaveRequestCallback uploadCallback = saveManager.GetUploadCallback();
		if (uploadCallback)
		{
			uploadCallback.SetOnSuccess(OnLoadEntryUploadResponse);
			uploadCallback.SetOnError(OnLoadEntryUploadError);
			m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	override protected void SelectEntry(Widget w, SCR_SaveLoadEntryComponent entryComponent)
	{
		super.SelectEntry(w, entryComponent);

		const string customName = GetSaveDisplayName(entryComponent);
		m_Widgets.m_SaveNameInputComponent0.SetValue(customName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateButtons()
	{
		if (m_mComponentEntries.IsEmpty())
		{
			m_Widgets.m_DeleteButtonComponent.SetVisible(false);
			m_Widgets.m_OverrideButtonComponent.SetVisible(false);
			m_Widgets.m_ConfirmButtonComponent.SetVisible(false);
			return;
		}

		string customName = m_Widgets.m_SaveNameInputComponent0.GetValue();
		bool isValid = !customName.IsEmpty();
		bool isOverride = false; //!m_bIsLoad && customName && GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName);

		if (m_bIsLoad)
		{
			m_Widgets.m_DeleteButtonComponent.SetVisible(m_SelectedSave != null);
			m_Widgets.m_DeleteButtonComponent.SetEnabled(m_SelectedSave != null);
		}
		else
		{
			m_Widgets.m_DeleteButtonComponent.SetVisible(isOverride);
			m_Widgets.m_DeleteButtonComponent.SetEnabled(isOverride);
		}

		m_Widgets.m_OverrideButtonComponent.SetVisible(isOverride, false);
		m_Widgets.m_OverrideButtonComponent.SetEnabled(isOverride && isValid);

		m_Widgets.m_ConfirmButtonComponent.SetVisible(!isOverride, false);
		m_Widgets.m_ConfirmButtonComponent.SetEnabled(!isOverride && isValid);
	}
}
