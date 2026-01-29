class SCR_SpawnPointRequestUIComponent : SCR_DeployRequestUIBaseComponent
{
	[Attribute("Selector")]
	protected string m_sSpawnPointSelector;
	protected Widget m_wSpawnPointSelector;

	protected RplId m_SelectedSpawnPointId = RplId.Invalid();
	protected SCR_SpawnPointSpinBox m_SpawnPointSelector;

	protected ref ScriptInvoker<RplId> m_OnSpawnPointSelected;
	static ref ScriptInvoker<RplId> s_OnSpawnPointSelected;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wSpawnPointSelector = w.FindAnyWidget(m_sSpawnPointSelector);
		if (m_wSpawnPointSelector)
			m_SpawnPointSelector = SCR_SpawnPointSpinBox.Cast(m_wSpawnPointSelector.FindHandler(SCR_SpawnPointSpinBox));

		if (!m_SpawnPointSelector)
			return;

		m_SpawnPointSelector.m_OnChanged.Insert(SelectSpawnPoint);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnPointFactionChange);
		SCR_SpawnPoint.Event_SpawnPointAdded.Insert(OnSpawnPointAdded);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(RemoveSpawnPoint);
		SCR_SpawnPoint.OnSpawnPointNameChanged.Insert(UpdateSpawnPointName);
	}

	override void HandlerDeattached(Widget w)
	{
		if (m_SpawnPointSelector)
			m_SpawnPointSelector.m_OnChanged.Remove(SelectSpawnPoint);
		
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnPointFactionChange);
		SCR_SpawnPoint.Event_SpawnPointAdded.Remove(OnSpawnPointAdded);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Remove(RemoveSpawnPoint);		
		SCR_SpawnPoint.OnSpawnPointNameChanged.Remove(UpdateSpawnPointName);
	}
	
	protected void SelectSpawnPoint(SCR_SpawnPointSpinBox spinbox, int itemId)
	{
		if (spinbox.IsEmpty())
			return;

		if (spinbox.GetSpawnPointId(itemId) != m_SelectedSpawnPointId)
		{
			m_SelectedSpawnPointId = spinbox.GetSpawnPointId(itemId);
			GetOnSpawnPointSelected().Invoke(m_SelectedSpawnPointId);
			SGetOnSpawnPointSelected().Invoke(m_SelectedSpawnPointId);
		}
	}

	protected void SetSpawnPoint(RplId spawnPointId)
	{
		m_SelectedSpawnPointId = spawnPointId;
		GetOnSpawnPointSelected().Invoke(spawnPointId);
	}

	void CycleSpawnPoints(bool next = true) // false for previous spawn
	{
		int currentIndex = m_SpawnPointSelector.GetCurrentIndex();
		int nextIndex = currentIndex + 1;
		if (!next)
			nextIndex = currentIndex - 1;

		int itemCount = m_SpawnPointSelector.m_aElementNames.Count();
		if (nextIndex >= itemCount)
			nextIndex = 0;
		else if (nextIndex < 0)
			nextIndex = itemCount - 1;
		
		m_SpawnPointSelector.SetCurrentItem(nextIndex);
	}

	protected void AddSpawnPoint(SCR_SpawnPoint spawnPoint)
	{
		if (spawnPoint && spawnPoint.IsSpawnPointVisibleForPlayer(SCR_PlayerController.GetLocalPlayerId()))
		{
			RplId currentSpawnPointId = m_SelectedSpawnPointId;
			string name = spawnPoint.GetSpawnPointName();
			m_SpawnPointSelector.AddItem(name, spawnPoint.GetRplId());
			
			if (spawnPoint.GetRplId() != currentSpawnPointId)
			{
				int itemId = m_SpawnPointSelector.GetItemId(currentSpawnPointId);
				if (itemId > -1)
					m_SpawnPointSelector.SetCurrentItem(itemId); // make sure currently selected spawn point stays selected after new point is added
			}
		}
	}

	protected void RemoveSpawnPoint(SCR_SpawnPoint spawnPoint)
	{
		RplId currentSpawnPointId = m_SelectedSpawnPointId;
		int itemId = m_SpawnPointSelector.GetItemId(spawnPoint.GetRplId());
		if (itemId == -1)
			return;

		m_SpawnPointSelector.RemoveItem(itemId);
		
		if (spawnPoint.GetRplId() != currentSpawnPointId)
		{
			itemId = m_SpawnPointSelector.GetItemId(currentSpawnPointId);
			m_SpawnPointSelector.SetCurrentItem(itemId);
			m_SelectedSpawnPointId = GetCurrentRplId();
		}
	}
	
	protected void UpdateSpawnPointName(RplId id, string name)
	{
		int itemId = m_SpawnPointSelector.GetItemId(id);
		m_SpawnPointSelector.SetItemName(itemId, name);
	}

	protected void OnSpawnPointAdded(SCR_SpawnPoint spawnPoint)
	{
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (playerFaction && playerFaction.GetFactionKey() == spawnPoint.GetFactionKey())
			AddSpawnPoint(spawnPoint);
	}

	protected void OnSpawnPointFactionChange(SCR_SpawnPoint spawnPoint)
	{
		Faction playerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (playerFaction && playerFaction.GetFactionKey() == spawnPoint.GetFactionKey())
			AddSpawnPoint(spawnPoint);
		else
			RemoveSpawnPoint(spawnPoint);
	}
	
	protected void ClearSpawnPoints()
	{
		m_SpawnPointSelector.ClearAll();
		// todo@lk: show warning
	}

	void ShowAvailableSpawnPoints(Faction faction)
	{
		if (!faction)
			return;

		ClearSpawnPoints();

		array<SCR_SpawnPoint> infos = SCR_SpawnPoint.GetSpawnPointsForFaction(faction.GetFactionKey());
		if (infos.IsEmpty())
		{
			SetSpawnPoint(RplId.Invalid());
			return;
		}

		infos.Sort(true);

		int pid = SCR_PlayerController.GetLocalPlayerId();
		foreach (SCR_SpawnPoint info : infos)
		{
			if (info.IsSpawnPointVisibleForPlayer(pid))
				AddSpawnPoint(info);
		}

		SetLastUsedSpawnPointSelected();
		SetRallyPointSpawnSelected();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the last used spawned point by player as default selected option in spawn selector
	//! Does not affect selection if there is no last used spawn point, or it is unavailable
	protected void SetLastUsedSpawnPointSelected()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		SCR_PlayerDeployMenuHandlerComponent menuHandler = SCR_PlayerDeployMenuHandlerComponent.Cast(pc.FindComponent(SCR_PlayerDeployMenuHandlerComponent));
		if (!menuHandler)
			return;

		int lastUsedSpawnPointRplId = menuHandler.GetLastUsedSpawnPointId();

		if (!SCR_SpawnPoint.GetSpawnPointByRplId(lastUsedSpawnPointRplId))
			return;

		int lastUsedSpawnPointId = m_SpawnPointSelector.GetItemId(lastUsedSpawnPointRplId);
		if (lastUsedSpawnPointId <= 0)
			return;

		m_SpawnPointSelector.SetCurrentItem(lastUsedSpawnPointId);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the player group Rally Point as default selected option in spawn selector
	protected void SetRallyPointSpawnSelected()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		SCR_PlayerControllerGroupComponent playerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(pc.GetPlayerId());
		if (!playerControllerGroupComponent)
			return;

		int playerGroupId = playerControllerGroupComponent.GetGroupID();
		SCR_AIGroup playerGroup = SCR_GroupsManagerComponent.GetInstance().FindGroup(playerGroupId);
		if (!playerGroup)
			return;

		int rallyPointBaseCallsignId = playerGroup.GetRallyPointId();
		if (rallyPointBaseCallsignId < 0)
			return;

		array<SCR_SpawnPoint> spawnPoints = m_SpawnPointSelector.GetSpawnPointsInList();
		IEntity parentEntity;
		SCR_MilitaryBaseComponent baseComponent;
		foreach (SCR_SpawnPoint spawnPoint : spawnPoints)
		{
			parentEntity = spawnPoint.GetRootParent();
			if (!parentEntity)
				continue;

			baseComponent = SCR_MilitaryBaseComponent.Cast(parentEntity.FindComponent(SCR_MilitaryBaseComponent));
			if (!baseComponent)
				continue;

			if (baseComponent.GetCallsign() != rallyPointBaseCallsignId)
				continue;

			int rallyPointSpawnPointId = m_SpawnPointSelector.GetItemId(spawnPoint.GetRplId());
			m_SpawnPointSelector.SetCurrentItem(rallyPointSpawnPointId);

			return;
		}
	}

	void UpdateRelevantSpawnPoints()
	{
		array<SCR_SpawnPoint> spawnPoints = m_SpawnPointSelector.GetSpawnPointsInList();
		PlayerController pc = GetGame().GetPlayerController();
		SCR_PlayerFactionAffiliationComponent plyFactionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		array<SCR_SpawnPoint> playerSpawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(plyFactionComp.GetAffiliatedFaction().GetFactionKey());

		foreach (SCR_SpawnPoint sp : spawnPoints)
		{
			if (!sp.IsSpawnPointVisibleForPlayer(pc.GetPlayerId()))
				RemoveSpawnPoint(sp);
		}

		foreach (SCR_SpawnPoint sp : playerSpawnPoints)
		{
			if (spawnPoints.Contains(sp))
				continue;

			if (sp.IsSpawnPointVisibleForPlayer(pc.GetPlayerId()))
				AddSpawnPoint(sp);
		}
	}

	void SelectSpawnPointExt(RplId id)
	{
		int itemId = m_SpawnPointSelector.GetItemId(id);
		if (itemId > -1)
			m_SpawnPointSelector.SetCurrentItem(itemId);
	}

	ScriptInvoker GetOnSpawnPointSelected()
	{
		if (!m_OnSpawnPointSelected)
			m_OnSpawnPointSelected = new ScriptInvoker();

		return m_OnSpawnPointSelected;
	}
	
	static ScriptInvoker SGetOnSpawnPointSelected()
	{
		if (!s_OnSpawnPointSelected)
			s_OnSpawnPointSelected = new ScriptInvoker();
		
		return s_OnSpawnPointSelected;
	}

	RplId GetCurrentRplId()
	{
		return m_SpawnPointSelector.GetSpawnPointId(m_SpawnPointSelector.GetCurrentIndex());
	}

	bool IsSelectorFocused()
	{
		return m_SpawnPointSelector && m_SpawnPointSelector.IsFocused();
	}
};