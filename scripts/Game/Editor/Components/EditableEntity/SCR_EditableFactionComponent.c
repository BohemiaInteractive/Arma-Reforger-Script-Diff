[ComponentEditorProps(category: "GameScripted/Editor (Editables)", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_EditableFactionComponentClass : SCR_EditableEntityComponentClass
{
	//------------------------------------------------------------------------------------------------
	static override bool GetEntitySourceBudgetCost(IEntityComponentSource editableEntitySource, out notnull array<ref SCR_EntityBudgetValue> budgetValues)
	{
		// Avoid fallback entityType cost
		return true;
	}
}

//! @ingroup Editable_Entities

//! Special configuration for editable faction.
class SCR_EditableFactionComponent : SCR_EditableEntityComponent
{
	[Attribute()]
	protected ref array<ref SCR_ArsenalItemCountConfig> m_MaxCountPerItemType;
	
	protected int m_iFactionIndex = -1;
	
	protected Faction m_Faction;
	protected SCR_Faction m_ScrFaction;
	protected ref SCR_UIInfo m_Info;
	
	//Faction info
	protected int m_iSpawnPointCount = -1;
	protected int m_iTaskCount = -1;
	
	
	//Script invokers
	protected ref ScriptInvoker Event_OnSpawnPointCountChanged = new ScriptInvoker();
	protected ref ScriptInvoker Event_OnTaskCountChanged = new ScriptInvoker();
	
	//Arsenal
	protected SCR_EArsenalItemType m_AllowedArsenalItemTypes;
	protected ref map<SCR_EArsenalItemType, int> m_aCurrentItemTaken = new map<SCR_EArsenalItemType, int>();
	
	private bool m_bQueuedSpawnpointChanges = false;
	private bool m_bQueuedTaskCountChanged = false;

	//------------------------------------------------------------------------------------------------
	//! Assign faction to this editable entity.
	//! \param[in] index Index of the faction in FactionManager array
	void SetFactionIndex(int index)
	{
		if (m_iFactionIndex != -1)
			return;
		
		SetFactionIndexBroadcast(index);
		Rpc(SetFactionIndexBroadcast, index);
	}

	//------------------------------------------------------------------------------------------------
	//! Get index of a faction represented by this delegate.
	//! \return Faction index
	int GetFactionIndex()
	{
		return m_iFactionIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetFactionIndexBroadcast(int index)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		//--- Find faction on the manager (factions are local, that's why they're identified by index)
		m_Faction = factionManager.GetFactionByIndex(index);
		if (!m_Faction)
			return;
		
		m_ScrFaction = SCR_Faction.Cast(m_Faction);
		if (!m_ScrFaction)
			return;
		
		//--- Set the value
		m_iFactionIndex = index;
		
		//--- Copy UI info into editable entity
		m_Info = SCR_UIInfo.CreateInfo(m_Faction.GetUIInfo());
		SetInfoInstance(m_Info);
		
		//--- Register the faction back in the manager on local machine
		SCR_DelegateFactionManagerComponent delegatesManager = SCR_DelegateFactionManagerComponent.GetInstance();
		if (delegatesManager)
			delegatesManager.SetFactionDelegate(m_Faction, this);
		
		//--- Hide non-playable faction
		SetVisible(m_ScrFaction.IsPlayable());
		
		//If Workbench or client
		if (SCR_Global.IsEditMode(GetOwner()) || Replication.IsClient())
			return;
	
		//Init count. Call one frame later to make sure the counts are correct
		GetGame().GetCallqueue().CallLater(InitSpawnPointCount); 
		GetGame().GetCallqueue().CallLater(InitTaskCount);
		
		//Spawnpoints
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Insert(OnSpawnPointCountChanged);
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnpointFactionChanged);
		
		//Tasks
		SCR_TaskSystem.GetOnTaskAdded().Insert(OnTaskAdded);
		SCR_TaskSystem.GetOnTaskRemoved().Insert(OnTaskRemoved);
	}
	
	//======================================== FACTION RELATIONSHIP REPLICATION ========================================\\
	//------------------------------------------------------------------------------------------------
	//! Replicates the setting faction to friendly. Called by SCR_FactionManager (Server only)
	//! \param[in] FactionIndex index to set faction friendly to
	void SetFactionFriendly_S(int FactionIndex)
	{
		Rpc(SetFactionFriendlyBroadcast, FactionIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetFactionFriendlyBroadcast(int FactionIndex)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!SCR_FactionManager)
			return;
		
		SCR_Faction refFaction = SCR_Faction.Cast(m_Faction);
		if (!refFaction)
			return;
		
		SCR_Faction scrFaction = SCR_Faction.Cast(factionManager.GetFactionByIndex(FactionIndex));
		if (!scrFaction)
			return;
		
		factionManager.SetFactionsFriendly(refFaction, scrFaction, updateAIs: false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Replicates the setting faction to hostile. Called by SCR_FactionManager (Server Only)
	//! \param[in] FactionIndex index to set faction hostile to
	void SetFactionHostile_S(int FactionIndex)
	{
		Rpc(SetFactionHostileBroadcast, FactionIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetFactionHostileBroadcast(int FactionIndex)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!SCR_FactionManager)
			return;
		
		SCR_Faction refFaction = SCR_Faction.Cast(m_Faction);
		if (!refFaction)
			return;
		
		SCR_Faction scrFaction = SCR_Faction.Cast(factionManager.GetFactionByIndex(FactionIndex));
		if (!scrFaction)
			return;
		
		factionManager.SetFactionsHostile(refFaction, scrFaction, updateAIs: false);
	}
	
	//======================================== FACTION SPAWNPOINTS ========================================\\
	//------------------------------------------------------------------------------------------------
	// Called on server
	protected void InitSpawnPointCount()
	{		
		int spawnPointCount = SCR_SpawnPoint.GetSpawnPointCountForFaction(m_Faction.GetFactionKey());
		
		InitSpawnPointCountBroadcast(spawnPointCount);
		Rpc(InitSpawnPointCountBroadcast, spawnPointCount);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void InitSpawnPointCountBroadcast(int spawnPointCount)
	{
		m_iSpawnPointCount = spawnPointCount;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSpawnpointFactionChanged(SCR_SpawnPoint spawnPoint)
	{
		QueueSpawnPointChanged(m_Faction.GetFactionKey());
	}
	
	//---------------------------------------- On Faction Spawnpoints changed ----------------------------------------\\
	//! Queues a spawn point change. We only have to notify once per frame that spawnpoints changed, instead of once per changed spawnpoint.
	//! \param[in] FactionKey faction faction that changed
	private void QueueSpawnPointChanged(FactionKey faction)
	{
		//changes already queued
		if(m_bQueuedSpawnpointChanges)
			return;
		
		m_bQueuedSpawnpointChanges = true;
		GetGame().GetCallqueue().CallLater(ProcessQueuedSpawnPointChanges);	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Queues a spawn point change.
	//! \param[in] FactionKey faction faction that changed
	protected void OnSpawnPointCountChanged(string factionKey)
	{
		if (factionKey != m_Faction.GetFactionKey()) 
			return;		
		
		QueueSpawnPointChanged(factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls the method that sends the invoker and calls the RPC for clients for spawn points changed
	//! Basically we are "accumulating" spawn point changes through the frame and then notifying about 
	//! all of them at once instead of one by one
	protected void ProcessQueuedSpawnPointChanges()
	{
		m_bQueuedSpawnpointChanges = false;
		
		int spawnPointCount = SCR_SpawnPoint.GetSpawnPointCountForFaction(m_Faction.GetFactionKey());
		
		//No change
		if (spawnPointCount == m_iSpawnPointCount)
			return;
		
		OnSpawnPointsChanged(m_Faction.GetFactionKey());
	}
	
	//------------------------------------------------------------------------------------------------
	// Called on server
	protected void OnSpawnPointsChanged(string factionKey)
	{		
		int spawnPointCount = SCR_SpawnPoint.GetSpawnPointCountForFaction(m_Faction.GetFactionKey());
		
		//Notification no spawns
		if (spawnPointCount == 0)
		{
			int thisRplId = Replication.FindId(this);
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_FACTION_NO_SPAWNS, thisRplId);
		}
		
		//Update Spawn count
		OnSpawnPointCountChangedBroadcast(spawnPointCount);
		
		Rpc(OnSpawnPointCountChangedBroadcast, spawnPointCount);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnSpawnPointCountChangedBroadcast(int spawnPointCount)
	{
		m_iSpawnPointCount = spawnPointCount;
		Event_OnSpawnPointCountChanged.Invoke(m_Faction, spawnPointCount);
	}

	//======================================== FACTION TASKS ========================================\\

	//------------------------------------------------------------------------------------------------
	// Called on server
	protected void InitTaskCount()
	{		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		array<SCR_Task> tasks = {};
		taskSystem.GetTasksByState(tasks, SCR_ETaskState.CREATED, m_Faction.GetFactionKey());
		
		InitTaskCountBroadcast(tasks.Count());
		Rpc(InitTaskCountBroadcast, tasks.Count());
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void InitTaskCountBroadcast(int factionTaskCount)
	{
		m_iTaskCount = factionTaskCount;
	}	
	
	//---------------------------------------- Faction Task Count Changed ----------------------------------------\\
	//Called on server
	protected void OnTaskAdded(notnull SCR_Task task)
	{		
		QueueTaskCountChanges(task);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTaskRemoved(notnull SCR_Task task)
	{		
		QueueTaskCountChanges(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Queues a task change. We only have to notify once per frame that tasks changed, instead of once per changed task.
	//! \param[in] SCR_Task task task that changed
	protected void QueueTaskCountChanges(notnull SCR_Task task)
	{
		if(m_bQueuedTaskCountChanged)
			return;
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		if (!task)
			return;
		
		array<string> factionKeys = task.GetOwnerFactionKeys();
		if (factionKeys.IsEmpty() || !factionKeys.Contains(m_Faction.GetFactionKey()))
			return;
		
		m_bQueuedTaskCountChanged = true;
		GetGame().GetCallqueue().CallLater(OnTaskCountChanged);	
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls the method that sends the invoker and calls the RPC for clients for task count changes
	//! Basically we are "accumulating" task changes through the frame and then notifying about 
	//! all of them at once instead of one by one
	protected void OnTaskCountChanged()
	{		
		m_bQueuedTaskCountChanged = false;
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		array<SCR_Task> tasks = {};
		taskSystem.GetTasksByState(tasks, SCR_ETaskState.CREATED |  SCR_ETaskState.ASSIGNED |  SCR_ETaskState.PROGRESSED , m_Faction.GetFactionKey());
		
		int tasksCount = tasks.Count();
		
		//Safty as task Update is called on any change in tasks, but it should catch all the OnTaskUpdates types regardless. So if the same value is given don't do anything
		if (m_iTaskCount == tasksCount)
			return;
		
		//Update task count
		OnTaskCountChangedBroadcast(tasksCount);
		Rpc(OnTaskCountChangedBroadcast, tasksCount);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnTaskCountChangedBroadcast(int factionTaskCount)
	{		
		m_iTaskCount = factionTaskCount;
		Event_OnTaskCountChanged.Invoke(m_Faction, factionTaskCount);
	}
	
	//======================================== FACTION PLAYABLE ========================================\\

	//------------------------------------------------------------------------------------------------
	//! Set faction Playable, can only be called from the server side
	//! \param[in] factionPlayable bool for setting faction playable
	void SetFactionPlayableServer(bool factionPlayable)
	{
		//If client cancel
		if (Replication.IsClient())
			return;
		
		if (m_ScrFaction.IsPlayable() == factionPlayable)
			return;
		
		//Update enabled
		OnFactionplayableChangedBroadcast(factionPlayable);
		Rpc(OnFactionplayableChangedBroadcast, factionPlayable);	
	}
	
	//------------------------------------------------------------------------------------------------
	// Broadcast to client
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnFactionplayableChangedBroadcast(bool factionPlayable)
	{	
		m_ScrFaction.SetIsPlayable(factionPlayable, true);
		
		//--- Hide non-playable faction, so it's unselected if it was currently selected
		SetVisible(factionPlayable);
	}
	
	//======================================== GETTERS ========================================\\

	//------------------------------------------------------------------------------------------------
	//! Get Spawnpoint count of faction
	//! \return int Spawn Point count
	int GetFactionSpawnPointCount()
	{	
		return m_iSpawnPointCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Task count of faction
	//! \return int Task count
	int GetFactionTasksCount()
	{				
		return m_iTaskCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get On Spawn Point Count Changed Script Invoker
	//! \return ScriptInvoker Event_OnSpawnPointCountChanged
	ScriptInvoker GetOnSpawnPointCountChanged()
	{
		return Event_OnSpawnPointCountChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get On Task Count Changed Script Invoker
	//! \return ScriptInvoker Event_OnTaskCountChanged
	ScriptInvoker GetOnTaskCountChanged()
	{
		return Event_OnTaskCountChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemType GetAllowedArsenalItemTypes()
	{
		return m_AllowedArsenalItemTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] allowedArsenalItemTypes
	void SetAllowedArsenalItemTypes(SCR_EArsenalItemType allowedArsenalItemTypes)
	{
		m_AllowedArsenalItemTypes = allowedArsenalItemTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanDuplicate(out notnull set<SCR_EditableEntityComponent> outRecipients)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override Faction GetFaction()
	{
		return m_Faction;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetPos(out vector pos)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetEntityBudgetCost(out notnull array<ref SCR_EntityBudgetValue> outBudgets, IEntity owner = null)
	{
		// Return true and empty cost array, avoid fallback entityType cost
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		owner.SetFlags(EntityFlags.NO_LINK, true);
	}
	
	//======================================== RPL ========================================\\

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;

		writer.WriteInt(m_iFactionIndex); 
		writer.WriteInt(m_iSpawnPointCount); 
		writer.WriteInt(m_iTaskCount);
		writer.WriteBool(m_ScrFaction.IsPlayable());
		
		SCR_Faction factionRef = SCR_Faction.Cast(m_Faction);
		if (!factionRef)
		{
			writer.WriteInt(0);
			return true;
		}
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
		{
			writer.WriteInt(0);
			return true;
		}
		
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		SCR_Faction scrFaction;

		// Make sure to write the number of factions so the client doesn't
		// end up reading beyond its bounds. This shouldn't really be necessary
		// because the number of factions is set in config. However, we need
		// this for logging so we actually know where a JIP issue comes from:
		// https://jira.bistudio.com/browse/ARMA4-63824
		int factionsCount = 0;
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;

			++factionsCount;
		}
		writer.WriteInt(factionsCount);
		
		// Write if faction is hostile or friendly
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;

			bool isFriendly = scrFaction.DoCheckIfFactionFriendly(factionRef);
			writer.WriteBool(isFriendly);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!super.RplLoad(reader))
			return false;

		int spawnPointCount, taskCount;
		bool isPlayable;
		
		reader.ReadInt(m_iFactionIndex);
		reader.ReadInt(spawnPointCount);
		reader.ReadInt(taskCount);
		reader.ReadBool(isPlayable);
		
		m_ScrFaction.InitFactionIsPlayable(isPlayable);
		SetFactionIndexBroadcast(m_iFactionIndex);
		InitSpawnPointCountBroadcast(spawnPointCount);
		InitTaskCountBroadcast(taskCount);

		int factionsCntServer = 0;
		reader.ReadInt(factionsCntServer);
		if (factionsCntServer == 0)
			return true;
		
		SCR_Faction factionRef = SCR_Faction.Cast(m_Faction);
		if (!factionRef)
		{
			// Consume the rest of the bits. The server writes as many bits as there are factions.
			bool dummy;
			for (int i=0; i<factionsCntServer; ++i)
				reader.ReadBool(dummy);
				
 			return true;
		}
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
		{
			// Consume the rest of the bits. The server writes as many bits as there are factions.
			bool dummy;
			for (int i=0; i<factionsCntServer; ++i)
				reader.ReadBool(dummy);
				
 			return true;
		}
		
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		int factionManagerFactionCnt = factions.Count();
		if (factionManagerFactionCnt != factionsCntServer)
		{
			Print(string.Format("Factions count mismatch. Local:%1, server:%2", factionManagerFactionCnt, factionsCntServer), LogLevel.WARNING);

			// Consume the rest of the bits. The server writes as many bits as there are factions.
			bool dummy;
			for (int i=0; i<factionsCntServer; ++i)
				reader.ReadBool(dummy);
				
 			return true;
		}

		SCR_Faction scrFaction;
		bool isFriendly;

		// Verify faction validity.
		int factionsCnt = 0;
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;
			
			++factionsCnt;
		}

		if (factionsCnt != factionsCntServer)
		{
			Print(string.Format("SCR_Faction factions count mismatch. Local:%1, server:%2", factionsCnt, factionsCntServer), LogLevel.WARNING);

			// Consume the rest of the bits. The server writes as many bits as there are factions.
			bool dummy;
			for (int i=0; i<factionsCntServer; ++i)
				reader.ReadBool(dummy);
				
 			return true;
		}
				
		// Read if faction is hostile or friendly and set the faction to hostile or friendly
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;
			
			reader.ReadBool(isFriendly);
			
			if (isFriendly)
				factionManager.SetFactionsFriendly(factionRef, scrFaction);
			else 
				factionManager.SetFactionsHostile(factionRef, scrFaction);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_EditableFactionComponent()
	{
		if (m_Faction && Replication.IsServer())
		{
			//Spawnpoints
			SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Remove(OnSpawnPointsChanged);
			SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnpointFactionChanged);
		
			//Tasks
			SCR_TaskSystem.GetInstance().GetOnTaskAdded().Remove(OnTaskAdded);
			SCR_TaskSystem.GetInstance().GetOnTaskRemoved().Remove(OnTaskRemoved);
		}
	}
}
