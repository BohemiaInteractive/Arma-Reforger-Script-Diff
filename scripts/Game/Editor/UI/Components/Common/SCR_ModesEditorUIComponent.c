//! @ingroup Editor_UI Editor_UI_Components

class SCR_ModesEditorUIComponent : SCR_DialogEditorUIComponent
{	
	//Rename to active modes
	protected ref array<EEditorMode> m_aActiveModes = {};
	protected ref SCR_SortedArray<SCR_EditorModePrefab> m_aOrderedEditorModePrefabs = new SCR_SortedArray<SCR_EditorModePrefab>();
	//Combobox
	protected SCR_CustomDropdownEditorUIComponent m_DropdownWidget;
	
	protected SCR_EditorManagerEntity m_EditorManager;
	
//	//------------------------------------------------------------------------------------------------
//	protected override bool IsUnique()
//	{
//		return false;
//	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnModeAdd(SCR_EditorModeEntity modeEntity)
	{
		if (!modeEntity)
			return;
		
		m_aActiveModes.Insert(modeEntity.GetModeType());
		RefreshModes();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnModeRemove(SCR_EditorModeEntity modeEntity)
	{
		if (!modeEntity) 
			return;
		
		int index = m_aActiveModes.Find(modeEntity.GetModeType());
		if (index < 0)
			return;
		
		m_aActiveModes.Remove(index);
		RefreshModes();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateDropdown()
	{			
		//m_DropdownWidget.ClearAll();
		if (!m_EditorManager)
			return;
		
		SCR_EditorModeEntity currentModeEntity = m_EditorManager.GetCurrentModeEntity();
		
		int count = m_aOrderedEditorModePrefabs.Count();
		
		for(int i = 0; i < count; ++i)
		{
			SCR_EditorModeUIInfo info = m_aOrderedEditorModePrefabs[i].GetInfo();
			
			if (info)
			{
				m_DropdownWidget.AddItem(info, info.GetModeColor());
				
				if (currentModeEntity && currentModeEntity.GetModeType() == m_aOrderedEditorModePrefabs[i].GetMode())
					m_DropdownWidget.SetCurrentItem(i, false);
			}
			
		}
		
		RefreshModes();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnModeSelected(SCR_CustomDropdownEditorUIComponent dropdown, int index)
	{
		if (!m_EditorManager) 
			return;		
		
		m_EditorManager.SetCurrentMode(m_aOrderedEditorModePrefabs[index].GetMode());
		m_DropdownWidget.CloseDropdown();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnModeChange(SCR_EditorModeEntity currentModeEntity, SCR_EditorModeEntity prevModeEntity)
	{	
		//Close if dialog or close list if not (mainly for if mode is forced changed)
		if (m_bIsInDialog)
			CloseDialog();
		
		if (!m_EditorManager)
			return;
			
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace)
		{
			if (workspace.GetFocusedWidget() == m_DropdownWidget.GetRootWidget())
				workspace.SetFocusedWidget(null);
		}
		
		RefreshModes();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RefreshModes()
	{		
		SCR_EditorModeEntity currentModeEntity = m_EditorManager.GetCurrentModeEntity();
		if (!currentModeEntity)
			return;
				
		int modeCount = 0; 

		int count = m_aOrderedEditorModePrefabs.Count();
		for(int i = 0; i < count; ++i)
		{
			m_DropdownWidget.SetItemVisible(i, m_aActiveModes.Contains(m_aOrderedEditorModePrefabs[i].GetMode()));
			
			if (!m_aActiveModes.Contains(m_aOrderedEditorModePrefabs[i].GetMode()))
				continue;

			modeCount++;
			
			//Set combobox selected element
			if (currentModeEntity.GetModeType() == m_aOrderedEditorModePrefabs[i].GetMode())
				m_DropdownWidget.SetCurrentItem(i, false);
		}
		
		//m_DropdownWidget.SetDropdownEnable(modeCount > 1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDialogOpened(SCR_DialogEditorUIComponent linkedComponent)
	{		
		super.OnDialogOpened(linkedComponent);
		//m_DropdownWidget.AllowFocus(true);
	
		int selected = 0;
		
		if (m_EditorManager)
		{
			EEditorMode currentMode = m_EditorManager.GetCurrentMode();
			int count = m_aOrderedEditorModePrefabs.Count();
			
			for(int i = 0; i < count; ++i)
			{
				if (m_aOrderedEditorModePrefabs[i].GetMode() == currentMode)
				{
					selected = i;
					break;
				}
			}
		
		}
	}
	
	//---- REFACTOR NOTE START: Set current mode might be more clear? 
	
	//------------------------------------------------------------------------------------------------
	protected void SetSiblingMode(int relIndex)
	{
		if (!m_EditorManager || m_EditorManager.IsModeChangeRequested())
			return;
		
		array<EEditorMode> modes = {};
		int modesCount = m_EditorManager.GetModes(modes);
		if (modesCount <= 1)
			return;
		
		int index = modes.Find(m_EditorManager.GetCurrentMode()) + relIndex;
		m_EditorManager.SetCurrentMode(modes[(index + modesCount) % modesCount])
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorModePrev(float value, EActionTrigger reason)
	{
		SetSiblingMode(-1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorModeNext(float value, EActionTrigger reason)
	{
		SetSiblingMode(1);
	}

	//------------------------------------------------------------------------------------------------
	override void OnRepeat()
	{
		if (m_EditorManager)
			m_EditorManager.RestorePreviousMode();
	}

	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceIsGamepad(bool isGamepad)
	{		
		if (m_bIsInDialog)
			CloseDialog();
		
		m_DropdownWidget.CloseDropdown();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDropdownOpen(SCR_CustomDropdownEditorUIComponent dropdown)
	{
		if (m_bIsInDialog)
			m_CanCloseWithToggleAction = false;

	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDropdownClosed(SCR_CustomDropdownEditorUIComponent dropdown)
	{
		if (m_bIsInDialog)
		{
			m_CanCloseWithToggleAction = true;
			CloseDialog();
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{				
		super.HandlerAttachedScripted(w);
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		if (!m_EditorManager) 
			return;
		
		m_DropdownWidget = SCR_CustomDropdownEditorUIComponent.Cast(w.FindHandler(SCR_CustomDropdownEditorUIComponent));
		if (!m_DropdownWidget)
			return;
		
		//~Hotfix for border always showing
		Widget border = w.FindAnyWidget("Border");
		if (border)
			border.SetOpacity(0);
		
		m_DropdownWidget.GetOnChanged().Insert(OnModeSelected);
		m_DropdownWidget.GetOnOpened().Insert(OnDropdownOpen);
		
		array<SCR_EditorModeEntity> modeEntities = {};
		int modesCount = m_EditorManager.GetModeEntities(modeEntities);
		for (int i = 0; i < modesCount; i++)
		{
			OnModeAdd(modeEntities[i]);
		}
		
		//Create combobox
		core.GetBaseModePrefabs(m_aOrderedEditorModePrefabs, -1, true);
		CreateDropdown();
		
		SCR_EditorModeEntity currentModeEntity = m_EditorManager.GetCurrentModeEntity();
		
		m_EditorManager.GetOnModeAdd().Insert(OnModeAdd);
		m_EditorManager.GetOnModeRemove().Insert(OnModeRemove);
		m_EditorManager.GetOnModeChange().Insert(OnModeChange);

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("EditorModePrev", EActionTrigger.PRESSED, OnEditorModePrev);
			inputManager.AddActionListener("EditorModeNext", EActionTrigger.PRESSED, OnEditorModeNext);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_EditorManager) 
		{
			m_EditorManager.GetOnModeAdd().Remove(OnModeAdd);
			m_EditorManager.GetOnModeRemove().Remove(OnModeRemove);
			m_EditorManager.GetOnModeChange().Remove(OnModeChange);
		}
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener("EditorModePrev", EActionTrigger.PRESSED, OnEditorModePrev);
			inputManager.RemoveActionListener("EditorModeNext", EActionTrigger.PRESSED, OnEditorModeNext);
		}
		
		if (m_DropdownWidget)
		{
			m_DropdownWidget.GetOnChanged().Remove(OnModeSelected);
			m_DropdownWidget.GetOnOpened().Remove(OnDropdownOpen);
			m_DropdownWidget.GetOnClosed().Remove(OnDropdownClosed);
		}
	}
}
