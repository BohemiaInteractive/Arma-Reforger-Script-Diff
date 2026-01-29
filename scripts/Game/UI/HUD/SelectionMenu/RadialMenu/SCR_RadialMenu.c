enum SCR_ERadialMenuType
{
	DEFAULT,
	ADD_COMMANDS_MENU
}

void ScriptInvokerSelectionMenuEntryMethod(SCR_SelectionMenuEntry entry);
typedef func ScriptInvokerSelectionMenuEntryMethod;
typedef ScriptInvokerBase<ScriptInvokerSelectionMenuEntryMethod> ScriptInvokerSelectionMenuEntry;

//------------------------------------------------------------------------------------------------
/*!
Radial menu class specifing behavior and interaction for circular interface
Can distrubute and select entries in set angle in distace
Can be interacted with mouse or gamepad thumbsticks
*/
[BaseContainerProps(configRoot: true)]
class SCR_RadialMenu : SCR_SelectionMenu
{
	protected static SCR_RadialMenu m_GlobalRadialMenu;
	protected static SCR_RadialMenu m_OpenedRadialMenu;  // reference to any currently open radial menu inheriting from this class
	
	const float SIZE_LARGE = 580;
	protected const float SIZE_SMALL = 400;

	protected vector m_vMenuCenterPos;

	protected float m_fEntriesAngleDistance;
	protected float m_fEntryAngleOffset;

	protected float m_fPointingAngle;
	protected bool m_bIsPointingToCenter;
	protected float m_fDynamicMouseSelectionTreshold;

	protected bool m_bActivateContext;
	protected bool m_bPreventSelectionContext;

	protected SCR_RadialMenuInputs m_RadialInputs;
	protected SCR_RadialMenuControllerInputs m_RadialControllerInputs;
	
	protected string m_sActionHint;

	// Events
	protected ref ScriptInvoker<SCR_RadialMenu, float> m_OnDisplaySizeChange;
	protected ref ScriptInvoker<SCR_RadialMenu, string> m_OnSetActionHint;
	
	SCR_ERadialMenuType m_eRadialType = SCR_ERadialMenuType.DEFAULT;

	protected ref ScriptInvokerSelectionMenuEntry m_OnEntryPerformed;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerSelectionMenuEntry GetOnEntryPerformed()
	{
		if (!m_OnEntryPerformed)
			m_OnEntryPerformed = new ScriptInvokerSelectionMenuEntry();

		return m_OnEntryPerformed;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnDisplaySizeChange(float size)
	{
		if (m_OnDisplaySizeChange)
			m_OnDisplaySizeChange.Invoke(this, size);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDisplaySizeChange()
	{
		if (!m_OnDisplaySizeChange)
			m_OnDisplaySizeChange = new ScriptInvoker();

		return m_OnDisplaySizeChange;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnSetActionHint(string action)
	{
		if (m_OnSetActionHint)
			m_OnSetActionHint.Invoke(this, action);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnSetActionHint()
	{
		if (!m_OnSetActionHint)
			m_OnSetActionHint = new ScriptInvoker();

		return m_OnSetActionHint;
	}
	
	//------------------------------------------------------------------------------------------------
	// Getting global menu
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	//! Find and get reference to global redial menu
	static SCR_RadialMenu GlobalRadialMenu()
	{
		if (!m_GlobalRadialMenu)
		{
			PlayerController controller = GetGame().GetPlayerController();
			if (!controller)	
				return null;
			
			SCR_RadialMenuGameModeComponent rm =  SCR_RadialMenuGameModeComponent.Cast(controller.FindComponent(SCR_RadialMenuGameModeComponent));
			
			if (!rm)
			{
				#ifdef RADMENU_DEBUG
				Print("[SCR_RadialMenuController] - Can't setup radial menu due to missing RM game mode!");
				#endif 
				return null;
			}
			
			m_GlobalRadialMenu = rm.GetMenu();
		}
		
		return m_GlobalRadialMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find and get reference to global redial menu
	static SCR_RadialMenu GetOpenedRadialMenu()
	{
		return m_OpenedRadialMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuOpened(ChimeraMenuBase menu)
	{
		if (m_OpenedRadialMenu)
			m_OpenedRadialMenu.Close();
	}

	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected override event void OnOpen()
	{
		super.OnOpen(owner);
		
		if (m_OpenedRadialMenu)
			m_OpenedRadialMenu.Close();
		
		// Setup controls
		m_Inputs.Init();
		AddActionListeners();

		GetGame().GetCallqueue().Remove(ReleaseContext);
		m_bActivateContext = true;
		
		m_OpenedRadialMenu = this;

		DeselectEntry();
	}

	//------------------------------------------------------------------------------------------------
	protected override event void OnClose()
	{
		super.OnClose(owner);
		RemoveActionListeners();
		
		m_OpenedRadialMenu = null;

		// Allow to stop using context after split of a second to prevent unwanted input
		if (m_RadialInputs)
			GetGame().GetCallqueue().CallLater(ReleaseContext, m_RadialInputs.m_iContextDeactivationTime);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReleaseContext()
	{
		m_bActivateContext = false;
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (System.IsConsoleApp())
			return;
		
		// Context
		if (m_bActivateContext && !m_bPreventSelectionContext)
			GetGame().GetInputManager().ActivateContext(m_Inputs.m_sContext);

		// Controller inputs
		if (m_bActivateContext && m_ControllerInputs)
			GetGame().GetInputManager().ActivateContext(m_ControllerInputs.m_sControllerContext);

		OnUpdate(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Empty method called on update ready for override
	override protected void OnUpdate(float timeSlice)
	{
		if (!m_bOpened)
			return;
		
		if (!m_bPreventSelectionContext)
			SelectEntry();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Radial menu set menu display will auto find HUD manager radial menu display if display is null
	override void SetMenuDisplay(SCR_SelectionMenuDisplay display = null)
	{
		// By default find radial menu display from HUD manager 
		if (!display)
		{
			SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
			
			if (hud)
			{
				display = SCR_SelectionMenuDisplay.Cast(hud.FindInfoDisplay(SCR_RadialMenuDisplay));
			}
		}
		
		super.SetMenuDisplay(display);
		
		// Setup cofiguration
		if (m_RadialControllerInputs)
			ChangeDisplaySize(m_RadialControllerInputs.m_bUseLargeSize, m_RadialControllerInputs.m_fCustomSize);
	}

	//------------------------------------------------------------------------------------------------
	//! For gamepad - based on direction of stick
	override protected void SelectEntry()
	{
		SCR_SelectionMenuEntry entry;
		int id = -1;

		entry = HandleSelection(id);
		
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			if (m_bIsPointingToCenter && m_RadialControllerInputs.m_bDeselectInCenter)
			{
				// Call deselection after split of a second to prevent deselection happening during closing
				if (GetGame().GetCallqueue().GetRemainingTime(DeselectEntry) == -1)
					GetGame().GetCallqueue().CallLater(DeselectEntry, m_RadialInputs.m_iGamepadDeselectionDelay);

				return;
			}
		}

		GetGame().GetCallqueue().Remove(DeselectEntry);

		// Update selection
		if (m_SelectedEntry != entry)
		{
			m_SelectedEntry = entry;
			InvokeOnSelect(entry, id);
			
			PlaySound(m_sSelectionSound);
			
			// Setup input action  
			if (entry && !entry.GetInputAction().IsEmpty())
				SetActionHint(entry.GetInputAction());
			else
				HideActionHint();
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void InvokeOnUpdateEntries(array<ref SCR_SelectionMenuEntry> entries)
	{
		if (!m_aEntries.IsEmpty())
			m_fEntriesAngleDistance = 360 / m_aEntries.Count();

		super.InvokeOnUpdateEntries(entries);
	}

	//------------------------------------------------------------------------------------------------
	override void SetController(IEntity owner, SCR_SelectionMenuControllerInputs controls)
	{
		super.SetController(owner, controls);

		m_RadialControllerInputs = SCR_RadialMenuControllerInputs.Cast(controls);
		m_RadialInputs = SCR_RadialMenuInputs.Cast(m_Inputs);

		if (m_RadialControllerInputs && m_RadialInputs)
			m_RadialInputs.SetUseRightStick(m_RadialControllerInputs.m_bUseRightStick);
		
		#ifdef RADMENU_DEBUG
		// Set controller configuration from diag menu
		DebugSetupControllerInputs(m_RadialControllerInputs);
		DebugFillWithEntries();
		#endif
		
		m_eRadialType = SCR_ERadialMenuType.DEFAULT;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPerformInput()
	{
		super.OnPerformInput();

		if (!m_SelectedEntry)
		{
			if (m_ControllerInputs.m_bCloseOnPerform)
				Close();
		}

		if (m_OnEntryPerformed)
			m_OnEntryPerformed.Invoke(m_SelectedEntry);
	}

	//------------------------------------------------------------------------------------------------
	//! Add 1-9 quick actions 
	override protected void AddActionListeners() 
	{
		super.AddActionListeners();
		
		SCR_MenuHelper.GetOnMenuOpened().Insert(OnMenuOpened);
		
		// Quick actions 1-9
		if (!m_RadialControllerInputs.m_bUseQuickActions)
			return;
		
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot1", EActionTrigger.DOWN, OnQuickAction1);
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot2", EActionTrigger.DOWN, OnQuickAction2);
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot3", EActionTrigger.DOWN, OnQuickAction3);
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot4", EActionTrigger.DOWN, OnQuickAction4);
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot5", EActionTrigger.DOWN, OnQuickAction5);
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot6", EActionTrigger.DOWN, OnQuickAction6);
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot7", EActionTrigger.DOWN, OnQuickAction7);
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot8", EActionTrigger.DOWN, OnQuickAction8);
		GetGame().GetInputManager().AddActionListener("InventoryQuickSlot9", EActionTrigger.DOWN, OnQuickAction9);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove 1-9 quick actions 
	override protected void RemoveActionListeners()
	{
		super.RemoveActionListeners();
		
		SCR_MenuHelper.GetOnMenuOpened().Remove(OnMenuOpened);
		
		// Quick actions 1-9
		if (!m_RadialControllerInputs.m_bUseQuickActions)
			return;
		
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot1", EActionTrigger.DOWN, OnQuickAction1);
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot2", EActionTrigger.DOWN, OnQuickAction2);
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot3", EActionTrigger.DOWN, OnQuickAction3);
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot4", EActionTrigger.DOWN, OnQuickAction4);
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot5", EActionTrigger.DOWN, OnQuickAction5);
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot6", EActionTrigger.DOWN, OnQuickAction6);
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot7", EActionTrigger.DOWN, OnQuickAction7);
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot8", EActionTrigger.DOWN, OnQuickAction8);
		GetGame().GetInputManager().RemoveActionListener("InventoryQuickSlot9", EActionTrigger.DOWN, OnQuickAction9);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnQuickAction1() { QuickActionUse(1); }
	protected void OnQuickAction2() { QuickActionUse(2); }
	protected void OnQuickAction3() { QuickActionUse(3); }
	protected void OnQuickAction4() { QuickActionUse(4); }
	protected void OnQuickAction5() { QuickActionUse(5); }
	protected void OnQuickAction6() { QuickActionUse(6); }
	protected void OnQuickAction7() { QuickActionUse(7); }
	protected void OnQuickAction8() { QuickActionUse(8); }
	protected void OnQuickAction9() { QuickActionUse(9); }
	
	//------------------------------------------------------------------------------------------------
	protected void QuickActionUse(int id)
	{
		// - 1 because slot actions starts with 1, but entries with 0
		id -= 1;
		
		if (!m_aEntries.IsIndexValid(id))
			return;
		
		m_SelectedEntry = m_aEntries[id];
		m_iSelectedEntryId = id;
		
		if (!SCR_SelectionMenuCategoryEntry.Cast(m_SelectedEntry))
			InvokeOnSelect(m_SelectedEntry, id);
		
		m_bEntryPerformed = true;
		PerformEntry(m_SelectedEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected int GetSelectedElementIndex(float angle, int elementCount)
	{
		if (elementCount == 0)
			return -1;

		float angleDeg = Math.Repeat(angle * Math.RAD2DEG, 360) - m_fEntryAngleOffset;
		float overflow = m_fEntriesAngleDistance * 0.5;

		// compensate overflowing offset
		if (angleDeg < 0)
		{
			angleDeg = Math.Repeat(angleDeg, 360);
		}

		if (m_fEntriesAngleDistance == 0)
		{
			#ifdef RADMENU_DEBUG
			DebugPrint("GetSelectedElementIndex", "No distance between entries!");
			#endif
			return -1;
		}

		// Find entry index withing circle
		int idx = Math.Round(angleDeg / m_fEntriesAngleDistance);

		if (idx < 0 || idx >= elementCount)
		{
			idx = -1;
		}

		// Count with overflow
		if (idx == -1)
		{
			if (angleDeg > (360 - overflow + m_fEntryAngleOffset))
				return 0;
			else
				return -1;
		}

		return idx;
	}

	//------------------------------------------------------------------------------------------------
	//! Multiply selection distance from center with ratio of reference resolution to current resoluution
	//! Should fix distance being calculated same for various resolutions
	protected float AdjustDistanceWithResolution(float distance)
	{
		// Current
		float curW, curH;
		GetGame().GetWorkspace().GetScreenSize(curW, curH);

		if (curW == 0 || curH == 0)
			return distance;

		// Reference 1920x1080
		int refW, refH;
		WidgetManager.GetReferenceScreenSize(refW, refH);
		
		float ratioX = curW / refW;
		float ratioY = curH / refH;
		
		float ratioDist = vector.Distance(
			Vector(m_vMenuCenterPos[0] / refW, m_vMenuCenterPos[1] / refH, 0),
			Vector(curW / refW, curH / refH,
			0));

		return distance / ratioDist;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_SelectionMenuEntry HandleSelection(out int id)
	{
		// Check m_Inputs
		if (!m_RadialInputs || !m_RadialControllerInputs)
			return null;
		
		float dist;
		vector dir;
		
		// Handle used input 
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			// Mouse 
			int mouseX, mouseY;
			WidgetManager.GetMousePos(mouseX, mouseY);
			dir = Vector(mouseX, mouseY, 0);
			
			dist = vector.Distance(m_vMenuCenterPos, dir);
			dist = AdjustDistanceWithResolution(dist);
			
			// Center the direction
			dir[0] = mouseX - m_vMenuCenterPos[0];
			dir[1] = (mouseY - m_vMenuCenterPos[1]) * -1;
			dir.Normalize();
			
			// Deselect if in center
			m_bIsPointingToCenter = (
				m_RadialControllerInputs.m_bDeselectInCenter && dist < m_fDynamicMouseSelectionTreshold);
			
			if (m_bIsPointingToCenter)
				return null;
		}
		else
		{
			// Gamepad 
			float x, y;
			m_RadialInputs.GetRadialXYInput(x, y);
			
			dir[0] = x;
			dir[1] = y;
			dist = vector.Distance(vector.Zero, Vector(x, y, 0));
			
			// Deselect if in center
			m_bIsPointingToCenter = (
				m_RadialControllerInputs.m_bDeselectInCenter && dist < m_RadialInputs.m_fGamepadSelectionTreshhold);
			
			if (m_bIsPointingToCenter)
				return null;
		}
		
		// Selected pointed
		float angle = Math.Atan2(dir[0], dir[1]);
		id = GetSelectedElementIndex(angle, m_aEntries.Count());
		
		m_fPointingAngle = angle * Math.RAD2DEG;
		
		if (m_aEntries.IsIndexValid(id))
			return m_aEntries[id];
		
		id = -1;
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void DeselectEntry()
	{
		m_SelectedEntry = null;
		InvokeOnSelect(null, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check quick 1-10 actions and perform entry on action trigger
	protected void ReactQuickInput()
	{
		for (int i = 1; i < 9; i++)
		{
			if (GetGame().GetInputManager().GetActionTriggered("InventoryQuickSlot" + i))
			{
				// - 1 because slot actions starts with 1, but entries with 0
				int id = i-1;
				
				if (!m_aEntries.IsIndexValid(id))
					continue;
				
				m_SelectedEntry = m_aEntries[id];
				m_iSelectedEntryId = id;
				
				if (!SCR_SelectionMenuCategoryEntry.Cast(m_SelectedEntry))
					InvokeOnSelect(m_SelectedEntry, id);
				
				m_bEntryPerformed = true;
				PerformEntry(m_SelectedEntry);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call to influence size of radial menu display layout
	//! useLarge = true will use predefined size for large menu radius (SIZE_LARGE)
	//! Custom size will ignore useLarge
	void ChangeDisplaySize(bool useLarge = true, float customSize = -1)
	{
		float size = SIZE_LARGE;
		if (!useLarge)
			size = SIZE_SMALL;

		if (customSize > 0)
			size = customSize;

		// Change seletion treshold
		m_fDynamicMouseSelectionTreshold = m_RadialInputs.m_fMouseSelectionTreshold;

		if (m_RadialInputs.m_bDynamicMouseTreshold)
			m_fDynamicMouseSelectionTreshold *= size / SIZE_LARGE; // Mutliply with large size based ratio

		// Invoke
		InvokeOnDisplaySizeChange(size);
	}

	//------------------------------------------------------------------------------------------------
	//! Set current hint to display and send invoke it so radial menu display will show the hint
	void SetActionHint(string action)
	{
		m_sActionHint = action;
		InvokeOnSetActionHint(m_sActionHint);
	}
	
	//------------------------------------------------------------------------------------------------
	//1 Invoke emtpy action to hide in radial menu display
	void HideActionHint()
	{
		InvokeOnSetActionHint("");
	}
	
	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void SetMenuCenterPos(vector centerPos)
	{
		m_vMenuCenterPos = centerPos;
	}

	//------------------------------------------------------------------------------------------------
	float GetEntriesAngleDistance()
	{
		return m_fEntriesAngleDistance;
	}

	//------------------------------------------------------------------------------------------------
	float GetPointingAngle()
	{
		return m_fPointingAngle;
	}

	//------------------------------------------------------------------------------------------------
	bool IsPointingToCenter()
	{
		return m_bIsPointingToCenter;
	}
	
	//------------------------------------------------------------------------------------------------
	// Debug
	//------------------------------------------------------------------------------------------------

	#ifdef RADMENU_DEBUG
	
	//------------------------------------------------------------------------------------------------
	override protected void DebugPrint(string method, string msg)
	{
		Print(string.Format("[SCR_RadialMenu] - %1() - '%2'", method, msg));
	}

	//------------------------------------------------------------------------------------------------
	protected void DebugSetupDiagMenu()
	{
		string rmDiagName = "Radial menu (refactored)";

		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_RADIALMENU_MENU, rmDiagName, "UI");

		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CONFIG_DEBUG, "", "Override config", rmDiagName);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CLOSE_ON_RELEASE_OPEN, "", "Close on open", rmDiagName);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_PERFORM_ON_CLOSE, "", "Perform on close", rmDiagName);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CLOSE_ON_PERFORM, "", "Close on perform", rmDiagName);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_DESELECT_IN_CENTER, "", "Delect in center", rmDiagName);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_USE_LARGE_SIZE, "", "Large size", rmDiagName);

		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CUSTOM_SIZE_ENABLE, "", "Override size", rmDiagName);
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CUSTOM_SIZE, "", "Sustom size", rmDiagName, "100, 1000, 100, 1");

		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_ENTRY_COUNT_ENABLE, "", "Entries setup", rmDiagName);
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_RADIALMENU_ENTRY_COUNT, "", "Entries", rmDiagName, "2, 32, 2, 1");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DebugCleanup()
	{	
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CONFIG_DEBUG);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CLOSE_ON_RELEASE_OPEN);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_PERFORM_ON_CLOSE);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CLOSE_ON_PERFORM);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_DESELECT_IN_CENTER);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_USE_LARGE_SIZE);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CUSTOM_SIZE_ENABLE);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CUSTOM_SIZE);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_ENTRY_COUNT_ENABLE);
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_RADIALMENU_ENTRY_COUNT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override controller inputs by radial menu diag menu
	protected void DebugSetupControllerInputs(out notnull SCR_RadialMenuControllerInputs inputs)
	{
		// Check if debug is enabled
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CONFIG_DEBUG))
			return;
		
		inputs.m_bCloseOnReleaseOpen = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CLOSE_ON_RELEASE_OPEN);
		inputs.m_bPerformOnClose = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_PERFORM_ON_CLOSE);
		inputs.m_bCloseOnPerform = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CLOSE_ON_PERFORM);
		inputs.m_bDeselectInCenter = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_DESELECT_IN_CENTER);
		inputs.m_bUseLargeSize = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_USE_LARGE_SIZE);
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CUSTOM_SIZE_ENABLE))
			inputs.m_fCustomSize = DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_RADIALMENU_CUSTOM_SIZE);
		else
			inputs.m_fCustomSize = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DebugFillWithEntries()
	{
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RADIALMENU_ENTRY_COUNT_ENABLE))
			return;
		
		m_aEntries.Clear();
		
		int count = DiagMenu.GetRangeValue(SCR_DebugMenuID.DEBUGUI_RADIALMENU_ENTRY_COUNT);
		for (int i = 0; i < count; i++)
		{
			SCR_SelectionMenuEntry entry = new SCR_SelectionMenuEntry();
			entry.SetName("Debug entry " + i);
			entry.Enable(true);
			
			AddEntry(entry);
		}
	}
	
	#endif
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenu()
	{
		#ifdef RADMENU_DEBUG
		DebugSetupDiagMenu();
		#endif
	} 
		
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenu()
	{
		#ifdef RADMENU_DEBUG
		DebugCleanup();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPreventSelectionContext(bool enable)
	{
		m_bPreventSelectionContext = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetPreventSelectionContext()
	{
		return m_bPreventSelectionContext;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetUseQuickActions()
	{
		if (!m_RadialControllerInputs)
			return false;
		
		return m_RadialControllerInputs.m_bUseQuickActions;
	}
};

//------------------------------------------------------------------------------------------------
/*!
Configurable radial menu inputs extending controls specifically for radial interface
*/
[BaseContainerProps(configRoot: true)]
class SCR_RadialMenuInputs : SCR_SelectionMenuInputs
{
	protected const string RADIAL_LEFT_CONTEXT = "RadialMenuLeftContext";
	protected const string RADIAL_RIGHT_CONTEXT = "RadialMenuRightContext";
	protected const string DEFAULT_RADIAL_LEFT_X = "RadialX";
	protected const string DEFAULT_RADIAL_LEFT_Y = "RadialY";
	protected const string DEFAULT_RADIAL_RIGHT_X = "RadialX2";
	protected const string DEFAULT_RADIAL_RIGHT_Y = "RadialY2";
	
	[Attribute("100", desc: "How far from center needs to be to select entry. Use when controller m_bDeselectInCenter = true")]
	float m_fMouseSelectionTreshold;
	
	[Attribute("250", UIWidgets.Slider, desc: "Delay in ms for how long should context be active - prevents character rotation and other unwanted input", "0 1000 1")]
	int m_iContextDeactivationTime;
	
	[Attribute("1", desc: "Allow dynamically setup inner mouse selection treshold base on menu size from SIZE_LARGE")]
	bool m_bDynamicMouseTreshold;
	
	[Attribute("0.5", "0 1 0.01", desc: "How far from center needs to be to select entry. Use when controller m_bDeselectInCenter = true")]
	float m_fGamepadSelectionTreshhold;
	
	[Attribute("500", UIWidgets.Slider, desc: "How long in milisecond should selection stay when moving to center.", "0 1000 1")]
	int m_iGamepadDeselectionDelay;

	protected bool m_bUseRightStick;
		
	//------------------------------------------------------------------------------------------------
	//! Set default setting on initialization
	//! Set X and Y action names into given parameters 
	override void Init()
	{
		SetUseRightStick(m_bUseRightStick);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetRadialXYInput(out float x, out float y)
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (m_bUseRightStick)
		{
			inputManager.ActivateContext(RADIAL_RIGHT_CONTEXT, m_iGamepadDeselectionDelay);
			x = inputManager.GetActionValue(DEFAULT_RADIAL_RIGHT_X);
			y = inputManager.GetActionValue(DEFAULT_RADIAL_RIGHT_Y);
		}
		else
		{
			inputManager.ActivateContext(RADIAL_LEFT_CONTEXT, m_iGamepadDeselectionDelay);
			x = inputManager.GetActionValue(DEFAULT_RADIAL_LEFT_X);
			y = inputManager.GetActionValue(DEFAULT_RADIAL_LEFT_Y);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetUseRightStick(bool use)
	{
		m_bUseRightStick = use;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_RadialMenuControllerInputs : SCR_SelectionMenuControllerInputs
{
	const static int MAX_HINTS = 9;
	
	[Attribute("1", desc: "True = right thumbstick used for menu navigation, otherwise use left thumbstick")]
	bool m_bUseRightStick;
	
	[Attribute("1", desc: "Select no entry if selection is pointing into menu center")]
	bool m_bDeselectInCenter;
	
	[Attribute("1", desc: "Use large or small size for radial menu visuals")]
	bool m_bUseLargeSize;
	
	[Attribute("-1", UIWidgets.Slider, desc: "Base radial menu size. Will ignore m_bUseLargeSize and use custom size instead of presets if it's more than 0", "-1 1000 1")]
	float m_fCustomSize;
	
	[Attribute("0", UIWidgets.Slider, desc: "Display the crosshair in the center of the layout")]
	bool m_bShowCrosshair;
	
	[Attribute("1", UIWidgets.Slider, desc: "Display inner circle backround if true")]
	bool m_bShowInnerBackground;
	
	[Attribute("0", UIWidgets.Slider, desc: "Allows to use entries with quick 1-10 input")]
	bool m_bUseQuickActions;
}