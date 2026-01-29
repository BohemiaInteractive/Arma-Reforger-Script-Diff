/*!
	Component responsible for displaying list of available factions and handling faction assign requests
	Fills m_wFactionList with m_sFactionButton widgets
*/
class SCR_FactionRequestUIComponent : SCR_DeployRequestUIBaseComponent
{
	[Attribute("FactionList", desc: "Container for available factions' buttons")]
	protected string m_sFactionList;
	protected Widget m_wFactionList;

	[Attribute("NoFactions")]
	protected string m_sNoFactions;
	protected Widget m_wNoFactions;

	[Attribute("{8A77FAE1C3B1F827}UI/layouts/Menus/DeployMenu/FactionButton.layout", desc: "Layout for faction button, has to have SCR_FactionButton attached to it.")]
	protected ResourceName m_sFactionButton;

	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;
	protected SCR_FactionManager m_FactionManager;
	
	protected ref ScriptInvoker<SCR_PlayerFactionAffiliationComponent, int, bool> m_OnFactionRequested;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wFactionList = w.FindAnyWidget(m_sFactionList);
		if (!m_wFactionList)
			return;

		m_wNoFactions = w.FindAnyWidget(m_sNoFactions);

		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (m_FactionManager)
			m_FactionManager.GetOnPlayerFactionCountChanged().Insert(UpdateFactionButtons);
		
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
			m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));

		ShowAvailableFactions();
	}

	//------------------------------------------------------------------------------------------------
	//! Display available playable factions in the list.
	void ShowAvailableFactions()
	{
		if (!m_wFactionList || !m_PlyFactionAffilComp)
			return;

		// Clear the list
		Widget child = m_wFactionList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			if (SCR_FactionButton.Cast(child.FindHandler(SCR_FactionButton))) // Don't clear widgets that are not SCR_FactionButton
				child.RemoveFromHierarchy();
			child = sibling;
		}

		m_aButtons.Clear();

		// fetch factions and create their button layouts
		array<Faction> availableFactions = {};
		int factionCount = m_FactionManager.GetFactionsList(availableFactions);

		Faction playerFaction = m_PlyFactionAffilComp.GetAffiliatedFaction();
		bool canChangeFaction = SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsFactionChangeAllowed();

		int playableFactionCount;
		int factionPlayerLimit = -1;

		for (int i = 0; i < factionCount; ++i)
		{
			SCR_Faction scriptedFaction = SCR_Faction.Cast(availableFactions[i]);
			if (!scriptedFaction)
				continue;
			
			scriptedFaction.GetOnFactionPlayableChanged().Insert(OnPlayableFactionChanged);

			Widget btnW = GetGame().GetWorkspace().CreateWidgets(m_sFactionButton, m_wFactionList);

			SCR_FactionButton btnComp;
			if (m_bUseListFromButton)
				btnComp = SCR_FactionButton.Cast(btnW.FindAnyWidget("Button").FindHandler(SCR_FactionButton));
			else
				btnComp = SCR_FactionButton.Cast(btnW.FindHandler(SCR_FactionButton));
			
			btnComp.SetFaction(scriptedFaction);
			btnComp.m_OnClicked.Insert(OnButtonClicked);
			btnComp.m_OnFocus.Insert(OnButtonFocused);
			btnComp.m_OnMouseEnter.Insert(OnButtonFocused);
			btnComp.m_OnMouseLeave.Insert(OnMouseLeft);
			
			if (!canChangeFaction && playerFaction)
			{
				btnComp.SetShouldUnlock(playerFaction == scriptedFaction);
				btnComp.SetEnabled(playerFaction == scriptedFaction);
			}

			factionPlayerLimit = scriptedFaction.GetPlayerLimit();
			if (factionPlayerLimit >= 0 && scriptedFaction.GetPlayerCount() >= factionPlayerLimit)
			{
				btnComp.SetShouldUnlock(playerFaction == scriptedFaction);
				btnComp.SetEnabled(playerFaction == scriptedFaction);
			}

			btnComp.SetVisible(scriptedFaction.IsPlayable(), false);

			if (scriptedFaction.IsPlayable())
				playableFactionCount++;

			m_aButtons.Insert(btnComp);
		}

		if (m_wNoFactions)
			m_wNoFactions.SetVisible(playableFactionCount == 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when local player's faction is assigned.
	void OnPlayerFactionAssigned(Faction assignedFaction)
	{
		bool canChangeFaction = SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsFactionChangeAllowed();

		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_FactionButton factionBtn = SCR_FactionButton.Cast(btn);
			if (factionBtn)
			{
				if (!canChangeFaction)
				{
					bool enabled = assignedFaction == factionBtn.GetFaction();
					factionBtn.m_OnClicked.Remove(OnButtonClicked);
					factionBtn.SetEnabled(enabled);
					factionBtn.SetShouldUnlock(enabled);
				}
				
				if (assignedFaction == factionBtn.GetFaction())
					factionBtn.HideTooltip();
				factionBtn.SetSelected(assignedFaction == factionBtn.GetFaction());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when playable factions are changed.
	protected void OnPlayableFactionChanged(SCR_Faction faction, bool playable)
	{
		int playableFactionCount = 0;
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_FactionButton factionBtn = SCR_FactionButton.Cast(btn);
			if (!factionBtn)
				continue;
			if (factionBtn.GetFaction() == faction)
				factionBtn.SetVisible(playable, false);
			if (factionBtn.GetFaction().IsPlayable())
				playableFactionCount++;
		}
		
		if (m_wNoFactions)
			m_wNoFactions.SetVisible(playableFactionCount == 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Update all available faction buttons.
	protected void UpdateFactionButtons(Faction faction, int newCount)
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_FactionButton factionBtn = SCR_FactionButton.Cast(btn);
			if (factionBtn)
				factionBtn.UpdatePlayerCount();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when button is focused.
	protected void OnButtonFocused(Widget w)
	{
		SCR_FactionButton factionBtn = SCR_FactionButton.Cast(w.FindHandler(SCR_FactionButton));
		if (!factionBtn)
			return;

		if (m_PlyFactionAffilComp)
			factionBtn.SetTooltipAvailable(m_PlyFactionAffilComp.GetAffiliatedFaction() != factionBtn.GetFaction());
		m_OnButtonFocused.Invoke(factionBtn.GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	//! Sends a request to assign a faction.
	protected void RequestPlayerFaction(SCR_FactionButton factionBtn)
	{
		if (m_PlyFactionAffilComp.GetAffiliatedFaction() == factionBtn.GetFaction())
		{
			GetOnFactionRequested().Invoke(m_PlyFactionAffilComp, m_FactionManager.GetFactionIndex(m_PlyFactionAffilComp.GetAffiliatedFaction()), true);
			return;
		}

		Lock(factionBtn);
		Faction faction = factionBtn.GetFaction();
		m_PlyFactionAffilComp.RequestFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when player clicks the faction button.
	protected void OnButtonClicked(SCR_FactionButton factionBtn)
	{
		if (SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsFactionChangeAllowed())
		{
			RequestPlayerFaction(factionBtn);
		}	
		else if (m_FactionManager)
		{
			int factionIndex = m_FactionManager.GetFactionIndex(factionBtn.GetFaction());
			SCR_PersistentFactionDialog dialog = SCR_PersistentFactionDialog.CreatePersistentFactionDialog(factionIndex);
			dialog.m_OnConfirm.Insert(OnFactionDialogConfirm);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when player closes the persistent faction dialog.
	protected void OnFactionDialogConfirm(SCR_PersistentFactionDialog dialog)
	{
		SCR_FactionButton btn = GetFactionButton(dialog.GetRequestedFactionIndex());
		if (btn)
			RequestPlayerFaction(btn);
	}

	//------------------------------------------------------------------------------------------------
	void RequestRandomFaction()
	{
		array<Faction> factions = {};
		m_FactionManager.GetFactionsList(factions);
		m_PlyFactionAffilComp.RequestFaction(factions.GetRandomElement());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the faction button associated with a faction index.
	\return faction button
	*/
	SCR_FactionButton GetFactionButton(int factionId)
	{
		Faction faction = m_FactionManager.GetFactionByIndex(factionId);
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_FactionButton factionBtn = SCR_FactionButton.Cast(btn);
			if (factionBtn && factionBtn.GetFaction() == faction)
				return factionBtn;
		}

		return null;
	}

	ScriptInvoker GetOnFactionRequested()
	{
		if (!m_OnFactionRequested)
			m_OnFactionRequested = new ScriptInvoker();
		
		return m_OnFactionRequested;
	}
};

class SCR_FactionButton : SCR_DeployButtonBase
{
	[Attribute("FactionName")]
	protected string m_sFactionName;
	protected TextWidget m_wFactionName;

	[Attribute("PlayerCountText")]
	protected string m_sPlayerCount;
	protected TextWidget m_wPlayerCount;

	[Attribute("PlayerIcon")]
	protected string m_sPlayerIcon;
	protected Widget m_wPlayerIcon;
	
	[Attribute("Arrow")]
	protected string m_sArrowIcon;
	protected ImageWidget m_wArrowIcon;

	protected SCR_Faction m_Faction;
	protected int m_iPlayerCount;
	protected int m_iPlayerLimit;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wFactionName = TextWidget.Cast(w.FindAnyWidget(m_sFactionName));
		m_wPlayerCount = TextWidget.Cast(w.FindAnyWidget(m_sPlayerCount));
		m_wPlayerIcon = ImageWidget.Cast(w.FindAnyWidget(m_sPlayerIcon));
		m_wArrowIcon = ImageWidget.Cast(w.FindAnyWidget(m_sArrowIcon));
	}

	//------------------------------------------------------------------------------------------------
	//! Set this button's faction.
	void SetFaction(SCR_Faction faction)
	{
		m_Faction = faction;

		SetFactionName(faction.GetFactionName());
		SetImage(faction.GetFactionFlag());

		UpdatePlayerCount();
	}

	//------------------------------------------------------------------------------------------------
	//! Get this button's faction.
	SCR_Faction GetFaction()
	{
		return m_Faction;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the player count of the faction.
	int GetPlayerCount()
	{
		return m_iPlayerCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the player limit of the faction.
	int GetPlayerLimit()
	{
		return m_iPlayerLimit;
	}

	//------------------------------------------------------------------------------------------------
	//! Update faction player count of this button.
	void UpdatePlayerCount()
	{
		if (m_Faction && m_wPlayerCount)
		{
			m_iPlayerCount = m_Faction.GetPlayerCount();
			m_iPlayerLimit = m_Faction.GetPlayerLimit();
			if (m_iPlayerLimit >= 0)
				m_wPlayerCount.SetTextFormat("#AR-SupportStation_ActionFormat_ItemAmount", m_iPlayerCount, m_iPlayerLimit);
			else
				m_wPlayerCount.SetText(m_iPlayerCount.ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set faction name of this button.
	void SetFactionName(string name)
	{
		if (m_wFactionName)
			m_wFactionName.SetText(name);
	}

	//------------------------------------------------------------------------------------------------
	//! Visually set the button selected.
	override void SetSelected(bool selected)
	{
		super.SetSelected(selected);
		if (!m_wArrowIcon)
			return;
		
		if (selected)
			m_wArrowIcon.SetRotation(90);
		else
			m_wArrowIcon.SetRotation(270);
	}	
};
