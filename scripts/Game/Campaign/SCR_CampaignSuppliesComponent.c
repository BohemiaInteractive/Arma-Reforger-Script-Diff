[EntityEditorProps(category: "GameScripted/Campaign", description: "Makes a vehicle able to carry Conflict resources.", color: "0 0 255 255")]
class SCR_CampaignSuppliesComponentClass : ScriptComponentClass
{
}

//! Makes a vehicle able to carry Conflict resources
class SCR_CampaignSuppliesComponent : ScriptComponent
{
	// Member variables 
	protected SCR_CampaignMilitaryBaseComponent m_LastLoadedAt;
	protected SCR_CampaignMilitaryBaseComponent m_LastUnloadedAt;
	protected SCR_CampaignMilitaryBaseComponent m_LastXPAwardedAt;
	protected bool m_bAwardUnloadXP = true;
	protected bool m_bIsPlayerInRange;
	
	protected static const float SUPPLY_TRUCK_UNLOAD_RADIUS = 25;	 //!< m: maximum distance from a supply depot a player can still (un)load their truck
	
	[RplProp()]
	protected ref array<int> m_aLoadingPlayerIDs = {};
	
	[RplProp()]
	protected ref array<int> m_aUnloadingPlayerIDs = {};
	
	// Script Invoker
	ref ScriptInvoker m_OnSuppliesChanged = new ScriptInvoker();
	ref ScriptInvoker m_OnSuppliesTruckDeleted = new ScriptInvoker();
	
	// Synced variables
	[Attribute("0", desc: "How many supplies this component holds when it is created."), RplProp(onRplName: "OnSuppliesChanged")]
	protected int m_iSupplies;
	
	// Synced variables
	[Attribute("0", desc: "Maximum supplies this component can hold."), RplProp(onRplName: "OnSuppliesChanged")]
	protected int m_iSuppliesMax;
	
	[Attribute("0", desc: "Maximum distance from a supply depot a player can still (un)load their truck")]
	protected float m_fOperationalRadius;
	
	[Attribute("0", UIWidgets.CheckBox, "This component belongs to a supply depot without a parent base.", "")]
	bool m_bIsStandaloneDepot;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] unloading true to verify in Unloading array, otherwise it will look in loading array
	//! \return first player from loading/unloading arrays
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	int GetLoadingPlayer(bool unloading = false)
	{
		array<int> loadingArray;
		//Switch between loading and unloading players
		if(!unloading)
			loadingArray = m_aLoadingPlayerIDs;
		else
			loadingArray = m_aUnloadingPlayerIDs;
		
		//return id of first player (if there is any)
		if(!loadingArray.IsEmpty())
			return loadingArray[0];	
		else
			return 0;
	}
	//------------------------------------------------------------------------------------------------
	//! Setter for Loading players array
	//! \param[in] playerID
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void SetSupplyLoadingPlayer(int playerID)
	{
		if(!m_aLoadingPlayerIDs.Contains(playerID))
		{
			m_aLoadingPlayerIDs.Insert(playerID);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	// Deletes from loading players array
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void DeleteSupplyLoadingPlayer(int playerID)
	{
		if(m_aLoadingPlayerIDs.Contains(playerID))
		{
			m_aLoadingPlayerIDs.RemoveItemOrdered(playerID);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Called on player killed
	protected void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		/*DeleteSupplyLoadingPlayer(playerId);
		DeleteSupplyUnloadingPlayer(playerId);*/
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setter for Unloading players array
	//! \param[in] playerID
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void SetSupplyUnloadingPlayer(int playerID)
	{
		if(!m_aUnloadingPlayerIDs.Contains(playerID))
		{
			m_aUnloadingPlayerIDs.Insert(playerID);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deletes from Unloading players array
	//! \param[in] playerID
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void DeleteSupplyUnloadingPlayer(int playerID)
	{
		if(m_aUnloadingPlayerIDs.Contains(playerID))
		{
			m_aUnloadingPlayerIDs.RemoveItemOrdered(playerID);
			Replication.BumpMe();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void OnSuppliesChanged()
	{
		// script invoker executed when ammout of supplies in vehicle changed. m_iSupplies is ammout of currently loaded supplies.
		m_OnSuppliesChanged.Invoke(m_iSupplies, m_iSuppliesMax);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	int GetSupplies()
	{
		return m_iSupplies;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	int GetSuppliesMax()
	{
		return m_iSuppliesMax;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] suppliesMax
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void SetSuppliesMax(int suppliesMax)
	{
		m_iSuppliesMax = suppliesMax;
		if (m_iSupplies > suppliesMax)
			m_iSupplies = m_iSuppliesMax;
		
		Replication.BumpMe();
		OnSuppliesChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the load / unload radius. If set custom use this. if it's set to zero, use default.
	//! \return
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	float GetOperationalRadius()
	{
		if (m_fOperationalRadius != 0)
			return m_fOperationalRadius;
		
		return SUPPLY_TRUCK_UNLOAD_RADIUS;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] status
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void SetIsPlayerInRange(bool status)
	{
		m_bIsPlayerInRange = status;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	bool GetIsPlayerInRange()
	{
		return m_bIsPlayerInRange;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void SetLastLoadedAt(SCR_CampaignMilitaryBaseComponent base)
	{
		m_LastLoadedAt = base;
		
		if (m_LastLoadedAt == m_LastUnloadedAt && m_LastUnloadedAt == m_LastXPAwardedAt)
			m_bAwardUnloadXP = false;
		else
			m_bAwardUnloadXP = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void SetLastUnloadedAt(SCR_CampaignMilitaryBaseComponent base)
	{
		m_LastUnloadedAt = base;
		
		if (m_LastLoadedAt == m_LastUnloadedAt)
			m_bAwardUnloadXP = false;
		else
			m_LastXPAwardedAt = base;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	bool AwardXP()
	{
		return m_bAwardUnloadXP;
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] supplies
	//! \param[in] replicate
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	void AddSupplies(int supplies, bool replicate = true)
	{
		int oldSupplies = m_iSupplies;
		m_iSupplies += supplies;
		
		if (m_iSupplies > m_iSuppliesMax)
			m_iSupplies = m_iSuppliesMax;
		
		if (m_iSupplies < 0)
			m_iSupplies = 0;

		if (replicate && m_iSupplies != oldSupplies)
			Replication.BumpMe();
		
		OnSuppliesChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	[Obsolete("Use SCR_ResourceComponent operations instead")]
	bool GetIsStandaloneDepot()
	{
		return m_bIsStandaloneDepot;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;
		
		gamemode.GetOnPlayerDisconnected().Insert(DeleteSupplyLoadingPlayer);
		gamemode.GetOnPlayerDisconnected().Insert(DeleteSupplyUnloadingPlayer);
		gamemode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		
		if (m_bIsStandaloneDepot && GetGame().InPlayMode())
		{
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
			if (campaign)
				campaign.GetBaseManager().RegisterRemnantSupplyDepot(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		
		if (m_iSupplies > m_iSuppliesMax)
		{
			string err = string.Format("SCR_CampaignSuppliesComponent on %1 carries more supplies (%2) than its maximum (%3)!", owner, m_iSupplies, m_iSuppliesMax);
			Print(err, LogLevel.ERROR);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] ent
	//! \return supplies component on the provided entity or null if cannot be found
	static SCR_CampaignSuppliesComponent GetSuppliesComponent(notnull IEntity ent)
	{
		SCR_CampaignSuppliesComponent suppliesComponent;
		
		suppliesComponent = SCR_CampaignSuppliesComponent.Cast(ent.FindComponent(SCR_CampaignSuppliesComponent));
		if (suppliesComponent)
			return suppliesComponent;
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(ent.FindComponent(SlotManagerComponent));	
		if (!slotManager)
			return null;
	
		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);
		IEntity truckBed;
		
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;
			
			truckBed = slot.GetAttachedEntity();
			
			if (!truckBed)
				continue;
			
			suppliesComponent = SCR_CampaignSuppliesComponent.Cast(truckBed.FindComponent(SCR_CampaignSuppliesComponent));
			if (suppliesComponent)
				return suppliesComponent;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CampaignSuppliesComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CampaignSuppliesComponent()
	{
		if (m_OnSuppliesTruckDeleted)
			m_OnSuppliesTruckDeleted.Invoke(GetOwner());
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;
		
		gamemode.GetOnPlayerDisconnected().Remove(DeleteSupplyLoadingPlayer);
		gamemode.GetOnPlayerDisconnected().Remove(DeleteSupplyUnloadingPlayer);
		gamemode.GetOnPlayerKilled().Remove(OnPlayerKilled);
	}
}
