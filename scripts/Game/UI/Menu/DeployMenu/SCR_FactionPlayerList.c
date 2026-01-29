class SCR_PlayerList : ScriptedWidgetComponent
{
	[Attribute("Flag")]
	protected string m_sFlag;
	protected ImageWidget m_wFlag;
	
	[Attribute("Name")]
	protected string m_sName;
	protected TextWidget m_wName;	
	
	[Attribute("Count")]
	protected string m_sPlayerCount;
	protected TextWidget m_wPlayerCount;
	
	[Attribute("Players")]
	protected string m_sPlayerList;
	protected Widget m_wPlayerList;	
	
	[Attribute("{E49B28A2FAEBA264}UI/layouts/Menus/DeployMenu/PlayerName.layout")]
	protected ResourceName m_sPlayerName;
	
	protected ref array<SCR_PlayerName> m_aPlayerNames = {};
	protected Widget m_wRoot;

	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		m_wFlag = ImageWidget.Cast(w.FindAnyWidget(m_sFlag));
		m_wName = TextWidget.Cast(w.FindAnyWidget(m_sName));
		m_wPlayerCount = TextWidget.Cast(w.FindAnyWidget(m_sPlayerCount));
		m_wPlayerList = w.FindAnyWidget(m_sPlayerList);
	}
	
	void UpdatePlayerList()
	{
	}
	
	void ShowPlayerList(bool show)
	{
		m_wRoot.SetVisible(show);
	}
	
	protected void CreatePlayerName(int pid)
	{
		Widget player = GetGame().GetWorkspace().CreateWidgets(m_sPlayerName, m_wPlayerList);
		if (player)
		{
			SCR_PlayerName playerName = SCR_PlayerName.Cast(player.FindHandler(SCR_PlayerName));
			playerName.SetPlayer(pid);
			// playerName.SetIcon(ResourceName.Empty); // todo@lk: set some icon from somewhere
			m_aPlayerNames.Insert(playerName);
		}
	}	
};

class SCR_FactionPlayerList : SCR_PlayerList
{
	protected SCR_Faction m_Faction;
	protected SCR_SpinBoxComponent m_SpinBoxComp;
	protected int m_iLastPage;
	
	[Attribute("10", params: "0 inf", UIWidgets.EditBox, "How much entries should be shown in list.")]
	protected int m_iEntriesPerPage;
	
	[Attribute("SpinBox")]
	protected string m_sSpinBoxElementName;
	
	[Attribute("ButtonLeft")]
	protected string m_sPagingButtonLeft;
	
	[Attribute("ButtonRight")]
	protected string m_sPagingButtonRight;
	
	//------------------------------------------------------------------------------------------------
	override event void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		factionManager.GetOnPlayerFactionCountChanged().Insert(UpdatePagination);
		factionManager.GetOnPlayerFactionCountChanged().Insert(UpdatePlayerList);
		
		Widget spinboxW = m_wRoot.FindAnyWidget(m_sSpinBoxElementName);
		if (!spinboxW)
			return;
		
		m_SpinBoxComp = SCR_SpinBoxComponent.Cast(spinboxW.FindHandler(SCR_SpinBoxComponent));
		if (!m_SpinBoxComp)
			return;
		
		m_SpinBoxComp.m_OnChanged.Insert(UpdatePlayerList);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFaction(Faction faction)
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return;
		
		m_Faction = scrFaction;

		if (m_wFlag && !scrFaction.GetFactionFlag().IsEmpty())
			m_wFlag.LoadImageTexture(0, scrFaction.GetFactionFlag());

		if (m_wName)
			m_wName.SetText(scrFaction.GetFactionName());
		
		UpdatePagination(faction);
		UpdatePlayerList();
		
		//Reset last page as faction was changed
		m_iLastPage = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePagination(Faction faction)
	{
		if (faction != m_Faction)
			return;
		
		array<int> players = {};
		m_Faction.GetPlayersInFaction(players);
		
		int pageCount = players.Count() / m_iEntriesPerPage;
		if ((players.Count() % m_iEntriesPerPage) > 0)
			pageCount++;

		SetLastIndex();
		m_SpinBoxComp.ClearAll();
		
		for (int i = 0; i < pageCount; i++)
		{
			m_SpinBoxComp.AddItem("", i == pageCount - 1);
		}
		
		m_SpinBoxComp.SetCurrentItem(m_iLastPage);
		
		//Show or disable navigation buttons depending on number of visible pagging entries
		HandleNavigationButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get index of first entry of current page
	protected int GetStartingIndex()
	{
		int page = m_SpinBoxComp.GetCurrentIndex();
		if (page < 1)
			return 0;
		
		return page * m_iEntriesPerPage;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleNavigationButtons()
	{
		if (!m_SpinBoxComp)
			return;
		
		bool showButtons = m_SpinBoxComp.GetNumItems() > 1;
		
		Widget button = m_wRoot.FindAnyWidget(m_sPagingButtonLeft);
		if (button)
			button.SetVisible(showButtons);
		
		button = m_wRoot.FindAnyWidget(m_sPagingButtonRight);
		if (button)
			button.SetVisible(showButtons);
		
		Widget spinboxW = m_wRoot.FindAnyWidget(m_sSpinBoxElementName);
		if (spinboxW)
			spinboxW.SetVisible(showButtons);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetLastIndex()
	{
		m_iLastPage = m_SpinBoxComp.GetCurrentIndex();
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdatePlayerList()
	{
		if (!m_Faction || !m_wPlayerList)
			return;

		m_aPlayerNames.Clear();

		Widget child = m_wPlayerList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}

		array<int> players = {};
		m_Faction.GetPlayersInFaction(players);
		
		int startIndex = GetStartingIndex();
		if (players.IsEmpty() || startIndex < 0)
		{
			if (m_wPlayerCount)
				m_wPlayerCount.SetText("0");
			return;
		}

		int maxIndex = startIndex + (m_iEntriesPerPage-1);
		for (int i = startIndex; i <= maxIndex; i++)
		{	
			if (i >= players.Count())
				break;
			
			if (m_Faction && (SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(players[i])) == m_Faction))
				CreatePlayerName(players[i]);
		}

		if (m_wPlayerCount)
			m_wPlayerCount.SetText(players.Count().ToString());
	}
	
	override event void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			factionManager.GetOnPlayerFactionCountChanged().Remove(UpdatePagination);
			factionManager.GetOnPlayerFactionCountChanged().Remove(UpdatePlayerList);		
		}
	}
};

class SCR_GroupPlayerList : SCR_PlayerList
{
	protected SCR_AIGroup m_Group;
	
	void SetGroup(SCR_AIGroup group)
	{
		if (!group)
			return;
		
		m_Group = group;

		if (m_wFlag && !group.GetGroupFlag().IsEmpty())
			m_wFlag.LoadImageTexture(0, group.GetGroupFlag());

		if (m_wName)
		{
			string name = m_Group.GetCustomName();
			if (name.IsEmpty())
			{
				string company, platoon, squad, character, format;
				m_Group.GetCallsigns(company, platoon, squad, character, format);
				name = string.Format("%1 %2 %3 %4 %5", company, platoon, squad, character, format);
			}

			m_wName.SetText(name);
		}

		UpdatePlayerList();
	}
	
	override void UpdatePlayerList()
	{
		if (!m_Group || !m_wPlayerList)
			return;

		m_aPlayerNames.Clear();

		Widget child = m_wPlayerList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}

		array<int> players = m_Group.GetPlayerIDs();
		foreach (int pid : players)
		{
			CreatePlayerName(pid);
		}

		if (m_wPlayerCount)
			m_wPlayerCount.SetText(m_aPlayerNames.Count().ToString());		
	}
};

class SCR_PlayerName : ScriptedWidgetComponent
{
	[Attribute("Icon")]
	protected string m_sIcon;
	protected ImageWidget m_wIcon;

	[Attribute("Name")]
	protected string m_sName;
	protected TextWidget m_wName;
	
	[Attribute("BadgeWrapper")]
	protected string m_sBadgeWrapper;
	protected SizeLayoutWidget m_wBadgeWrapper;

	protected ResourceName m_sIcons = "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-32.imageset";
	protected int m_iPlayerId;


	override void HandlerAttached(Widget w)
	{
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(m_sIcon));
		m_wName = TextWidget.Cast(w.FindAnyWidget(m_sName));
		m_wBadgeWrapper = SizeLayoutWidget.Cast(w.FindAnyWidget(m_sBadgeWrapper));
		SetBadgeColor(m_wBadgeWrapper);
	}

	//------------------------------------------------------------------------------------------------
	void SetIcon(ResourceName icon)
	{
		if (!m_wIcon)
			return;

		if (m_wIcon)
			m_wIcon.SetVisible(m_wIcon.LoadImageTexture(0, icon));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] badgeWrapper Widget that holds the badge widget, used to set its color to the player's faction color.
	void SetBadgeColor(SizeLayoutWidget badgeWrapper)
	{
		SCR_FactionManager m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		if (!badgeWrapper || !m_FactionManager)
			return;
		
		Faction faction = m_FactionManager.GetLocalPlayerFaction();
		if (!faction)
			return;

		badgeWrapper.SetColor(faction.GetFactionColor());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] pid
	void SetPlayer(int pid)
	{
		m_iPlayerId = pid;
		if (m_wName)
			m_wName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(pid));
			
		SetPlatform();
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_iPlayerId;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPlatform()
	{
		if (!m_wIcon)
			return;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		playerController && playerController.SetPlatformImageTo(m_iPlayerId, m_wIcon, showOnPC: true, showOnXbox: true)

		// todo@lk: local profile pic
	}
};
