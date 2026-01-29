// todo@lk: figure out what to call this, journal is probably not the best choice, idiot
class SCR_MapJournalUI : SCR_MapUIBaseComponent
{
	[Attribute("JournalFrame", desc: "Root frame widget name")]
	protected string m_sRootWidgetName;
	
	[Attribute("faction", desc: "Map task list imageset quad name")]
	protected string m_sTaskListToolMenuIconName;
	
	[Attribute("exclamationCircle", desc: "Toolmenu imageset quad name")]
	protected string m_sToolMenuIconName;
	
	[Attribute("8", desc: "Offset substracted from total widget size.")]
	protected int m_iOffset;

	protected SCR_MapToolMenuUI m_ToolMenu;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	protected SCR_MapTaskListUI m_MapTaskList;
	protected SCR_JournalSetupConfig m_JournalConfig;
	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;
	protected SCR_MapJournalUIButton m_LastInteractedEntry;
	
	protected ref array<SCR_MapJournalUIButton> m_aEntries = {};
	protected int m_iCurrentEntryId = -1;

	protected Widget m_wJournalFrame;

	//We'll store the bool in order to know if is it the first opening or not
	protected bool m_bFirstOpening = true;
	
	protected ref SCR_JournalWidgets m_Widgets = new SCR_JournalWidgets();
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_RespawnBriefingComponent briefingComp = SCR_RespawnBriefingComponent.GetInstance();
		if (!briefingComp)
			return;

		if (!briefingComp.LoadJournalConfig())
			return;

		m_JournalConfig = briefingComp.GetJournalSetup();
		if (!m_JournalConfig)
			return;
	
		m_ToolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (m_ToolMenu)
		{
			m_ToolMenuEntry = m_ToolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, m_sToolMenuIconName, 1, m_bIsExclusive);
			m_ToolMenuEntry.m_OnClick.Insert(ToggleVisible);
			m_ToolMenuEntry.GetOnDisableMapUIInvoker().Insert(DisableMapUIComponent);
			m_ToolMenuEntry.SetEnabled(true);
		}
		
		m_MapTaskList = SCR_MapTaskListUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapTaskListUI));
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		if (!m_JournalConfig)
			return;

		m_wJournalFrame = m_RootWidget.FindAnyWidget(m_sRootWidgetName);
		if (!m_wJournalFrame)
			return;
		
		if (!m_wJournalFrame.GetChildren())
		{
			Widget journal = GetGame().GetWorkspace().CreateWidgets(m_Widgets.GetLayout(), m_wJournalFrame);
			if (!journal)
				return;
			
			m_Widgets.Init(journal);
		}

		if (m_Widgets.m_wFocusButton)
			m_Widgets.m_FocusButtonComponent.GetOnFocus().Insert(FocusOnFirstEntry);

		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!m_PlyFactionAffilComp)
			return;

		m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse);

		GetJournalForPlayer();		
		
		SCR_RespawnBriefingComponent briefingComp = SCR_RespawnBriefingComponent.GetInstance();	

		if (!m_bFirstOpening || !briefingComp || (briefingComp && !briefingComp.ShowJournalOnStart()))
			ToggleVisible();
		
		if (m_bFirstOpening)
		{
			//if opened for the first time, use the value from config
			ShowEntryByID(m_JournalConfig.GetJournalEntryToBeShown());
			m_bFirstOpening = false;
		}
		else
		{
			//if opened the second and other times, use the last opened one
			ShowEntryByID(m_iCurrentEntryId);
		}
		
		//## Need to be called in order to wait for widget initialization.
		GetGame().GetCallqueue().Call(UpdateJournalPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adjuts size of journal to match height of tool bar
	protected void UpdateJournalPosition()
	{
		Widget toolMenuBackground = m_ToolMenu.GetBackgroundWidget();
		toolMenuBackground.Update();
		
		float sizeW, sizeY;
		toolMenuBackground.GetScreenSize(sizeW, sizeY);
		
		sizeY = GetGame().GetWorkspace().DPIUnscale(sizeY) - m_iOffset;
		
		m_Widgets.m_wSizeLayout.SetHeightOverride(sizeY);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionResponse(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (response)
		{
			m_Widgets.m_wEntryLayout.SetVisible(false);
			GetJournalForPlayer();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle visibility for the journal
	protected void ToggleVisible()
	{
		if (!m_wJournalFrame || !m_ToolMenu)
			return;
		
		bool visible = m_wJournalFrame.IsVisible();
		m_wJournalFrame.SetVisible(!visible);
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(!visible);
		
		if (!visible)
			FocusOnFirstEntry();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] visibility of journal to be set.
	void SetJournalVisibility(bool visibility)
	{
		m_ToolMenuEntry.SetActive(visibility);
		m_wJournalFrame.SetVisible(visibility);
		
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(visibility);
		
		if (visibility)
			FocusOnFirstEntry();
	}

	//------------------------------------------------------------------------------------------------
	protected void GetJournalForPlayer()
	{
		m_aEntries.Clear();

		m_iCurrentEntryId = -1;

		FactionKey factionKey = FactionKey.Empty;
		if (m_PlyFactionAffilComp)
		{
			Faction plyFaction = m_PlyFactionAffilComp.GetAffiliatedFaction();
			if (plyFaction)
				factionKey = plyFaction.GetFactionKey();
		}

		SCR_JournalConfig factionJournal = m_JournalConfig.GetJournalConfig(factionKey);
		if (!factionJournal)
			return;

		array<ref SCR_JournalEntry> journalEntries = factionJournal.GetEntries();
		if (!journalEntries)
			return;
		
		Widget widget;
		SCR_MapJournalUIButton component;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		foreach (int entryId, SCR_JournalEntry entry : journalEntries)
		{	
			entry = journalEntries[entryId];
			
			widget = workspace.CreateWidgets(entry.GetEntryButtonLayout(), m_Widgets.m_wEntriesWrapper);
			component = SCR_MapJournalUIButton.Cast(widget.FindHandler(SCR_MapJournalUIButton));
			if (!component)
				continue;
			
			component.SetContent(entry.GetEntryName());
			component.SetEntry(entry, entryId);
			
			component.GetOnToggled().Insert(ShowEntry);
			m_aEntries.Insert(component);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Show the entry defined by the parameter
	//! \param[in] id index of the entry defined in the SCR_JournalConfig
	protected void ShowEntryByID(int id)
	{
		if (id < 0)
			return;
		
		SCR_MapJournalUIButton entryBtn;

		Widget child = m_Widgets.m_wEntriesWrapper.GetChildren();
		while (child)
		{
			entryBtn = SCR_MapJournalUIButton.Cast(child.FindHandler(SCR_MapJournalUIButton));
			if (!entryBtn)
				continue;

			if (entryBtn.GetId() == id)
			{
				ShowEntry(entryBtn);
				break;
			}

			child = child.GetSibling();
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	//!	Controls visuals of journal entries.
	//! \param[in] journalBtn clicked button.
	protected void ShowEntry(SCR_MapJournalUIButton journalBtn)
	{	
		if (journalBtn.GetId() == m_iCurrentEntryId)
		{	
			journalBtn.SetToggled(false);
			m_Widgets.m_wEntryLayout.SetVisible(!m_Widgets.m_wEntryLayout.IsVisible());
			m_iCurrentEntryId = -1;
		}
		else
		{
			Widget child = m_Widgets.m_wEntryLayout.GetChildren();
			if (child)
				delete child;
			
			// Toggle off last interacted button
			if (m_LastInteractedEntry)
				m_LastInteractedEntry.SetToggled(false);
			
			m_iCurrentEntryId = journalBtn.GetId();
			
			journalBtn.SetToggled(true);
			journalBtn.ShowEntry(m_Widgets.m_wEntryLayout);
			m_Widgets.m_wEntryLayout.SetVisible(true);
		}
		
		m_LastInteractedEntry = journalBtn;
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusOnFirstEntry()
	{
		if (!m_aEntries.IsEmpty() && m_aEntries[0])
			GetGame().GetWorkspace().SetFocusedWidget(m_aEntries[0].GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	bool IsVisible()
	{
		return m_wJournalFrame && m_wJournalFrame.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	void DisableMapUIComponent()
	{
		SetJournalVisibility(false);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.GetOnDisableMapUIInvoker().Remove(DisableMapUIComponent);
	}
}
