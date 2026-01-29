
	//------------------------------------------------------------------------------------------------
	//!
	//! \return

	//------------------------------------------------------------------------------------------------
	//!
	//! \return

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
//! Component responsible for requesting and visualization of available loadouts in deploy menu.
class SCR_LoadoutRequestUIComponent : SCR_DeployRequestUIBaseComponent
{
	[Attribute("LoadoutList", desc: "List for available player's loadouts' buttons")]
	protected string m_sLoadoutList;
	protected GridLayoutWidget m_wLoadoutList;

	[Attribute("LoadoutPreview")]
	protected string m_sLoadoutPreview;
	protected Widget m_wLoadoutPreview;

	[Attribute("LoadoutIcon")]
	protected string m_sLoadoutIcon;
	protected ImageWidget m_wLoadoutIcon;

	[Attribute("LoadoutName")]
	protected string m_sLoadoutName;
	protected TextWidget m_wLoadoutName;

	[Attribute("Dawn")]
	protected string m_sDTDawn;
	
	[Attribute("Forenoon")]
	protected string m_sDTForenoon;
	
	[Attribute("Afternoon")]
	protected string m_sDTAfternoon;
	
	[Attribute("Dusk")]
	protected string m_sDTDusk;
	
	[Attribute("Evening")]
	protected string m_sDTEvening;
		
	[Attribute("Night")]
	protected string m_sDTNight;
	
	[Attribute("#AR-Loadout_Free_Supply_Cost")]
	protected string m_sFreeLoadout;
	
	protected ref array<string> m_sDayModifierStrings = { m_sDTDawn, m_sDTForenoon, m_sDTAfternoon, m_sDTDusk, m_sDTEvening, m_sDTNight };
	
	[Attribute("{39D815C843414C76}UI/layouts/Menus/DeployMenu/LoadoutButton.layout", desc: "Layout for loadout button, has to have SCR_LoadoutButton attached to it.")]
	protected ResourceName m_sLoadoutButton;

	[Attribute("Selector")]
	protected string m_sLoadoutSelector;
	protected Widget m_wLoadoutSelector;

	[Attribute("LoadoutSelectorRoot")]
	protected string m_sRoot;

	protected SCR_LoadoutGallery m_LoadoutSelector;

	protected SCR_LoadoutManager m_LoadoutManager;
	protected SCR_PlayerLoadoutComponent m_PlyLoadoutComp;
	protected SCR_LoadoutPreviewComponent m_PreviewComp;
	protected SCR_ArsenalManagerComponent m_ArsenalManagerComp;
	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;
	protected SCR_PlayerControllerGroupComponent m_PlayerControllerGroupComponent;
	protected SCR_PlayerSupplyAllocationUI m_PlayerSupplyAllocationUI;
	protected IEntity m_PreviewedEntity;

	protected Widget m_wLoadouty;
	protected Widget m_wSupplies;
	protected Widget m_wSuppliesWrapper;
	protected ResourceName m_sLoadoutImageSet;
	protected RichTextWidget m_wSuppliesText;
	protected RichTextWidget m_wLoadoutNameText;
	protected Widget m_wMSARSupplies;
	protected string m_sMSARSuppliesName = "MSAR_Supplies";

	protected const int LOADOUTS_PER_ROW = 2;
	
	protected ref ScriptInvokerInt m_OnPlayerEntryFocused;
	protected ref ScriptInvokerWidget m_OnPlayerEntryFocusLost;

	protected SCR_UIInfo uiInfo;
	protected string m_sLoadoutImageName;
	protected string m_sDayModifier;
	protected string m_sWeatherModifier;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w.FindAnyWidget(m_sRoot);

		m_LoadoutManager = GetGame().GetLoadoutManager();
		if (!m_LoadoutManager)
		{
			Print("Loadout manager is missing in the world! Deploy menu won't work correctly.", LogLevel.ERROR);
			return;	
		}

		if (m_LoadoutManager)
			m_LoadoutManager.GetOnMappedPlayerLoadoutInfoChanged().Insert(OnLoadoutsChanged); // todo@lk: update loadout list
		
		m_wLoadoutIcon = ImageWidget.Cast(w.FindAnyWidget(m_sLoadoutIcon));
		m_wLoadoutName = TextWidget.Cast(w.FindAnyWidget(m_sLoadoutName));
		m_wLoadoutList = GridLayoutWidget.Cast(w.FindAnyWidget(m_sLoadoutList));

		m_wLoadouty = w.FindAnyWidget("Loadouty");
		
		m_wExpandButtonName = TextWidget.Cast(w.FindAnyWidget(m_sExpandButtonName));
		m_wExpandButtonIcon = ImageWidget.Cast(w.FindAnyWidget(m_sExpandButtonIcon));
		m_wExpandButton = w.FindAnyWidget(m_sExpandButton);

		if (SCR_ArsenalManagerComponent.GetArsenalManager(m_ArsenalManagerComp))
		{
			m_ArsenalManagerComp.GetOnLoadoutUpdated().Insert(UpdateLoadouts);
			m_ArsenalManagerComp.GetOnLoadoutSpawnSupplyCostMultiplierChanged().Insert(OnLoadoutSpawnCostMultiplierChanged);
		}
			
		if (m_wExpandButton && m_wExpandButton.IsVisible())
		{
			SCR_ButtonBaseComponent expandBtn = SCR_ButtonBaseComponent.Cast(m_wExpandButton.FindHandler(SCR_ButtonBaseComponent));
			expandBtn.m_OnClicked.Insert(ToggleCollapsed);
			GetOnListCollapse().Insert(OnListExpand);
		}		
		
		m_wLoadoutSelector = w.FindAnyWidget(m_sLoadoutSelector);
		if (m_wLoadoutSelector)
			m_LoadoutSelector = SCR_LoadoutGallery.Cast(m_wLoadoutSelector.FindHandler(SCR_LoadoutGallery));

		if (m_LoadoutSelector)
		{
			m_LoadoutSelector.GetOnLoadoutClicked().Insert(OnRequestPlayerLoadout);
			m_LoadoutSelector.GetOnLoadoutHovered().Insert(SetLoadoutPreview);
		}

		m_wLoadoutPreview = w.FindAnyWidget(m_sLoadoutPreview);
		if (m_wLoadoutPreview)
			m_PreviewComp = SCR_LoadoutPreviewComponent.Cast(m_wLoadoutPreview.FindHandler(SCR_LoadoutPreviewComponent));

		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			m_PlyLoadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
			m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (GetPlayerLoadout())
			{
				if (m_wExpandButtonName)
					m_wExpandButtonName.SetText(GetPlayerLoadout().GetLoadoutName());	
				SetLoadoutPreview(GetPlayerLoadout());
			}
		}

		if (m_wLoadouty)
			m_wSupplies = m_wLoadouty.FindAnyWidget("w_Supplies");
		if (m_wSupplies)
		{
			m_wSuppliesText = RichTextWidget.Cast(m_wSupplies.FindAnyWidget("SuppliesText"));
			m_wLoadoutNameText = RichTextWidget.Cast(m_wSupplies.FindAnyWidget("LoadoutNameText"));
		}

		m_wMSARSupplies = w.FindAnyWidget(m_sMSARSuppliesName);
		if (m_wMSARSupplies)
			m_PlayerSupplyAllocationUI = SCR_PlayerSupplyAllocationUI.Cast(m_wMSARSupplies.FindHandler(SCR_PlayerSupplyAllocationUI));

		m_PlayerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (m_PlayerControllerGroupComponent)
			m_PlayerControllerGroupComponent.GetOnGroupChanged().Insert(OnGroupChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_ArsenalManagerComp)
		{
			m_ArsenalManagerComp.GetOnLoadoutUpdated().Remove(UpdateLoadouts);
			m_ArsenalManagerComp.GetOnLoadoutSpawnSupplyCostMultiplierChanged().Remove(OnLoadoutSpawnCostMultiplierChanged);
		}

		if (m_PlayerControllerGroupComponent)
			m_PlayerControllerGroupComponent.GetOnGroupChanged().Remove(OnGroupChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void ToggleCollapsed()
	{
		if (m_wExpandButton && m_wExpandButton.IsVisible())
		{
			bool visible = !IsExpanded();
			SetExpanded(visible);
			GetOnListCollapse().Invoke(this, visible);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetExpanded(bool expanded)
	{
		m_wLoadouty.SetVisible(expanded);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool IsExpanded()
	{
		return m_wLoadouty.IsVisible();
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadoutsChanged(SCR_BasePlayerLoadout loadout, int newCount)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Called from deploy menu when player's loadout is assigned.
	void OnPlayerLoadoutAssigned(SCR_PlayerLoadoutComponent component)
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(btn);
			if (loadoutBtn && loadoutBtn.GetPlayerId() == component.GetPlayerId())
			{
				loadoutBtn.SetLoadout(component.GetLoadout());
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupChanged(int groupId)
	{
		if (!m_PlyFactionAffilComp)
			return;

		ShowAvailableLoadouts(m_PlyFactionAffilComp.GetAffiliatedFaction());
	}

	//------------------------------------------------------------------------------------------------
	//! Show available loadouts in the loadout selector.
	void ShowAvailableLoadouts(Faction faction)
	{
		if (!m_LoadoutManager)
			return;

		ResetPlayerLoadoutPreview();

		if (!m_LoadoutSelector)
			return;

		m_LoadoutSelector.ClearAll();

		array<ref SCR_BasePlayerLoadout> availableLoadouts = {};

		PlayerController pc = GetGame().GetPlayerController();
		SCR_PlayerControllerGroupComponent pcGroupComponent = SCR_PlayerControllerGroupComponent.Cast(pc.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!pcGroupComponent)
			return;

		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return;

		SCR_AIGroup group = pcGroupComponent.GetPlayersGroup();
		if (group && scrFaction.IsGroupRolesConfigured())
		{
			m_LoadoutManager.GetPlayerLoadoutsByGroup(group, faction, availableLoadouts);
		}
		else
		{
			// default behavior
			m_LoadoutManager.GetPlayerLoadoutsByFaction(faction, availableLoadouts);
		}

		SCR_PlayerArsenalLoadout arsenalLoadout = null;
		foreach (SCR_BasePlayerLoadout loadout : availableLoadouts)
		{
			if (!loadout.IsLoadoutAvailableClient())
				continue;

			if (loadout.IsInherited(SCR_PlayerArsenalLoadout))
				arsenalLoadout = SCR_PlayerArsenalLoadout.Cast(loadout);

			m_LoadoutSelector.AddItem(loadout, loadout.IsLoadoutAvailableClient());
		}
		
		if (!availableLoadouts.IsEmpty())
		{
			SCR_BasePlayerLoadout selectedLoadout = GetPlayerLoadout();

			if (arsenalLoadout && selectedLoadout != arsenalLoadout)
			{
				m_PlyLoadoutComp.RequestLoadout(arsenalLoadout);
			}
			else if (!selectedLoadout || !availableLoadouts.Contains(selectedLoadout))
			{
				// need request every time when group is changed
				foreach (SCR_BasePlayerLoadout loadout : availableLoadouts)
				{
					if (loadout.IsLoadoutAvailableClient())
					{
						m_PlyLoadoutComp.RequestLoadout(loadout);
						break;
					}
				}
			}
		}

		GetGame().GetCallqueue().CallLater(RefreshLoadoutPreview, 0, false); // delayed refresh call helps with performance
	}

	//------------------------------------------------------------------------------------------------
	//! Fill the loadout list with players' loadouts.
	void ShowPlayerLoadouts(array<int> playerIds, int slotCount = -1)
	{
		if (!m_wLoadoutList || !m_LoadoutManager)
			return;

		SetListVisible(true);

		ClearLoadoutList();
		
		if (!playerIds || playerIds.IsEmpty())
			return;

		for (int i = 0; i < playerIds.Count(); ++i)
		{
			int pid = playerIds[i];
			SCR_BasePlayerLoadout playerLoadout = m_LoadoutManager.GetPlayerLoadout(pid);
			CreatePlayerLoadoutButton(playerLoadout, pid, i);
		}
		
		for (int i = playerIds.Count(); i < slotCount; ++i)
		{
			CreateEmptySlot(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removed all buttons from the loadout list.
	protected void ClearLoadoutList()
	{
		Widget child = m_wLoadoutList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}
		
		m_aButtons.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// Create a non-interactive loadout button with player's name.
	protected void CreatePlayerLoadoutButton(SCR_BasePlayerLoadout loadout, int pid, int order)
	{
		Widget name = GetGame().GetWorkspace().CreateWidgets(m_sLoadoutButton, m_wLoadoutList);
		if (!name)
			return;
		
		SCR_LoadoutButton buttonComp = SCR_LoadoutButton.Cast(name.FindHandler(SCR_LoadoutButton));
		if (!buttonComp)
			return;

		buttonComp.SetLoadout(loadout);
		buttonComp.SetPlayer(pid);
		buttonComp.SetSelected(pid == GetGame().GetPlayerController().GetPlayerId());
		buttonComp.SetEnabled(loadout.IsLoadoutAvailableClient());

		buttonComp.m_OnFocus.Insert(OnButtonFocused);
		buttonComp.m_OnFocusLost.Insert(OnButtonFocusLost);
		buttonComp.m_OnMouseEnter.Insert(OnButtonFocused);
		buttonComp.m_OnMouseLeave.Insert(OnMouseLeft);

		int cnt = m_aButtons.Insert(buttonComp);
		
		GridSlot.SetColumn(name, order % LOADOUTS_PER_ROW);
		GridSlot.SetRow(name, order / LOADOUTS_PER_ROW);
	}

	//------------------------------------------------------------------------------------------------
	//! Create an empty slot in the loadout grid.
	protected void CreateEmptySlot(int order)
	{
		Widget slot = GetGame().GetWorkspace().CreateWidgets(m_sLoadoutButton, m_wLoadoutList);
		SCR_DeployButtonBase handler = SCR_DeployButtonBase.Cast(slot.FindHandler(SCR_DeployButtonBase));

		handler.SetShouldUnlock(false);
		m_aButtons.Insert(handler);
		
		GridSlot.SetColumn(slot, order % LOADOUTS_PER_ROW);
		GridSlot.SetRow(slot, order / LOADOUTS_PER_ROW);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLoadouts(int playerID, bool hasValidLoadout)
	{
		array<ref SCR_BasePlayerLoadout> availableLoadouts = {};
		m_LoadoutManager.GetPlayerLoadoutsByFaction(m_PlyFactionAffilComp.GetAffiliatedFaction(), availableLoadouts);
		foreach (SCR_BasePlayerLoadout loadout : availableLoadouts)
		{
			SCR_LoadoutButton loadoutBtn = m_LoadoutSelector.GetButtonForLoadout(loadout);
			if (loadoutBtn)
				loadoutBtn.SetEnabled(loadout.IsLoadoutAvailableClient());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Send a loadout request when clicking on a loadout button.
	protected void OnRequestPlayerLoadout(SCR_LoadoutButton loadoutBtn)
	{
		SCR_BasePlayerLoadout loadout = loadoutBtn.GetLoadout();
		if (!loadout)
			return;

		Lock(loadoutBtn);
		RequestPlayerLoadout(loadout);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RequestPlayerLoadout(SCR_BasePlayerLoadout loadout)
	{
		SetLoadoutPreview(loadout);
		
		if (m_wExpandButtonName)
			m_wExpandButtonName.SetText(loadout.GetLoadoutName());			
		
		m_PlyLoadoutComp.RequestLoadout(loadout);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonFocused(Widget w)
	{
		SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(w.FindHandler(SCR_LoadoutButton));
		if (!loadoutBtn)
			return;
		
		m_OnButtonFocused.Invoke();
		
		if (m_OnPlayerEntryFocused)
			m_OnPlayerEntryFocused.Invoke(loadoutBtn.GetPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonFocusLost(Widget w)
	{
		if (m_OnPlayerEntryFocusLost)
			m_OnPlayerEntryFocusLost.Invoke(w);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnPlayerEntryFocused()
	{
		if (!m_OnPlayerEntryFocused)
			m_OnPlayerEntryFocused = new ScriptInvokerInt();
		
		return m_OnPlayerEntryFocused;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerWidget GetOnPlayerEntryFocusLost()
	{
		if (!m_OnPlayerEntryFocusLost)
			m_OnPlayerEntryFocusLost = new ScriptInvokerWidget();
		
		return m_OnPlayerEntryFocusLost;
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Send a request to assign a random faction loadout.
	void RequestRandomLoadout(Faction faction)
	{
		array<ref SCR_BasePlayerLoadout> loadouts = {};
		m_LoadoutManager.GetPlayerLoadoutsByFaction(faction, loadouts);
		SCR_BasePlayerLoadout rndLoadout = loadouts.GetRandomElement();
		m_PlyLoadoutComp.RequestLoadout(loadouts.GetRandomElement());
		GetGame().GetCallqueue().CallLater(SetLoadoutPreview, 10, false, rndLoadout);
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	void SetSelected(SCR_PlayerLoadoutComponent component)
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_LoadoutButton loadoutBtn = SCR_LoadoutButton.Cast(btn);
			if (loadoutBtn)
				loadoutBtn.SetSelected(loadoutBtn.GetPlayerId() == component.GetPlayerId());
		}
		
		if (m_LoadoutSelector)
			m_LoadoutSelector.SetSelected(component.GetLoadout());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSpawnPointChanged(RplId id)
	{
		RefreshLoadoutPreview();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadoutSpawnCostMultiplierChanged(float newMultiplier)
	{
		RefreshLoadoutPreview();
	}

	//------------------------------------------------------------------------------------------------
	void RegisterOnSpawnPointInvoker(notnull OnSpawnPointRplIdSetInvoker invoker)
	{
		invoker.Insert(OnSpawnPointChanged);
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	protected float GetLoadoutCost(SCR_BasePlayerLoadout loadout = null)
	{
		IEntity spawnPoint;
		
		//~ TODO: Get loadout refresh should be called on spawnpoint selection changed
		//~ Get selected spawnpoint
		SCR_DeployMenuMain deployMenu = SCR_DeployMenuMain.GetDeployMenu();
		if (deployMenu)
		{
			SCR_SpawnPointRequestUIComponent spawnPointRequest = deployMenu.GetSpawnPointRequestHandler();
			if (spawnPointRequest)
			{
				RplComponent rplComp = RplComponent.Cast(Replication.FindItem(spawnPointRequest.GetCurrentRplId()));
				if (rplComp)
					spawnPoint = rplComp.GetEntity();
			}
		}
		
		if (loadout)
			return SCR_ArsenalManagerComponent.GetLoadoutCalculatedSupplyCost(loadout, true, -1, SCR_Faction.Cast(m_PlyFactionAffilComp.GetAffiliatedFaction()), spawnPoint);
		else
			return SCR_ArsenalManagerComponent.GetLoadoutCalculatedSupplyCost(m_PlyLoadoutComp.GetLoadout(), true, -1, SCR_Faction.Cast(m_PlyFactionAffilComp.GetAffiliatedFaction()), spawnPoint);
	}

	//------------------------------------------------------------------------------------------------
	//! Set a loadout shown in the preview widget.
	protected void SetLoadoutPreview(SCR_BasePlayerLoadout loadout)
	{
		InputManager inputManager = GetGame().GetInputManager();
		
		if (inputManager && inputManager.GetLastUsedInputDevice() == EInputDeviceType.KEYBOARD)
			return;
		
		if (m_PreviewComp && loadout)
		{
			if (m_wLoadoutName)
				m_wLoadoutName.SetText(loadout.GetLoadoutName());			
			m_PreviewedEntity = m_PreviewComp.SetPreviewedLoadout(loadout);
			m_wLoadoutPreview.SetVisible(true);

			if (m_wSuppliesText)
			{
				int supplyCost = GetLoadoutCost(loadout);

				// Don't show supply cost outside of Deploy Menu
				if (supplyCost < 0 || !SCR_DeployMenuMain.GetDeployMenu())
				{
					m_wSupplies.SetVisible(false);
				}
				else
				{	
					m_wSupplies.SetVisible(true);
					m_wSuppliesText.SetText(SCR_ResourceSystemHelper.SuppliesToString(supplyCost));
				}
			}
			
			if (m_wMSARSupplies && m_PlayerSupplyAllocationUI && SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
				m_PlayerSupplyAllocationUI.OnLoadoutSelected(loadout);

			if (m_wLoadoutNameText && m_wSupplies.IsVisible())
				m_wLoadoutNameText.SetText(loadout.GetLoadoutName());

			if (m_wExpandButtonName)
				m_wExpandButtonName.SetText(loadout.GetLoadoutName());
			
			if (m_wExpandButtonIcon && loadout)
			{
				SCR_EditableEntityUIInfo entityUIInfo = GetUIInfo(loadout);
				entityUIInfo.SetIconTo(m_wExpandButtonIcon);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Refreshes current loadout preview widget.
	void RefreshLoadoutPreview()
	{
		SCR_BasePlayerLoadout loadout = m_PlyLoadoutComp.GetLoadout();
		if (!loadout)
			return;

		SetLoadoutPreview(loadout);

		if (m_LoadoutSelector)
			m_LoadoutSelector.SetSelected(loadout);

		if (m_wLoadoutName && loadout)
			m_wLoadoutName.SetText(loadout.GetLoadoutName());

		if (m_wExpandButtonName && loadout)
			m_wExpandButtonName.SetText(loadout.GetLoadoutName());

		if (m_wExpandButtonIcon && loadout)
		{
			SCR_EditableEntityUIInfo entityUIInfo = GetUIInfo(loadout);
			
			entityUIInfo.SetIconTo(m_wExpandButtonIcon);
		}
		
		if (m_wSuppliesText && loadout)
		{
			int supplyCost = GetLoadoutCost();
			
			// Don't show supply cost outside of Deploy Menu
			if (supplyCost < 0 || !SCR_DeployMenuMain.GetDeployMenu())
			{
				m_wSupplies.SetVisible(false);
			}
			else
			{
				m_wSupplies.SetVisible(true);
				m_wSuppliesText.SetText(SCR_ResourceSystemHelper.SuppliesToString(supplyCost));
			}
		}
		
		if (m_wMSARSupplies && loadout && m_PlayerSupplyAllocationUI && SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
			m_PlayerSupplyAllocationUI.OnLoadoutSelected(loadout);

		if (m_wLoadoutNameText && loadout && m_wSupplies.IsVisible())
			m_wLoadoutNameText.SetText(loadout.GetLoadoutName());

		ImageWidget LoadoutBackground = ImageWidget.Cast((m_wLoadoutSelector.GetParent()).GetParent().FindAnyWidget("Background"));
		
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)	
			return;
		
		TimeAndWeatherManagerEntity tmwManager = world.GetTimeAndWeatherManager();
		if (!tmwManager)
			return;
				
		int dayTime = tmwManager.GetCurrentDayTimeUIInfo(uiInfo);
		SCR_Faction faction = SCR_Faction.Cast(m_PlyFactionAffilComp.GetAffiliatedFaction());
		WeatherStateTransitionManager transitionManager = WeatherStateTransitionManager.Cast(tmwManager.GetTransitionManager());
		m_sWeatherModifier = transitionManager.GetCurrentState().GetStateName();
		
		if (!m_sDayModifierStrings.IsIndexValid(dayTime))
			return;
		
		m_sDayModifier = m_sDayModifierStrings[dayTime];
		
		if (!faction) 
			return;
		
		m_sLoadoutImageSet = faction.GetFactionBackground();
		
		if (m_sWeatherModifier == "Rainy")
			m_sLoadoutImageName = m_sDayModifier + m_sWeatherModifier;	
		else
			m_sLoadoutImageName = m_sDayModifier;

		LoadoutBackground.LoadImageFromSet(0, m_sLoadoutImageSet, m_sLoadoutImageName);
	}
	//---- REFACTOR NOTE END ----
	//------------------------------------------------------------------------------------------------
	//! Get local player's assigned loadout.
	SCR_BasePlayerLoadout GetPlayerLoadout()
	{
		return m_PlyLoadoutComp.GetAssignedLoadout();
	}

	//------------------------------------------------------------------------------------------------
	//! Add a loadout into the loadout selector.
	protected void AddLoadout(SCR_BasePlayerLoadout loadout)
	{
		if (loadout)
			m_LoadoutSelector.AddItem(loadout);
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetPlayerLoadoutPreview()
	{
		if (m_wLoadoutPreview)
			m_wLoadoutPreview.SetVisible(false);

		if (m_wLoadoutName)
			m_wLoadoutName.SetText("#AR-DeployMenu_SelectLoadout");
	}

	//------------------------------------------------------------------------------------------------
	void SetListVisible(bool visible)
	{
		if (m_wLoadoutList)
			m_wLoadoutList.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	void ShowLoadoutSelector(bool show)
	{
		m_wRoot.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EditableEntityUIInfo GetUIInfo(SCR_BasePlayerLoadout loadout)
	{
		if (!loadout)
			return null;

		Resource res = Resource.Load(loadout.GetLoadoutResource());
		IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
		if (source)
		{
			BaseContainer container = source.GetObject("m_UIInfo");
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

			return info;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override void SetListWidget(Widget list)
	{
		if (m_wLoadoutList)
		{
//			SetListVisible(false);
			ClearLoadoutList();		
		}
		
		m_wLoadoutList = GridLayoutWidget.Cast(list);
	}

	//------------------------------------------------------------------------------------------------
	bool IsSelectorFocused()
	{
		return m_LoadoutSelector && m_LoadoutSelector.GetFocused();
	}
};

//------------------------------------------------------------------------------------------------
class SCR_LoadoutButton : SCR_DeployButtonBase
{
	[Attribute("PlayerName")]
	protected string m_sPlayerName;
	
	[Attribute("Leader")]
	protected string m_sLeaderText;
	
	[Attribute("PlatformIcon")]
	protected string m_sPlatformIconName;
	
	[Attribute("Badge")]
	protected string m_sBadgeWidgetName;
	
	[Attribute("SuppliesLoadoutText")]
	protected string m_sSuppliesLoadoutText;

	protected TextWidget m_wSuppliesLoadoutText;
	
	protected TextWidget m_wPlayerName;
	protected Widget m_wLeaderText;
	protected ImageWidget m_wPlatformIcon;
	protected SizeLayoutWidget m_wBadge;

	protected SCR_BasePlayerLoadout m_Loadout;
	protected int m_iPlayerId = -1;
	protected bool m_bIsSelected;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wPlayerName = TextWidget.Cast(w.FindAnyWidget(m_sPlayerName));
		m_wLeaderText = w.FindAnyWidget(m_sLeaderText);
		m_wElements = w.FindAnyWidget(m_sElements);
		m_wPlatformIcon = ImageWidget.Cast(w.FindAnyWidget(m_sPlatformIconName));
		m_wBadge = SizeLayoutWidget.Cast(w.FindAnyWidget(m_sBadgeWidgetName));
		m_wSuppliesLoadoutText = TextWidget.Cast(GetRootWidget().FindAnyWidget(m_sSuppliesLoadoutText));
		
		if (m_wLeaderText)
			SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnLeaderChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(OnLeaderChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if(m_bShowBorderOnFocus)
			m_bShowBorderOnFocus = false;
		
		if(!GetGame().GetInputManager().IsUsingMouseAndKeyboard() || m_bIsSelected)
			ShowBorder(true, true);
		else
			ShowBorder(false, true);
		
		return super.OnFocus(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if(!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && !m_bIsSelected)
			ShowBorder(false, true);
		
		if(m_bIsSelected)
			ShowBorder(true, false);
		
		return super.OnFocusLost(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assign loadout associated with this button.
	void SetLoadout(SCR_BasePlayerLoadout loadout)
	{
		m_Loadout = loadout;
		if (!loadout)
			return;

		SCR_EditableEntityUIInfo entityUIInfo = GetUIInfo();
		
		if (entityUIInfo)
		{
			string iconPath = entityUIInfo.GetIconPath();
			if (iconPath.IsEmpty())
				SetImage(entityUIInfo.GetImageSetPath(), entityUIInfo.GetIconSetName());
			else
				SetImage(iconPath);
			
			if (!m_wBadge)
				return;
			
			m_wBadge.SetOpacity(1.00);
			m_wBadge.SetColor(entityUIInfo.GetFaction().GetFactionColor());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set player id associated with this button and update the visuals.
	void SetPlayer(int pid)
	{
		m_iPlayerId = pid;
		SetPlayerName(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(pid));
		
		if (m_wPlatformIcon)
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).SetPlatformImageTo(m_iPlayerId, m_wPlatformIcon);

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup group = groupManager.GetPlayerGroup(pid);
		if (!group)
			return;

		SetIsLeader(pid == m_iPlayerId && group.IsPlayerLeader(pid));
	}

	//------------------------------------------------------------------------------------------------
	//! Update visuals when group leader changes.
	protected void OnLeaderChanged(int groupId, int pid)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup group = groupManager.FindGroup(groupId);
		if (!group)
			return;

		if (pid == m_iPlayerId)
			SetIsLeader(group.IsPlayerLeader(pid));
	}

	//------------------------------------------------------------------------------------------------
	protected void SetIsLeader(bool leader)
	{
		if (m_wLeaderText)
			m_wLeaderText.SetVisible(leader);
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_iPlayerId;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsSelected()
	{
		return m_bIsSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetSelected(bool selected)
	{
		super.SetSelected(selected);

		m_bIsSelected = selected;
		
		ShowBorder(selected, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPlayerName(string name)
	{
		if (m_wPlayerName)
			m_wPlayerName.SetText(name);
	}

	//------------------------------------------------------------------------------------------------
	//!return player loadout
	SCR_BasePlayerLoadout GetLoadout()
	{
		return m_Loadout;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EditableEntityUIInfo GetUIInfo()
	{
		Resource res = Resource.Load(m_Loadout.GetLoadoutResource());
		IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
		if (source)
		{
			BaseContainer container = source.GetObject("m_UIInfo");

			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

			return info;
		}

		return null;
	}
};
