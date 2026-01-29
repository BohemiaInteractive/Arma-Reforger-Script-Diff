//! @ingroup Editor_UI Editor_UI_Components

class SCR_PlayersListEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute()]
	protected string m_sListWidgetName;

	[Attribute()]
	protected string m_sCountWidgetName;
	
	protected Widget m_ListWidget;
	protected TextWidget m_CountWidget;
	protected SCR_PlayersManagerEditorComponent m_PlayersManager;
	protected SCR_PlayerDelegateEditableEntityFilter m_PlayerDelegatesFilter;
	protected ResourceName m_EntryPrefab;
	protected ref map<SCR_EditablePlayerDelegateComponent, SCR_PlayersListEntryEditorUIComponent> m_mEntries = new map<SCR_EditablePlayerDelegateComponent, SCR_PlayersListEntryEditorUIComponent>();
	
	//------------------------------------------------------------------------------------------------
	protected void AddPlayer(SCR_EditableEntityComponent entity)
	{
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
		if (!delegate || m_mEntries.Contains(delegate))
			return;
		
		int playerID = delegate.GetPlayerID();
		
		Widget entryWidget = m_ListWidget.GetWorkspace().CreateWidgets(m_EntryPrefab, m_ListWidget);
		SCR_PlayersListEntryEditorUIComponent entry = SCR_PlayersListEntryEditorUIComponent.Cast(entryWidget.FindHandler(SCR_PlayersListEntryEditorUIComponent));
		
		//--- Name
		entry.SetName(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID));
		
		//--- Life status
		bool dead = true;
		GenericEntity player = GenericEntity.Cast(SCR_PossessingManagerComponent.GetPlayerMainEntity(playerID));
		if (player)
		{
			DamageManagerComponent playerDamage = DamageManagerComponent.Cast(player.FindComponent(DamageManagerComponent));
			//TODO @Zguba LEVEL2: update during DamageState refactor
			if (playerDamage) 
				dead = (playerDamage.GetHealthScaled() == 0);
		}

		entry.SetDead(dead);
		
		//--- Entity link
		entry.SetLinkedEntity(delegate);
		
		m_mEntries.Insert(delegate, entry);
		UpdateCount();
	}

	//------------------------------------------------------------------------------------------------
	protected void RemovePlayer(SCR_EditableEntityComponent entity)
	{
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
		if (!delegate)
			return;
		
		SCR_PlayersListEntryEditorUIComponent entry;
		if (m_mEntries.Find(delegate, entry))
			entry.GetWidget().RemoveFromHierarchy();
		
		UpdateCount();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCount()
	{
		if (m_CountWidget)
			m_CountWidget.SetText(m_mEntries.Count().ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDelegatesChanged(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (entitiesInsert)
		{
			foreach (SCR_EditableEntityComponent delegate: entitiesInsert)
			{
				AddPlayer(delegate);
			}
		}

		if (entitiesRemove)
		{
			foreach (SCR_EditableEntityComponent delegate: entitiesRemove)
			{
				RemovePlayer(delegate);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSpawn(int playerID, SCR_EditableEntityComponent entity, SCR_EditableEntityComponent entityPrev)
	{
		foreach (SCR_EditablePlayerDelegateComponent delegate, SCR_PlayersListEntryEditorUIComponent entry : m_mEntries)
		{
			if (delegate.GetPlayerID() == playerID)
			{
				entry.SetLinkedEntity(delegate);
				entry.SetName(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID));
				entry.SetDead(false);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDeath(int playerID, SCR_EditableEntityComponent entity, SCR_EditableEntityComponent killer)
	{
		foreach (SCR_EditablePlayerDelegateComponent delegate, SCR_PlayersListEntryEditorUIComponent entry : m_mEntries)
		{
			if (delegate.GetPlayerID() == playerID)
			{
				entry.SetDead(true);
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_PlayerDelegatesFilter = SCR_PlayerDelegateEditableEntityFilter.Cast(SCR_PlayerDelegateEditableEntityFilter.GetInstance(EEditableEntityState.PLAYER, true));
		if (!m_PlayerDelegatesFilter)
			return;
		
		m_PlayersManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent));
		if (m_PlayersManager)
		{
			m_PlayersManager.GetOnSpawn().Insert(OnSpawn);
			m_PlayersManager.GetOnDeath().Insert(OnDeath);
		}
		
		//--- GUI init
		m_ListWidget = w.FindAnyWidget(m_sListWidgetName);
		if (!m_ListWidget)
			return;
		
		m_EntryPrefab = SCR_LayoutTemplateComponent.GetLayout(m_ListWidget);
		if (m_EntryPrefab.IsEmpty())
			return;
		
		m_CountWidget = TextWidget.Cast(w.FindAnyWidget(m_sCountWidgetName));
		
		//--- Filter init
		set<SCR_EditableEntityComponent> entities = new set<SCR_EditableEntityComponent>();
		for (int i = 0, count = m_PlayerDelegatesFilter.GetEntities(entities); i < count; i++)
		{
			//for (int e = 0; e < 100; e++)
			AddPlayer(entities[i]);
		}
		
		m_PlayerDelegatesFilter.GetOnChanged().Insert(OnPlayerDelegatesChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_PlayersManager)
		{
			m_PlayersManager.GetOnSpawn().Remove(OnSpawn);
			m_PlayersManager.GetOnDeath().Remove(OnDeath);
		}
		
		if (m_PlayerDelegatesFilter)
			m_PlayerDelegatesFilter.GetOnChanged().Remove(OnPlayerDelegatesChanged);
	}
}
