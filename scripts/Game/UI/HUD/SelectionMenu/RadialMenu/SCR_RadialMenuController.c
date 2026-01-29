/*!
Class created for quick application of radial menu into components
It finds global menu, holds controller settings and data required for menu
*/

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_RadialMenuController
{
	[Attribute("1")]
	protected bool m_sEnableControl;
	
	[Attribute()]
	protected ref SCR_SelectionMenuControllerInputs m_RMControls;
	
	[Attribute()]
	protected ref SCR_SelectionMenuData m_Data;

	protected ref SCR_RadialMenu m_RadialMenuToControl; // Menu that is kept for auto retaking control, can be chenged by user
	protected ref SCR_RadialMenu m_RadialMenu;
	protected IEntity m_Owner;
	
	// Callbacks
	//protected ref ScriptInvoker<SCR_RadialMenuController> m_OnBeforeOpen;
	protected ref ScriptInvoker<SCR_RadialMenuController, bool> m_OnInputOpen;
	protected ref ScriptInvoker<SCR_RadialMenuController> m_OnTakeControl;
	protected ref ScriptInvoker<SCR_RadialMenuController, bool> m_OnControllerChanged;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnInputOpen()
	{
		if (!m_OnInputOpen)
			m_OnInputOpen = new ScriptInvoker();

		return m_OnInputOpen;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnInputOpen(bool hasControl)
	{
		if (m_OnInputOpen)
			m_OnInputOpen.Invoke(this, hasControl);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnTakeControl()
	{
		if (!m_OnTakeControl)
			m_OnTakeControl = new ScriptInvoker();

		return m_OnTakeControl;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnTakeControl()
	{
		if (m_OnTakeControl)
			m_OnTakeControl.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnControllerChanged(bool hasControl)
	{
		if (m_OnControllerChanged)
			m_OnControllerChanged.Invoke(this, hasControl);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnControllerChanged()
	{
		if (!m_OnControllerChanged)
			m_OnControllerChanged = new ScriptInvoker();

		return m_OnControllerChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Take control over selected menu
	//! Filling no menu will take control over the global radial menu
	void Control(IEntity owner, SCR_RadialMenu radialMenu = null)
	{
		// Clear previous inputs 
		if (radialMenu)
			radialMenu.GetOnControllerChanged().Remove(OnMenuControllerChanged);
		
		m_Owner = owner;
		
		// Setup radial menu
		if (radialMenu == null)
			radialMenu = SCR_RadialMenu.GlobalRadialMenu();
				
		m_RadialMenu = radialMenu;
		if (!m_RadialMenu)	
			return;
		
		m_RadialMenu.SetController(m_Owner, m_RMControls);
		
		if (m_RadialMenu)
			m_RadialMenu.GetOnControllerChanged().Insert(OnMenuControllerChanged);
		
		InvokeOnTakeControl();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stop opening controls of current menu
	void StopControl(bool closeMenu = false)
	{	
		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnControllerChanged().Remove(OnMenuControllerChanged);
			
			if (closeMenu)
				m_RadialMenu.Close();
		
			m_RadialMenu = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetMenuDisplay()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateMenuData()
	{
		if (m_RadialMenu)
			m_RadialMenu.AddEntries(m_Data.GetEntries(), true);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void OnInputOpen()
	{	
		bool hasControl = HasControl();
		InvokeOnInputOpen(hasControl);
		
		if (!m_sEnableControl)
			return;
		
		if (!HasControl())
			Control(m_Owner, m_RadialMenuToControl);
		
		// Check owner character concious state
		if (!m_RMControls.m_bShowWhileUnconcious)
		{	
			IEntity controllerEntity = SCR_PlayerController.GetLocalControlledEntity();
			if (controllerEntity)
			{
				CharacterControllerComponent characterControler = CharacterControllerComponent.Cast(controllerEntity.FindComponent(CharacterControllerComponent));
				if (characterControler && characterControler.IsUnconscious())
					return;
			}
		}
		
		// Setup and open menu 
		if (!m_RadialMenu)
			return;
		
		if (!m_RadialMenu.HasDisplay())
			m_RadialMenu.SetMenuDisplay();
		
		m_RadialMenu.Open();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuControllerChanged(SCR_SelectionMenu menu, SCR_SelectionMenuControllerInputs controller)
	{
		// Check menu
		if (menu != m_RadialMenu)
			return;
		
		// Check control 
		if (controller != m_RMControls)
		{
			menu.ClearEntries();
			
			m_RadialMenu.Close();
			InvokeOnControllerChanged(false);
			
			
			StopControl();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Get
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetEnableControl(bool enable)
	{
		m_sEnableControl = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetEnableControl()
	{
		return m_sEnableControl;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRadialMenuToControl(SCR_RadialMenu radialMenu)
	{
		m_RadialMenuToControl = radialMenu;
	}
		
	//------------------------------------------------------------------------------------------------
	SCR_RadialMenu GetRadialMenu()
	{
		return m_RadialMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuControllerInputs GetControls()
	{
		return m_RMControls;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuData GetData()
	{
		return m_Data;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsMenuOpen()
	{		
		return m_RadialMenu && m_RadialMenu.IsOpened();
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseMenu()
	{
		if (m_RadialMenu)
			m_RadialMenu.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true if cached menu is controlled by this owner entity 
	bool HasControl()
	{
		if (!m_RadialMenu)
		{
			Print("[SCR_RadialMenuController] - has no menu selected to control!", LogLevel.WARNING);
			return false;
		}
		
		if (!m_RadialMenu.GetControllerInputs())
			return false;

		return m_RadialMenu.GetControllerInputs() == m_RMControls;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuController()
	{
		if (m_RMControls && !m_RMControls.m_sOpenAction.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_RMControls.m_sOpenAction, EActionTrigger.DOWN, OnInputOpen);
	}
}