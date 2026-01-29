class SCR_MapMarkerSquadLeaderComponent : SCR_MapMarkerDynamicWComponent
{
	bool m_bIsHovered;
	protected bool m_bIsInit;
	protected Widget m_wOwnSquadBackground;
	protected Widget m_wOwnSquadIcon;
	protected Widget m_wOwnSquadIconGlow;
	protected Widget m_wGroupInfo;
	protected Widget m_wGroupInfoList;
	protected TextWidget m_wGroupFrequency;
	
	protected ref array<Widget> m_aGroupMemberEntries = {};
	
	[Attribute("{CCD81F58E9D6EEA6}UI/layouts/Map/MapMarkerGroupInfo.layout", desc: "group info layout")]
	protected ResourceName m_sGroupInfoLayout;
	
	[Attribute("{B09864CA15145CD3}UI/layouts/Map/MapMarkerGroupInfoLine.layout", desc: "group info line layout")]
	protected ResourceName m_sLineLayout;
		
	[Attribute("lineText", desc: "line text widget")]
	protected string m_sLineTextWidgetName;
	
	[Attribute("lineIcon", desc: "line icon widget")]
	protected string m_sLineIconWidgetName;
	
	[Attribute("groupCohesionIcon", desc: "group cohesion icon widget")]
	protected string m_sGroupCohesionWidgetName;
	
	[Attribute("PlatformIcon")]
	protected string m_sPlatformIconWidgetName;
	
	[Attribute("40", desc: "pixels, group info offset")]
	protected int m_iGroupInfoOffset;
		
	protected ref array<int> m_aDisplayedPlayerIDs = {};

	//------------------------------------------------------------------------------------------------
	//! Differentiates visuals between our group and the others
	//! \param[in] state
	void SetGroupActive(bool state, string factionName = string.Empty)
	{
		Color iconColor = Color.FromInt(Color.BLACK);
		
		if (state)
		{
			/*m_wOwnSquadBackground.SetVisible(true);
			m_wOwnSquadIcon.SetVisible(true);
			m_wOwnSquadIconGlow.SetVisible(true);
			
			// TODO temp set until these are added to configs
			if (factionName == "US")
				iconColor = new Color(0.0, 0.18, 0.61, 1.0);
			else if (factionName == "USSR")
				iconColor = new Color(0.51, 0.02, 0.02, 1.0);*/
		}
		else 
		{
			m_wOwnSquadBackground.SetVisible(false);
			m_wOwnSquadIcon.SetVisible(false);
			m_wOwnSquadIconGlow.SetVisible(false);
		}
		
		//m_wOwnSquadIcon.SetColor(iconColor);
		//m_wOwnSquadIconGlow.SetColor(iconColor);
	}
			
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] screenX
	//! \param[in] screenY
	void UpdateGroupInfoPosition(int screenX, int screenY)
	{
		if (m_wGroupInfo)
			FrameSlot.SetPos(m_wGroupInfo, GetGame().GetWorkspace().DPIUnscale(screenX), GetGame().GetWorkspace().DPIUnscale(screenY) - m_iGroupInfoOffset);	// needs unscaled coords
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button != 0)	// LMB only
			return true;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		SCR_MapCursorModule cursorModule;
		if (mapEntity)
			cursorModule = SCR_MapCursorModule.Cast(mapEntity.GetMapModule(SCR_MapCursorModule));

		if (cursorModule && cursorModule.GetCursorState() & SCR_MapCursorModule.STATE_POPUP_RESTRICTED)
			return true;
		
		SCR_AIGroup group = SCR_MapMarkerSquadLeader.Cast(m_MarkerEnt).GetGroup();
		if (!group)
			return false;

		SCR_PlayerControllerGroupComponent playerControllerGroup = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (playerControllerGroup)
			playerControllerGroup.SetSelectedGroupID(group.GetGroupID());

		GetGame().OpenGroupMenu();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{		
		m_MarkerEnt.LayerChangeLogic(0);
		
		SCR_AIGroup group = SCR_MapMarkerSquadLeader.Cast(m_MarkerEnt).GetGroup();
		if (group)
		{
			if (!m_bIsInit)
			{
				m_wGroupInfo = GetGame().GetWorkspace().CreateWidgets(m_sGroupInfoLayout, m_wRoot.GetParent());
				if (!m_wGroupInfo)
					return false;
				
				m_wGroupInfoList = m_wGroupInfo.FindAnyWidget("groupInfoList");
				m_wGroupFrequency = TextWidget.Cast(m_wGroupInfo.FindAnyWidget("groupFrequency"));
				
				int capacity = group.GetMaxMembers();
				
				for (int i = 0; i < capacity; i++)
				{
					m_aGroupMemberEntries.Insert(GetGame().GetWorkspace().CreateWidgets(m_sLineLayout, m_wGroupInfoList));
				}
				
				m_bIsInit = true;
			}
			
			float fFrequency = Math.Round(group.GetRadioFrequency() * 0.1) * 0.01; 	// Format the frequency text: round and convert to 2 digits with one possible decimal place (39500 -> 39.5)			
			m_wGroupFrequency.SetText(fFrequency.ToString(3, 1));
			
			int playerCount = group.GetPlayerCount();
			array<int> membersCopy = {};
			membersCopy.Copy(group.GetPlayerIDs());
			
			PlayerManager pManager = GetGame().GetPlayerManager();
			int leaderID = group.GetLeaderID();
			
			// leader entry first, order of IDs is not guaranteed
			foreach (int id : membersCopy)
			{
				if (id == leaderID)
				{
					Widget entry = m_aGroupMemberEntries[0];
					TextWidget txtW = TextWidget.Cast(entry.FindWidget(m_sLineTextWidgetName));
					txtW.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(id));
					entry.SetVisible(true);
					
					ImageWidget platformImage= ImageWidget.Cast(entry.FindAnyWidget(m_sPlatformIconWidgetName));
					if (platformImage)
						SCR_PlayerController.Cast(GetGame().GetPlayerController()).SetPlatformImageTo(id, platformImage);
					
					if (GetGame().GetPlayerController().GetPlayerId() == id)
						txtW.SetColor(GUIColors.ORANGE);
					else 
						txtW.SetColor(GUIColors.DEFAULT);
					
					ImageWidget imgW = ImageWidget.Cast(entry.FindWidget(m_sLineIconWidgetName));
					imgW.SetOpacity(1);
					
					membersCopy.RemoveItem(id);
					break;	
				}
			}
			
			// members
			foreach (int i, Widget entry :  m_aGroupMemberEntries) 
			{
				if (i == 0)		// leader
					continue;
				
				if (i < playerCount)
				{
					TextWidget txtW = TextWidget.Cast(entry.FindWidget(m_sLineTextWidgetName));
					txtW.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(membersCopy[i-1]));
					entry.SetVisible(true);
					
					ImageWidget platformImage= ImageWidget.Cast(entry.FindAnyWidget(m_sPlatformIconWidgetName));
					if (platformImage)
						SCR_PlayerController.Cast(GetGame().GetPlayerController()).SetPlatformImageTo(membersCopy[i-1], platformImage);
					
					if (GetGame().GetPlayerController().GetPlayerId() == membersCopy[i-1])
						txtW.SetColor(GUIColors.ORANGE);
					else 
						txtW.SetColor(GUIColors.DEFAULT);
					
					ImageWidget imgW = ImageWidget.Cast(entry.FindWidget(m_sLineIconWidgetName));
					imgW.SetOpacity(0);
					
					continue;
				} 
				
				entry.SetVisible(false);
			}
			
			m_wGroupInfo.SetVisible(true);

			m_aDisplayedPlayerIDs.Clear();
			m_aDisplayedPlayerIDs.Insert(leaderID);
			foreach (int id : membersCopy)
			{
				m_aDisplayedPlayerIDs.Insert(id);
			}

			SCR_AIGroupCohesionComponent cohesionComponent = SCR_AIGroupCohesionComponent.Cast(group.FindComponent(SCR_AIGroupCohesionComponent));
			if (cohesionComponent) 
			{
				array<int> cluster = {};
				cohesionComponent.GetPlayersInCohesion(cluster);
				OnClusterUpdated(cluster);
			}
		}
		
		m_bIsHovered = true;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClusterUpdated(notnull array<int> cluster)
	{
		array<int> players = SCR_MapMarkerSquadLeader.Cast(m_MarkerEnt).GetGroup().GetPlayerIDs();
		if (!players || players.Count() == 0)
			return;

		foreach (int i, int playerID : m_aDisplayedPlayerIDs)
		{
			int playerId = m_aDisplayedPlayerIDs[i];
			ImageWidget cohesionImage = ImageWidget.Cast(m_aGroupMemberEntries[i].FindAnyWidget(m_sGroupCohesionWidgetName));
			if (!cohesionImage)
				continue;

			if (cluster.Contains(playerId))
				cohesionImage.SetColor(Color.FromInt(Color.WHITE));
			else
				cohesionImage.SetColor(Color.FromInt(Color.GRAY_25));
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_wGroupInfo.SetVisible(false);
		m_bIsHovered = false;
		
		m_MarkerEnt.LayerChangeLogic(m_iLayerID);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wOwnSquadBackground = m_wRoot.FindAnyWidget("OwnSquadBackground");
		m_wOwnSquadIcon = m_wRoot.FindAnyWidget("OwnSquadIcon");
		m_wOwnSquadIconGlow = m_wRoot.FindAnyWidget("OwnSquadIconGlow");
	}
}
