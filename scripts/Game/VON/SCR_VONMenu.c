//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_VONMenu
{
	[Attribute()]
	protected ref SCR_RadialMenuController m_RadialController;
	
	[Attribute()]
	protected ref SCR_RadialMenu m_RadialMenu;
	
	const float ADJUST_COOLDOWN = 0.175;	// seconds
	
	protected bool m_bIsDisabled;			
	protected bool m_bIsModifierActive;
	protected float m_fAdjustCooldown;		// cooldown before you can cycle up/down by holding a button
	protected float m_FrequencyListTimer;	// timer before frequency list is hidden 
	
	protected SCR_VONEntry m_SelectedEntry;				// entry which is hovered or selected by controller
	protected SCR_VONEntryRadio m_LastSelectedEntry; 	// last selected entry, to check if item preview should be reloaded
	protected SCR_VONRadialDisplay m_Display;			// cached display to allow external access
	protected SCR_VONController m_VONController;
		
	//------------------------------------------------------------------------------------------------
	SCR_RadialMenu GetRadialMenu()
	{
		return m_RadialMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMenuDisabled(bool state)
	{
		m_bIsDisabled = state;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get channel name tied to a frequency
	//! \param frequency is the subject frequency
	//! \return channel name or empty string if not found
	static string GetKnownChannel(int frequency)
	{
		SCR_FactionManager factionMgr =  SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionMgr)
			return string.Empty;
		
		SCR_Faction milFaction = SCR_Faction.Cast(factionMgr.SGetPlayerFaction(GetGame().GetPlayerController().GetPlayerId()));
		if (milFaction && milFaction.GetFactionRadioFrequency() == frequency)
			return "#AR-Comm_PlatoonChannel";
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!milFaction || !groupManager)
			return string.Empty;
		
		array<SCR_AIGroup> playableGroups = groupManager.GetPlayableGroupsByFaction(milFaction);
		if (!playableGroups)
			return string.Empty;
		
		foreach (int i, SCR_AIGroup group : playableGroups)
		{				
			if (group.GetRadioFrequency() == frequency)
			{
				string company, platoon, squad, character, format;
				group.GetCallsigns(company, platoon, squad, character, format);
				
				return WidgetManager.Translate(format, company, platoon, squad, character);
			}
		}

		SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		if (!groupTaskManager || !groupTaskManager.IsEnabledAssigningFrequencies())
			return string.Empty;

		SCR_Task foundTask = groupTaskManager.GetTaskByFrequency(milFaction, frequency);
		if (foundTask)
			return groupTaskManager.GetTaskTranslatedName(foundTask);

		return string.Empty;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Add simple entry
	void AddRadialEntry(notnull SCR_VONEntry entry, SCR_SelectionMenuCategoryEntry category = null)
	{
		entry.SetName(entry.GetDisplayText());
		
		if (category)
			category.AddEntry(entry);
		else
			m_RadialMenu.AddEntry(entry);
		
		entry.InitEntry();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add simple category
	void AddRadialCategory(notnull SCR_SelectionMenuCategoryEntry entry)
	{		
		entry.SetName(entry.GetName());
		m_RadialMenu.AddCategoryEntry(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get sibling transceiver indeces when there are more in a single device
	protected int GetGroupedRadioIndex(notnull SCR_VONEntryRadio entry)
	{
		// TODO support for more than 2 transceivers
		
		array<ref SCR_VONEntry> entries = {};
		m_VONController.GetVONEntries(entries);
		SCR_VONEntryRadio radioEntry;
		BaseRadioComponent radioComp = entry.GetTransceiver().GetRadio();
		
		foreach (int i, SCR_VONEntry entryVON : entries)
		{
			if (entry == entryVON)
				continue;
			
			radioEntry = SCR_VONEntryRadio.Cast(entryVON);
			if (radioEntry && radioEntry.GetTransceiver().GetRadio() == radioComp)
				return i;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get sibling transceiver entries when there are more in a single device
	protected array<SCR_VONEntryRadio> GetGroupedEntries(notnull SCR_VONEntryRadio entry)
	{
		array<SCR_VONEntryRadio> grouped = {};
		array<ref SCR_VONEntry> entries = {};
		m_VONController.GetVONEntries(entries);
		SCR_VONEntryRadio radioEntry;
		BaseRadioComponent radioComp = entry.GetTransceiver().GetRadio();
		
		foreach (SCR_VONEntry entryVON : entries)
		{
			if (entry == entryVON)
				continue;
			
			radioEntry = SCR_VONEntryRadio.Cast(entryVON);
			if (radioEntry && radioEntry.GetTransceiver().GetRadio() == radioComp)
				grouped.Insert(radioEntry);
		}
			
		return grouped;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenuController event
	protected void OnInputOpenMenu(SCR_RadialMenuController controller, bool hasControl)
	{
		/*if (!m_RadialMenu.HasDisplay())	// TODO currently has to be called after control, sequencing needs adjusting
		{
			SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
			m_Display = SCR_VONRadialDisplay.Cast(hud.FindInfoDisplay(SCR_VONRadialDisplay));
			m_RadialMenu.SetMenuDisplay(m_Display);
		}*/
		
		if (!hasControl && !m_bIsDisabled)	
		{
			m_RadialController.Control(GetGame().GetPlayerController(), m_RadialMenu);
			
			SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
			m_Display = SCR_VONRadialDisplay.Cast(hud.FindInfoDisplay(SCR_VONRadialDisplay));
			m_RadialMenu.SetMenuDisplay(m_Display);
		}
		else if (m_bIsDisabled || m_RadialMenu.IsOpened())
		{
			m_RadialMenu.Close();
			m_RadialController.SetEnableControl(false);
			return;
		}
				
		m_RadialController.SetEnableControl(true);
		
		if (!m_VONController.GetVONComponent())
		{
			if (!m_VONController.AssignVONComponent())
			{
				m_RadialController.SetEnableControl(false);
				return;
			}
		}
		
		if (m_VONController.GetVONEntryCount() == 0)
		{
			m_RadialController.SetEnableControl(false);
			return;
		}
		
		m_RadialMenu.ClearEntries();
		
		array<ref SCR_VONEntry> entries = {};
		m_VONController.GetVONEntries(entries);
		
		foreach (SCR_VONEntry entry : entries)
		{
			AddRadialEntry(entry);
		}
		
		if (m_RadialMenu.GetEntryCount() < 8 && m_RadialMenu.GetEntryCount() % 2 != 0)
		{
			SCR_VONEntry dummy = new SCR_VONEntry();
			dummy.Enable(false);
			dummy.SetIcon("{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset", "VON_radio");
			AddRadialEntry(dummy);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenuController event
	protected void OnControllerTakeControl(SCR_RadialMenuController controller)
	{
		m_RadialMenu.GetOnPerform().Insert(OnEntryPerformed);
		m_RadialMenu.GetOnSelect().Insert(OnEntrySelected);
		m_RadialMenu.GetOnOpen().Insert(OnOpenMenu);
		m_RadialMenu.GetOnClose().Insert(OnCloseMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenuController event
	protected void OnControllerChanged(SCR_RadialMenuController controller)
	{
		m_RadialMenu.GetOnPerform().Remove(OnEntryPerformed);
		m_RadialMenu.GetOnSelect().Remove(OnEntrySelected);
		m_RadialMenu.GetOnOpen().Remove(OnOpenMenu);
		m_RadialMenu.GetOnClose().Remove(OnCloseMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenu event
	protected void OnOpenMenu(SCR_SelectionMenu menu)
	{				
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.AddActionListener("VONMenuTuneFrequency", EActionTrigger.VALUE, ActionTuneFrequency);
		inputMgr.AddActionListener("VONMenuCycleChannel", EActionTrigger.VALUE, ActionCycleChannel);
		inputMgr.AddActionListener("VONMenuAction", EActionTrigger.DOWN, OnMenuToggle);
		inputMgr.AddActionListener("VONMenuMuteAction", EActionTrigger.DOWN, OnMenuMuteToggle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenu event
	protected void OnCloseMenu(SCR_SelectionMenu menu)
	{	
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.RemoveActionListener("VONMenuTuneFrequency", EActionTrigger.VALUE, ActionTuneFrequency);
		inputMgr.RemoveActionListener("VONMenuCycleChannel", EActionTrigger.VALUE, ActionCycleChannel);
		inputMgr.RemoveActionListener("VONMenuAction", EActionTrigger.DOWN, OnMenuToggle);
		inputMgr.RemoveActionListener("VONMenuMuteAction", EActionTrigger.DOWN, OnMenuMuteToggle);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenu event
	protected void OnEntryPerformed(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry)
	{
		m_VONController.SetEntryActive(SCR_VONEntry.Cast(entry), true);				
		
		m_RadialMenu.UpdateEntries();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenu event
	protected void OnEntrySelected(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry, int id)
	{
		if (m_SelectedEntry)
			m_SelectedEntry.SetSelected(false);
		
		m_SelectedEntry = SCR_VONEntry.Cast(entry);
		if (m_SelectedEntry)
			m_SelectedEntry.SetSelected(true);
		
		SCR_VONEntryRadio selected = SCR_VONEntryRadio.Cast(entry);
		if (selected)
		{
			int grouped = GetGroupedRadioIndex(selected);
			m_Display.MarkSegmentBackground(grouped);
			m_Display.UpdateFrequencyList(selected);
						
			if (!m_LastSelectedEntry || m_LastSelectedEntry.GetTransceiver().GetRadio() != selected.GetTransceiver().GetRadio())
			{
				m_Display.SetPreviewItem(selected.GetGadget().GetOwner());
				m_Display.FadeItemPreview();
			}
			
			m_LastSelectedEntry = selected;
		}
		else 
		{
			m_Display.MarkSegmentBackground(-1);
			m_Display.SetFrequenciesVisible(false);
			
			m_Display.SetPreviewItem(null);
			m_LastSelectedEntry = null;
		}
		
		m_RadialMenu.UpdateEntries();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_VONController event
	protected void OnActiveEntriesChanged(SCR_VONEntry entry, bool isActive)
	{
		m_RadialMenu.UpdateEntries();
	}
		
	//------------------------------------------------------------------------------------------------
	// ACTION LISTENER CALLBACKS
	//------------------------------------------------------------------------------------------------
	protected void OnAdjustEntry(int input, bool isModif)
	{
		if (!m_RadialMenu.GetSelectionEntry())
			return;
		
		m_Display.SetFrequenciesVisible(true);
		m_FrequencyListTimer = 3;
		
		SCR_VONEntry entry = SCR_VONEntry.Cast(m_RadialMenu.GetSelectionEntry());
		
		if (isModif)
			entry.AdjustEntryModif(input);
		else
			entry.AdjustEntry(input);
			
			
		SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
		if (radioEntry)
		{
			m_Display.UpdateFrequencyList(radioEntry);
			radioEntry.SetChannelText(GetKnownChannel(radioEntry.GetEntryFrequency()));
			radioEntry.Update();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Menu item configuration callback
	protected void ActionTuneFrequency(float value, EActionTrigger reason)
	{
		if (value == 0)
			return;

		if (value < 0)
			OnAdjustEntry(-1, true);
		else if (value > 0)
			OnAdjustEntry(1, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Menu item configuration callback
	protected void ActionCycleChannel(float value, EActionTrigger reason)
	{
		if (value == 0)
			return;

		if (value < 0)
			OnAdjustEntry(-1, false);
		else if (value > 0)
			OnAdjustEntry(1, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Entry toggle callback
	protected void OnMenuToggle(float value, EActionTrigger reason)
	{		
		if (!m_RadialMenu.GetSelectionEntry())
			return;
								
		SCR_VONEntry.Cast(m_RadialMenu.GetSelectionEntry()).ToggleEntry();
		SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(m_RadialMenu.GetSelectionEntry());
		if (!radioEntry)
			return;

		GetGame().GetCallqueue().CallLater(UpdateEntries, 150); // TODO the radio getter IsPowered is workign with a strange delay, so update with a delay now and wait for transceivers to be powerable 
			
		//radioEntry.Update();
		
		array<SCR_VONEntryRadio> grouped = GetGroupedEntries(radioEntry);	//radio transceiver grouping
		foreach (SCR_VONEntryRadio entry : grouped)
		{
			//entry.Update(); // currently toggle will turn the entire radio off so just update the grouped transceiver here
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Entry toggle callback
	protected void OnMenuMuteToggle(float value, EActionTrigger reason)
	{		
		SCR_SelectionMenuEntry selectedMenuEntry = m_RadialMenu.GetSelectionEntry();
		if (!selectedMenuEntry)
			return;
								
		SCR_VONEntryRadio selectedRadioEntry = SCR_VONEntryRadio.Cast(selectedMenuEntry);
		if (!selectedRadioEntry)
			return;
		
		bool isMuted = selectedRadioEntry.ToggleMuteEntry();
		SCR_VONEntry vonEntry = SCR_VONEntry.Cast(m_RadialMenu.GetSelectionEntry());
		if (isMuted)
		{		
			// Select another entry that is not muted
			array<ref SCR_SelectionMenuEntry> entries = m_RadialMenu.GetEntries();
			SCR_VONEntryRadio radioEntry;
			foreach (SCR_SelectionMenuEntry entry : entries)
			{
				if (entry == selectedMenuEntry)
					continue;
				
				radioEntry = SCR_VONEntryRadio.Cast(entry);
				if (!radioEntry || radioEntry.GetIsMuted())
					continue;
				
				m_VONController.SetEntryActive(SCR_VONEntry.Cast(entry), true);
				break;
			}
		}

		UpdateEntries();
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO temporary function to update through calllater since it cant be called with overloaded methods
	protected void UpdateEntries()
	{
		m_RadialMenu.UpdateEntries();
	}
	
	//------------------------------------------------------------------------------------------------
	//! frame update
	void Update(float timeSlice)
	{
		if (!m_RadialMenu)
			return;
		
		m_RadialMenu.Update(timeSlice);
		
		if (m_RadialMenu.IsOpened() && m_fAdjustCooldown > 0)
			m_fAdjustCooldown -= timeSlice;
		
		if (m_FrequencyListTimer > 0)
		{
			m_FrequencyListTimer -= timeSlice;
			if (m_FrequencyListTimer <= 0)
				m_Display.SetFrequenciesVisible(false);
		}

	}
	
	//------------------------------------------------------------------------------------------------
	void Init(SCR_VONController controllerVON)
	{
		m_VONController = controllerVON;
		
		m_VONController.GetOnEntriesActiveChangedInvoker().Insert(OnActiveEntriesChanged);
		
		m_RadialController.GetOnInputOpen().Insert(OnInputOpenMenu);
		m_RadialController.GetOnTakeControl().Insert(OnControllerTakeControl);
		m_RadialController.GetOnControllerChanged().Insert(OnControllerChanged);
	}
};
