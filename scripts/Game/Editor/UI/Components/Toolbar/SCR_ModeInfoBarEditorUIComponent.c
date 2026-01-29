class SCR_ModeInfoBarEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute("OpenPauseMenuButton")]
	protected string m_sPauseMenuButtonName;

	[Attribute("CloseEditorButton")]
	protected string m_sCloseEditorButtonName;
	
	[Attribute("EditingSave")]
	protected string m_sEditingSave;
	
	protected SCR_EditedSaveUIComponent m_EditedSaveUI;

	//------------------------------------------------------------------------------------------------
	protected void OpenPauseMenuButton(Widget widget, float value, EActionTrigger actionTrigger)
	{
		if (!GetGame().GetMenuManager().IsAnyDialogOpen())
			ArmaReforgerScripted.OpenPauseMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseEditorButton(Widget widget, float value, EActionTrigger actionTrigger)
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();

		if (editorManagerEntity)
			editorManagerEntity.Close();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		Widget buttonWidget = w.FindAnyWidget(m_sPauseMenuButtonName);
		if (buttonWidget)
			ButtonActionComponent.GetOnAction(buttonWidget).Insert(OpenPauseMenuButton);

		buttonWidget = w.FindAnyWidget(m_sCloseEditorButtonName);
		if (buttonWidget)
			ButtonActionComponent.GetOnAction(buttonWidget).Insert(CloseEditorButton);
		
		Widget editedSaveWidget = w.FindAnyWidget(m_sEditingSave);
		if (editedSaveWidget)
			m_EditedSaveUI = SCR_EditedSaveUIComponent.Cast(editedSaveWidget.FindHandler(SCR_EditedSaveUIComponent));
		
		// Hide workshop sharing saves in MP and for PS
		// TODO: Remove force hide once uploading is re-implemented for Xbox/Steam
		if (true || Replication.IsRunning() || System.GetPlatform() == EPlatform.PS5 || System.GetPlatform() == EPlatform.PS4 || System.GetPlatform() == EPlatform.PS5_PRO)
		{
			if (m_EditedSaveUI)
				m_EditedSaveUI.GetRootWidget().SetVisible(false);
			
			return;
		}

		if (GetGame().InPlayMode())
		{
			bool show = GetGame().GetSaveGameManager().IsSavingAllowed();

			SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
			if (show && editorManagerEntity)
				show = editorManagerEntity.GetCurrentMode() == EEditorMode.EDIT;

			if (m_EditedSaveUI)
				m_EditedSaveUI.GetRootWidget().SetVisible(show);
		}
		else
		{
			if (m_EditedSaveUI)
				m_EditedSaveUI.GetRootWidget().SetVisible(m_EditedSaveUI.GetRootWidget().IsVisible());
		}
	}
}
