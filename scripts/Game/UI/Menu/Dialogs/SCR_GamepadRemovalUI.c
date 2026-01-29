//------------------------------------------------------------------------------------------------
//! Dialog displayed when the gamepad is removed
class SCR_GamepadRemovalUI : DialogUI
{
	protected const string TEXTURE_XBOX = 		"{F669664236AE1AD7}UI/Textures/Common/Xbox_controller.edds";
	protected const string TEXTURE_PLAYSTATION = 	"{7DB8A1E50905C0EB}UI/Textures/Common/PlayStation_controller.edds";
	
	protected const string GAMEPAD_ANY_BUTTON_ACTION = "GamepadAnyButton";
	protected const string KEYBOARD_ANY_BUTTON_ACTION = "KeyboardAnyButton";	
	
	protected static ref ScriptInvokerDialog m_OnGamepadRemovalDialogOpen;
	protected static ref ScriptInvokerDialog m_OnGamepadRemovalDialogClose;
	
	protected Widget m_wBackground;
	protected ImageWidget m_wImage;
	protected Widget m_wImageContainer;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
	
		if (GetGame().m_bIsMainMenuOpen)
		{
			// Default background opacity is semi-transparent. In Core Menus, we want it to be fully opaque
			m_wBackground = GetRootWidget().FindAnyWidget("BackgroundRounded");
			if (m_wBackground)
				m_wBackground.SetOpacity(1);
		}
		
		m_wImageContainer = GetRootWidget().FindAnyWidget("ContentLayoutContainer");
		m_wImage = ImageWidget.Cast(GetRootWidget().FindAnyWidget("ControllerImage"));
		UpdateImage();
		
		GetGame().GetInputManager().AddActionListener(GAMEPAD_ANY_BUTTON_ACTION, EActionTrigger.PRESSED, CloseAnimated);
		GetGame().GetInputManager().AddActionListener(KEYBOARD_ANY_BUTTON_ACTION, EActionTrigger.PRESSED, CloseAnimated);
		
		if (m_OnGamepadRemovalDialogOpen)
			m_OnGamepadRemovalDialogOpen.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose() 
	{
		super.OnMenuClose();
		
		GetGame().GetInputManager().RemoveActionListener(GAMEPAD_ANY_BUTTON_ACTION, EActionTrigger.PRESSED, CloseAnimated);
		GetGame().GetInputManager().RemoveActionListener(KEYBOARD_ANY_BUTTON_ACTION, EActionTrigger.PRESSED, CloseAnimated);
		
		if (m_OnGamepadRemovalDialogClose)
			m_OnGamepadRemovalDialogClose.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateImage()
	{
		if (!m_wImage)
		{
			if (m_wImageContainer)
				m_wImageContainer.SetVisible(false);
			
			return;
		}
		
		switch (System.GetPlatform())
		{
			case EPlatform.PS4:
			case EPlatform.PS5:
			case EPlatform.PS5_PRO:
			{
				m_wImage.LoadImageTexture(0, TEXTURE_PLAYSTATION);
				break;
			}
			
			case EPlatform.XBOX_ONE:
			case EPlatform.XBOX_ONE_S:
			case EPlatform.XBOX_ONE_X:
			case EPlatform.XBOX_SERIES_S:
			case EPlatform.XBOX_SERIES_X:
			{
				m_wImage.LoadImageTexture(0, TEXTURE_XBOX);
				break;
			}
			
			default:
			{
				if (m_wImageContainer)
					m_wImageContainer.SetVisible(false);
				else
					m_wImage.SetVisible(false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_GamepadRemovalUI OpenGamepadRemovalDialog()
	{
		MenuBase dialog;
		MenuManager menuManager;
		ArmaReforgerScripted game = GetGame();
		if (game)
			menuManager = game.GetMenuManager();

		if (menuManager && game.IsPlatformGameConsole())
			dialog = menuManager.OpenDialog(ChimeraMenuPreset.GamepadRemovalDialog, DialogPriority.CRITICAL, 0, true);
		
		return SCR_GamepadRemovalUI.Cast(dialog);
		
		//TODO: What happens during interaction with input fields, if the controller is removed while the console keyboard is displayed?
	}
	
	//------------------------------------------------------------------------------------------------
	static void CloseGamepadRemovalDialog()
	{
		ArmaReforgerScripted game = GetGame();
		MenuManager menuManager = game.GetMenuManager();

		if (game.IsPlatformGameConsole())
			menuManager.CloseMenuByPreset(ChimeraMenuPreset.GamepadRemovalDialog);
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerDialog GetOnGamepadRemovalDialogClose()
	{
		if (!m_OnGamepadRemovalDialogClose)
			m_OnGamepadRemovalDialogClose = new ScriptInvokerDialog();

		return m_OnGamepadRemovalDialogClose;
	}
}