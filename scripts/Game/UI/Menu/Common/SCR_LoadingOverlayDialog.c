/*!
Loading overlay wrapped into a dialog.
It's meant to cover whole screen to prevent user interatction.
*/
class SCR_LoadingOverlayDialog : ChimeraMenuBase
{
	// Called immediately when CloseAnimated is called.
	ref ScriptInvoker m_OnCloseStarted = new ScriptInvoker();
	
	// Called when the close animation has finished. Called from OnMenuClose.
	ref ScriptInvoker m_OnCloseFinished = new ScriptInvoker();
	
	protected SCR_LoadingOverlay m_LoadingOverlay;
	
	//-------------------------------------------------------------------------------------------------------
	// P U B L I C 
	//-------------------------------------------------------------------------------------------------------
	
	
	//-------------------------------------------------------------------------------------------------------
	static SCR_LoadingOverlayDialog Create(string text = string.Empty)
	{
		MenuBase menuBase = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.LoadingOverlay);
		SCR_LoadingOverlayDialog loadingOverlay = SCR_LoadingOverlayDialog.Cast(menuBase);
		loadingOverlay.Init(text);		
		return loadingOverlay;
	}
	
	
	//-------------------------------------------------------------------------------------------------------
	void CloseAnimated(bool invokeOnCloseStarted = true)
	{
		if (invokeOnCloseStarted)
			m_OnCloseStarted.Invoke();
		
		// Play fade-out animation
		AnimateWidget.Opacity(GetRootWidget(), 0, UIConstants.FADE_RATE_FAST);
		GetGame().GetCallqueue().CallLater(Close, 1000.0 / UIConstants.FADE_RATE_FAST);
	}
	
	
	
	
	//-------------------------------------------------------------------------------------------------------
	// P R O T E C T E D
	//-------------------------------------------------------------------------------------------------------
	
	
	//-------------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		Widget w = GetRootWidget().FindWidget("m_LoadingOverlay");
		m_LoadingOverlay = SCR_LoadingOverlay.Cast(w.FindHandler(SCR_LoadingOverlay));
		
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, Callback_OnBackButton);
		
		// Play fade-in animation
		GetRootWidget().SetOpacity(0);
		AnimateWidget.Opacity(GetRootWidget(), 1, UIConstants.FADE_RATE_FAST);
	}
	
	
	//-------------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, Callback_OnBackButton);
		
		m_OnCloseFinished.Invoke();
	}
	
	
	//-------------------------------------------------------------------------------------------------------
	protected void Init(string text)
	{
		m_LoadingOverlay.SetText(text);
	}
	
	
	//-------------------------------------------------------------------------------------------------------
	protected void Callback_OnBackButton()
	{
		this.CloseAnimated();
	}
};
