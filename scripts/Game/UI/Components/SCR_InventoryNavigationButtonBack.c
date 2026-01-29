//! Interactive non-focus button showing hint, and triggering actions 

//------------------------------------------------------------------------------------------------
class SCR_InventoryNavigationButtonBack : SCR_InputButtonComponent 
{
	protected bool m_bCloseWithGamepad;
	protected SCR_InventoryStorageBaseUI m_pParentStorage = null;
	protected int m_iStorageIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	void OnActivate(Widget w)
	{
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
			return;
		
		OnClick(w, 0, 0, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != 0 || !m_wRoot.IsVisible() || !m_wRoot.IsEnabled())
			return false;
		
		m_OnActivated.Invoke(this, m_sActionName, m_pParentStorage, m_iStorageIndex);
		PlaySound(m_sSoundClicked);
		PlaySound(m_sSoundContainerClosed);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInput()
	{
		if (!m_wRoot)
			return;
		
		if (!m_bCloseWithGamepad)
			return;

		bool bEnabled = m_wRoot.IsEnabled();
		bool bVisible = m_wRoot.IsVisible();
		
		if (!bEnabled || !bVisible)
			return;
				
		m_OnActivated.Invoke(this, m_sActionName, m_pParentStorage, m_iStorageIndex);
	}

	//------------------------------------------------------------------------------------------------
	override bool SetAction(string action, EInputDeviceType currentInputDevice = -1, bool forceUpdate = false)
	{
		// Remove old listener and add a new one
		GetGame().GetInputManager().RemoveActionListener(m_sActionName, EActionTrigger.DOWN, OnInput);
		GetGame().GetInputManager().AddActionListener(m_sActionName, EActionTrigger.DOWN, OnInput);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		
		GetGame().GetWorkspace().SetFocusedWidget(null);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentStorage(SCR_InventoryStorageBaseUI pParentStorage)
	{
		m_pParentStorage = pParentStorage;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStorageIndex(int index)
	{
		m_iStorageIndex = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableCloseWithGamepad(bool enable)
	{
		m_bCloseWithGamepad = enable;
	}

	//------------------------------------------------------------------------------------------------
	int GetStorageIndex()
	{
		return m_iStorageIndex;
	}
};