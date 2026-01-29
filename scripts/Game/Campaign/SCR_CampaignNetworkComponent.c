#define ENABLE_BASE_DESTRUCTION
[EntityEditorProps(category: "GameScripted/Campaign", description: "Handles client > server communication in Campaign. Should be attached to PlayerController.", color: "0 0 255 255")]
class SCR_CampaignNetworkComponentClass : ScriptComponentClass
{
}

//! Used to identify various notifications for client
enum ECampaignClientNotificationID
{
	VEHICLE_SPAWNED,
	NO_SPACE,
	OUT_OF_STOCK,
	SUPPLIES_LOADED,
	SUPPLIES_UNLOADED,
	RESPAWN
}

void OnBaseCapturedDelegate(SCR_CampaignMilitaryBaseComponent base, int playerId);
typedef func OnBaseCapturedDelegate;

//~ Supplies transfer invoker
void ScriptInvokerTransferSuppliesMethod(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue);
typedef func ScriptInvokerTransferSuppliesMethod;
typedef ScriptInvokerBase<ScriptInvokerTransferSuppliesMethod> ScriptInvokerTransferSupplies;

//! Takes care of Campaign-specific server <> client communication and requests
class SCR_CampaignNetworkComponent : ScriptComponent
{
	// Member variables 
	protected SCR_PlayerController m_PlayerController;
	protected RplComponent m_RplComponent;
	protected bool m_bFirstSpawn = true;

	protected vector m_vLastLoadedAt;
	protected SCR_ResourceComponent m_LastLoadedComponent;
	protected float m_fLoadedSupplyAmount;
	protected bool m_bOrphanSuppliesLoaded;

	protected float m_fNoRewardSupplies;

	protected int m_iTotalSuppliesDelivered;

	protected static ref ScriptInvokerInt3 s_OnSuppliesDelivered;
	protected static ref ScriptInvokerBase<OnBaseCapturedDelegate> s_OnBaseCaptured; // <base, playerID>
	protected static ref ScriptInvokerTransferSupplies s_OnSuppliesTransferred;

	static const int SUPPLY_DELIVERY_XP_PERCENT = 100; // Decimal fraction used in formula to calculate XP reward
	static const int SUPPLY_DELIVERY_THRESHOLD_SQ = 200 * 200;

	//********************************//
	//RUNTIME SYNCHED MEMBER VARIABLES//
	//********************************//
	
	[RplProp(condition: RplCondition.OwnerOnly)]
	protected WorldTimestamp m_fLastAssetRequestTimestamp;

	[RplProp(condition: RplCondition.OwnerOnly)]
	protected WorldTimestamp m_fLastHQRadioMessageTimestamp;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return
	static SCR_CampaignNetworkComponent GetCampaignNetworkComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		
		if (!playerController)
			return null;
		
		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		
		return networkComponent;
	}
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	//! \return request cooldown
	WorldTimestamp GetLastRequestTimestamp()
	{
		return m_fLastAssetRequestTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceComponent GetLastLoadedComponent()
	{
		return m_LastLoadedComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] request cooldown timestamp
	void SetLastRequestTimestamp(WorldTimestamp timestamp)
	{
		if (IsProxy())
			return;
		
		m_fLastAssetRequestTimestamp = timestamp;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerInt3 GetOnSuppliesDelivered()
	{
		if (!s_OnSuppliesDelivered)
			s_OnSuppliesDelivered = new ScriptInvokerInt3();

		return s_OnSuppliesDelivered;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerTransferSupplies GetOnTransferSupplies()
	{
		if (!s_OnSuppliesTransferred)
			s_OnSuppliesTransferred = new ScriptInvokerTransferSupplies();
 
		return s_OnSuppliesTransferred;
	}

	//------------------------------------------------------------------------------------------------
	//! Get event called when the player captures a base.
	//! Invoker params are: SCR_CampaignMilitaryBaseComponent base, int playerId
	//! \return ScriptInvokerBase<OnBaseCapturedDelegate>
	static ScriptInvokerBase<OnBaseCapturedDelegate> GetOnBaseCaptured()
	{
		if (!s_OnBaseCaptured)
			s_OnBaseCaptured = new ScriptInvokerBase<OnBaseCapturedDelegate>();

		return s_OnBaseCaptured;
	}

	//------------------------------------------------------------------------------------------------
	//! \return if the session is run as client
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Repair destroyed mandatory part of composition
	void RepairComposition(int index, int repairCost, int destructibleID, SCR_SiteSlotEntity slotEnt, notnull SCR_CampaignMilitaryBaseComponent base)
	{
#ifdef ENABLE_BASE_DESTRUCTION
		Rpc(RpcAsk_RepairComposition, index, repairCost, destructibleID, Replication.FindId(slotEnt), Replication.FindId(base));
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send server request to load supplies at a base
	//! \param[in] suppliesID Unique ID identifying the supplies
	//! \param[in] player Player trying to unload supplies
	//! \param[in] base Base at which the supplies are being unloaded
	//! \param[in] amount
	void LoadSupplies(RplId suppliesID, IEntity player, SCR_CampaignMilitaryBaseComponent base, int amount = 0)
	{
		if (!player || !base)
			return;
		
		RplId baseID = Replication.FindId(base);
		
		if (!m_PlayerController)
			return;
		
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_LoadSupplies, suppliesID, playerID, baseID, amount);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] suppliesID
	//! \param[in] player
	//! \param[in] depot
	//! \param[in] amount
	void LoadSuppliesStandalone(RplId suppliesID, IEntity player, SCR_CampaignSuppliesComponent depot, int amount = 0)
	{
		if (!player || !depot)
			return;
		
		RplId depotID = Replication.FindId(depot);
		
		if (!m_PlayerController)
			return;
		
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_LoadSuppliesStandalone, suppliesID, playerID, depotID, amount);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] suppliesID
	//! \param[in] supplies
	//! \param[in] IsUnloading
	void StartLoading(RplId suppliesID, int supplies, bool IsUnloading = false)
	{
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_SuppliesLoadingStarted, suppliesID, playerID, supplies, IsUnloading);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] suppliesID
	//! \param[in] IsUnloading
	void StopLoading(RplId suppliesID, bool IsUnloading = false)
	{
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_SuppliesLoadingCanceled, suppliesID, playerID, IsUnloading);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send server request to unload supplies at a base
	//! \param[in] truckID Unique ID identifying the supply truck
	//! \param[in] player Player trying to unload supplies
	//! \param[in] base Base at which the supplies are being unloaded
	void UnloadSupplies(RplId suppliesID, IEntity player, SCR_CampaignMilitaryBaseComponent base, int amount = 0)
	{
		if (!player || !base)
			return;
		
		RplId baseID = Replication.FindId(base);
		
		if (!m_PlayerController)
			return;
		
		int playerID = m_PlayerController.GetPlayerId();
		Rpc(RpcAsk_UnloadSupplies, suppliesID, playerID, baseID, amount);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void AddRadio()
	{
		Rpc(RpcAsk_AddRadio, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddRadio(int playerID)
	{
		PlayerManager pMan = GetGame().GetPlayerManager();
		
		if (!pMan)
			return;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pMan.GetPlayerControlledEntity(playerID));
		
		if (!player)
			return;
		
		EntitySpawnParams spawnParams = EntitySpawnParams();	
		spawnParams.TransformMode = ETransformMode.WORLD;
		player.GetWorldTransform(spawnParams.Transform);
		
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(player.GetFaction());
		
		if (!faction)
			return;
		
		Resource res = Resource.Load(faction.GetRadioPrefab());
		
		if (!res)
			return;
		
		IEntity radio = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(),spawnParams);
		
		if (!radio)
			return;
		
		RplComponent rplC = RplComponent.Cast(radio.FindComponent(RplComponent));
		
		if (!rplC)
			return;
		
		Rpc(RpcDo_AddRadio, rplC.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_AddRadio(RplId ID)
	{
		GetGame().GetCallqueue().CallLater(FindRadioDelayed, 10, true, ID)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FindRadioDelayed(RplId ID)
	{
		RplComponent rplC = RplComponent.Cast(Replication.FindItem(ID));
		
		if (!rplC)
			return;
		
		GetGame().GetCallqueue().Remove(FindRadioDelayed);
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		
		if (!player)
			return;
		
		IEntity radio = rplC.GetEntity();
		
		if (!radio)
			return;
		
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!inventory)
			return;
			
		inventory.InsertItem(radio);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] comp
	//! \param[in] deploy
	void DeployMobileAsembly(notnull SCR_CampaignMobileAssemblyComponent comp, bool deploy)
	{
		Rpc(RpcAsk_DeployMobileAsembly, Replication.FindId(comp), deploy, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_DeployMobileAsembly(RplId assemblyComponentID, bool deploy, int playerID)
	{
		SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(Replication.FindItem(assemblyComponentID));
		
		if (!comp)
			return;
		
		float depth;
		
		if (SCR_WorldTools.IsObjectUnderwater(comp.GetOwner(), vector.Zero, -1, depth) && depth > SCR_CampaignMobileAssemblyComponent.MAX_WATER_DEPTH)
			return;
		
		if (comp.IsDeployed() == deploy)
			return;
		
		if (deploy)
			comp.Deploy(SCR_EMobileAssemblyStatus.DEPLOYED, playerID);
		else
			comp.Deploy(SCR_EMobileAssemblyStatus.DISMANTLED, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send server request to capture a base (change its owner)
	//! \param[in] base Base to be captured
	void CaptureBase(SCR_CampaignMilitaryBaseComponent base)
	{
		if (!base)
			return;
		
		Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
		int factionID = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetFactionIndex(faction);
		int playerID = m_PlayerController.GetPlayerId();
		IEntity player = m_PlayerController.GetControlledEntity();
		
		if (!player)
			return;
		
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
		
		if (!comp)
			return;
		
		if (comp.IsDead())
			return;
		
		Rpc(RpcAsk_CaptureBase, Replication.FindId(base), factionID, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] base
	//! \param[in] factionIndex
	void CaptureBaseGM(SCR_CampaignMilitaryBaseComponent base, int factionIndex)
	{
		if (!base)
			return;
		
		Rpc(RpcAsk_CaptureBase, Replication.FindId(base), factionIndex, SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] base
	//! \param[in] isBeingCaptured
	void ToggleBaseCapture(notnull SCR_CampaignMilitaryBaseComponent base, bool isBeingCaptured)
	{
		Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
		int factionID = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetFactionIndex(faction);
		int playerID = m_PlayerController.GetPlayerId();
		
		if (isBeingCaptured)
			Rpc(RpcAsk_CaptureBaseBegin, Replication.FindId(base), factionID, playerID);
		else
			Rpc(RpcAsk_CaptureBaseEnd, Replication.FindId(base));
	}
	
	//***********//
	//RPC METHODS//
	//***********//
	
#ifdef ENABLE_BASE_DESTRUCTION
	//------------------------------------------------------------------------------------------------
	//! Repair damaged entity in composition
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RepairComposition(int index, int repairCost, int destructibleID, RplId slotID, RplId baseID)
	{
		if (index == -1 || repairCost == -1 || destructibleID == -1)
			return;
		
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		SCR_DestructionMultiPhaseComponent destructibleComp = SCR_DestructionMultiPhaseComponent.Cast(destructionManager.FindDynamicallySpawnedDestructibleByIndex(destructibleID, index));
		if (!destructibleComp)
			return;
				
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		if (!base)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;
			
		IEntity composition = SCR_EntityHelper.GetMainParent(destructibleComp.GetOwner());
		if (!composition)
			return;
		
		SCR_CampaignServiceEntityComponent serviceEntityComp = SCR_CampaignServiceEntityComponent.Cast(destructibleComp.GetOwner().FindComponent(SCR_CampaignServiceEntityComponent));
		if (!serviceEntityComp)
			return; 
		
		SCR_CampaignServiceCompositionComponent serviceCompositionComp = SCR_CampaignServiceCompositionComponent.Cast(composition.FindComponent(SCR_CampaignServiceCompositionComponent));
		if (!serviceCompositionComp)
			return;	
		
		// Check if the composition entity belong to is disabled or not. If so, increase number of spawn tickets again.
//		if (!serviceCompositionComp.IsServiceOperable())
//			campaign.OnStructureChanged(base, SCR_SiteSlotEntity.Cast(Replication.FindItem(slotID)), base.GetServiceByLabel(serviceCompositionComp.GetCompositionType()), true);
		
		// Repair entity
		serviceEntityComp.RepairEntity();
		
		// Supply in base are reduced (cost of repair)
		base.AddSupplies(-repairCost);
		
		// Update map UI
		if (RplSession.Mode() != RplMode.Dedicated)
			base.GetMapDescriptor().HandleMapInfo();
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] base
	//! \param[in] suppliesCnt
	void AddSuppliesFromContextMenu(notnull SCR_CampaignMilitaryBaseComponent base, int suppliesCnt)
	{
		RplId baseID = Replication.FindId(base);
		
		if (!baseID.IsValid())
			return;
		
		Rpc(RpcAsk_AddSuppliesFromContextMenu, baseID, suppliesCnt);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddSuppliesFromContextMenu(RplId baseID, int suppliesCnt)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		if (base.GetSupplies() >= base.GetSuppliesMax())
			return;
		
		base.AddSupplies(Math.Min(suppliesCnt, base.GetSuppliesMax() - base.GetSupplies()));
	}
	//------------------------------------------------------------------------------------------------
	// Sends player notification to players inside Vehicle
	//! \param[in] messageID
	//! \param[in] vehicleEntity entity of vehicle
	//! \param[in] playerID ID of player to notify about
	//! \param[in] number optional second parameter for another int number
	protected void SendToVehicleOccupants(ENotification messageID, IEntity vehicleEntity, int playerID, int number = 0)
	{
		if (!vehicleEntity)
			return;

		IEntity parentVehicle = vehicleEntity.GetParent();
		if (!parentVehicle)
			return;
		
		//Gettings players from inside of vehicle. Condition allows spawning only Cargo.
		SCR_BaseCompartmentManagerComponent comp = SCR_BaseCompartmentManagerComponent.Cast(parentVehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!comp)
			return;

		array<IEntity> occupants = {};
		comp.GetOccupants(occupants);
		
		if(occupants.IsEmpty())
			return;
		
		foreach (IEntity occupant : occupants)
		{
			int occupantID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(occupant);
			if(number != 0)
				SCR_NotificationsComponent.SendToPlayer(occupantID, messageID, playerID, number);
			else
				SCR_NotificationsComponent.SendToPlayer(occupantID, messageID, playerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Handles start of loading/unloading supplies
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SuppliesLoadingStarted(RplId suppliesID, int playerID, int supplies, bool IsUnloading)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));
		array<IEntity> occupants = {}; //new array<IEntity>;
		
		if(!suppliesComponent)
			return;
		
		//Gettings vehicle entity from cargo
		IEntity vehicleEntity = suppliesComponent.GetOwner();
		
		if(!IsUnloading)
		{
			//suppliesComponent.SetSupplyLoadingPlayer(playerID);
			SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_LOADING_PLAYER, vehicleEntity, playerID, supplies);
		}
		else
		{
			//suppliesComponent.SetSupplyUnloadingPlayer(playerID);
			SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_UNLOADING_PLAYER, vehicleEntity, playerID, supplies);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Handles canceling of loading/unloading supplies
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SuppliesLoadingCanceled(RplId suppliesID, int playerID, bool IsUnloading)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));
		
		if(!suppliesComponent)
			return;
		
		//Gettings vehicle entity from cargo
		IEntity vehicleEntity = suppliesComponent.GetOwner();
		
		if(!IsUnloading)
		{
			//suppliesComponent.DeleteSupplyLoadingPlayer(playerID);
			SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_LOADING_PLAYER_STOPPED, vehicleEntity, playerID);
		}
		else
		{
			//suppliesComponent.DeleteSupplyUnloadingPlayer(playerID);
			SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_UNLOADING_PLAYER_STOPPED, vehicleEntity, playerID);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Load supplies at a base
	//! \param[in] suppliesID supplies entity ID
	//! \param[in] playerID player entity ID
	//! \param[in] baseID base entity ID
	//! \param[in] amount
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LoadSupplies(RplId suppliesID, int playerID, RplId baseID, int amount)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));

		if (!suppliesComponent)
			return;

		IEntity box = suppliesComponent.GetOwner();
		
		if (!box)
			return;

		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		if (amount > base.GetSupplies())
			return;

		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		if (!player)
			return;
		
//		if (suppliesComponent.GetSupplies() == suppliesComponent.GetSuppliesMax())
//			return;
		
		Faction playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(player);
		
		if (!playerFaction || playerFaction != base.GetFaction())
			return;
		
		SCR_CampaignSuppliesComponent baseSuppliesComponent = SCR_CampaignSuppliesComponent.Cast(base.GetOwner().FindComponent(SCR_CampaignSuppliesComponent));
		if (!baseSuppliesComponent)
			return;

//		float distSq = Math.Pow(baseSuppliesComponent.GetOperationalRadius(), 2);
//		vector vehPos = box.GetOrigin();
//
//		if (vector.DistanceSq(vehPos, player.GetOrigin()) > 100)
//			return;
//
//		if (vector.DistanceSq(vehPos, base.GetOwner().GetOrigin()) > distSq)
//		{
//			SCR_ServicePointComponent service = base.GetServiceByType(SCR_EServicePointType.SUPPLY_DEPOT);
//			if (!service)
//				return;
//
//			if (vector.DistanceSq(vehPos, service.GetOwner().GetOrigin()) > distSq)
//				return;
//		}
//
//		// Validity check passed, perform action
//		int finalAmount = Math.Min(suppliesComponent.GetSuppliesMax() - suppliesComponent.GetSupplies(), amount);
//		suppliesComponent.AddSupplies(finalAmount);
//		base.AddSupplies(-finalAmount);
//		suppliesComponent.SetLastLoadedAt(base);
//		Rpc(RpcDo_PlayerFeedbackValueBase, ECampaignClientNotificationID.SUPPLIES_LOADED, (float)finalAmount, base.GetCallsign());
//		suppliesComponent.DeleteSupplyLoadingPlayer(playerID);
		
		SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_LOADING_PLAYER_FINISHED, box, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_LoadSuppliesStandalone(RplId suppliesID, int playerID, RplId depotID, int amount)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));

		if (!suppliesComponent)
			return;
		
		SCR_CampaignSuppliesComponent depot = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(depotID));
		
		if (!depot)
			return;
		
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		if (!player)
			return;
		
		IEntity box = suppliesComponent.GetOwner();
		
		if (!box)
			return;
		
//		if (amount > depot.GetSupplies())
//			return;
//
//		if (suppliesComponent.GetSupplies() == suppliesComponent.GetSuppliesMax())
//			return;
//
//		float distSq = Math.Pow(depot.GetOperationalRadius(), 2);
//		vector vehPos = box.GetOrigin();
//
//		if (vector.DistanceSq(vehPos, depot.GetOwner().GetOrigin()) > distSq || vector.DistanceSq(vehPos, player.GetOrigin()) > 100)
//			return;
//
//		// Validity check passed, perform action
//		int finalAmount = Math.Min(suppliesComponent.GetSuppliesMax() - suppliesComponent.GetSupplies(), amount);
//		suppliesComponent.AddSupplies(finalAmount);
//		Rpc(RpcDo_PlayerFeedbackValueBase, ECampaignClientNotificationID.SUPPLIES_LOADED, (float)finalAmount, -1);
//		suppliesComponent.DeleteSupplyLoadingPlayer(playerID);
		SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_LOADING_PLAYER_FINISHED, box, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unload supplies at a base
	//! \param[in] vehicleID Vehicle entity ID
	//! \param[in] playerID Vehicle entity ID
	//! \param[in] baseID Vehicle entity ID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UnloadSupplies(RplId suppliesID, int playerID, RplId baseID, int amount)
	{
		SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(Replication.FindItem(suppliesID));

		if (!suppliesComponent)
			return;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		if (!player)
			return;
		
		IEntity box = suppliesComponent.GetOwner();
		
		if (!box)
			return;
		
//		if (amount > suppliesComponent.GetSupplies())
//			return;
//
//		Faction playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(player);
//		SCR_CampaignFaction owningFaction = base.GetCampaignFaction();
//
//		if (!playerFaction || playerFaction != owningFaction)
//			return;
//
//		SCR_CampaignSuppliesComponent baseSuppliesComponent = SCR_CampaignSuppliesComponent.Cast(base.GetOwner().FindComponent(SCR_CampaignSuppliesComponent));
//		if (!baseSuppliesComponent)
//			return;
//
//		float distSq = Math.Pow(baseSuppliesComponent.GetOperationalRadius(), 2);
//		vector vehPos = box.GetOrigin();
//
//		if (vector.DistanceSq(vehPos, player.GetOrigin()) > 100)
//			return;
//
//		if (vector.DistanceSq(vehPos, base.GetOwner().GetOrigin()) > distSq)
//		{
//			SCR_ServicePointComponent service = base.GetServiceByType(SCR_EServicePointType.SUPPLY_DEPOT);
//			if (!service)
//				return;
//
//			if (vector.DistanceSq(vehPos, service.GetOwner().GetOrigin()) > distSq)
//				return;
//		}
//
//		// Validity check passed, perform action
//		int suppliesCur = base.GetSupplies();
//		int suppliesMax = base.GetSuppliesMax();
//		int suppliesCnt = Math.Min(amount, suppliesMax - suppliesCur);
//		float rewardMultiplier = suppliesCnt / suppliesComponent.GetSuppliesMax();
//		suppliesComponent.AddSupplies(-suppliesCnt);
//		base.AddSupplies(suppliesCnt);
//		suppliesComponent.SetLastUnloadedAt(base);
//		Rpc(RpcDo_PlayerFeedbackValueBase, ECampaignClientNotificationID.SUPPLIES_UNLOADED, (float)suppliesCnt, base.GetCallsign());
//
//		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
//
//		// Award XP unless the truck was just loaded in this base
//		// ... or if it was both loaded and unloaded in the previous base
//		// (handled in suppliesComponent)
//		if (compXP && suppliesComponent.AwardXP())
//			compXP.AwardXP(playerID, SCR_EXPRewards.SUPPLIES_DELIVERED, rewardMultiplier);
//
//		SendPlayerMessage(SCR_ERadioMsg.SUPPLIES, base.GetCallsign(), public: false);
//		suppliesComponent.DeleteSupplyUnloadingPlayer(playerID);
		
		SendToVehicleOccupants(ENotification.SUPPLY_TRUCK_UNLOADING_PLAYER_FINISHED, box, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CaptureBaseBegin(RplId baseID, int factionIndex, int playerID)
	{
		SCR_CampaignFaction faction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetCampaignFactionByIndex(factionIndex);
		
		if (!faction)
			return;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		base.BeginCapture(faction, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CaptureBaseEnd(RplId baseID)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		base.EndCapture();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] msgType
	//! \param[in] baseCallsign
	//! \param[in] calledID
	//! \param[in] public
	//! \param[in] param
	//! \param[in] checkHQReached
	void SendPlayerMessage(SCR_ERadioMsg msgType, int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN, int calledID = SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX, bool public = true, int param = SCR_CampaignRadioMsg.INVALID_RADIO_MSG_PARAM, bool checkHQReached = false)
	{
		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		if (!fManager)
			return;
		
		if (!m_PlayerController)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CallsignManagerComponent callsignManager = SCR_CallsignManagerComponent.Cast(campaign.FindComponent(SCR_CallsignManagerComponent));
		
		if (!callsignManager)
			return;
		
		IEntity player = m_PlayerController.GetMainEntity();
		
		if (!player)
			return;

		int companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller;
		
		if (!callsignManager.GetEntityCallsignIndexes(player, companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller))
    		return;
		
		SCR_GadgetManagerComponent gadgetMan = SCR_GadgetManagerComponent.Cast(player.FindComponent(SCR_GadgetManagerComponent));
		
		if (!gadgetMan)
			return;
		
		IEntity radioEnt = gadgetMan.GetGadgetByType(EGadgetType.RADIO);
		
		if (!radioEnt)
			return;
		
		BaseRadioComponent radio = BaseRadioComponent.Cast(radioEnt.FindComponent(BaseRadioComponent));
		
		if (!radio || !radio.IsPowered())
			return;
		
		BaseTransceiver transmitter = radio.GetTransceiver(0);
		
		if (!transmitter)
			return;

		IEntity called = GetGame().GetPlayerManager().GetPlayerControlledEntity(calledID);
		
		int factionId = fManager.GetFactionIndex(fManager.GetPlayerFaction(m_PlayerController.GetPlayerId()));
		
		SCR_CampaignRadioMsg msg = new SCR_CampaignRadioMsg();
		msg.SetRadioMsg(msgType);
		msg.SetFactionId(factionId);
		msg.SetBaseCallsign(baseCallsign);
		msg.SetCallerCallsign(companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller);
		msg.SetIsPublic(public);
		msg.SetParam(param);
		msg.SetPlayerID(m_PlayerController.GetPlayerId());
		msg.SetEncryptionKey(radio.GetEncryptionKey());
		
		int companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled;
		
		if (called && callsignManager.GetEntityCallsignIndexes(called, companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled))
			msg.SetCalledCallsign(companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled);

		Rpc(RpcDo_PlayRadioMsg, msgType, factionId, baseCallsign, CompressCallsign(companyCallsignIndexCaller, platoonCallsignIndexCaller, squadCallsignIndexCaller, characterCallsignIndexCaller), CompressCallsign(companyCallsignIndexCalled, platoonCallsignIndexCalled, squadCallsignIndexCalled, characterCallsignIndexCalled), param, msg.GetSeed(), 1.0);
		
		if (public)
			transmitter.BeginTransmission(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] time
	void SetLastHQRadioMessageTimestamp(WorldTimestamp time)
	{
		m_fLastHQRadioMessageTimestamp = time;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	protected int CompressCallsign(int company, int platoon, int squad, int character)
	{
		return (company * 1000000) + (platoon * 10000) + (squad * 100) + character;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DecompressCallsign(int callsign, out int company, out int platoon, out int squad, out int character)
	{
		company = Math.Floor(callsign * 0.000001);
		callsign = callsign - (company * 1000000);
		
		platoon = Math.Floor(callsign * 0.0001);
		callsign = callsign - (platoon * 10000);
		
		squad = Math.Floor(callsign * 0.01);
		callsign = callsign - (squad * 100);

		character = callsign;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Capture a base (change its owner)
	//! \param[in] baseID Base entity ID
	//! \param[in] factionIndex Index of new faction to own the base
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CaptureBase(RplId baseID, int factionIndex, int playerID)
	{
		SCR_CampaignFaction faction = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager()).GetCampaignFactionByIndex(factionIndex);
		
		if (!faction)
			return;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(baseID));
		
		if (!base)
			return;
		
		if (base.BeginCapture(faction, playerID))
			base.SetFaction(faction);
		
		if (base.GetType() == SCR_ECampaignBaseType.RELAY && playerID != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
			SendPlayerMessage(SCR_ERadioMsg.RELAY);

		if (s_OnBaseCaptured)
			s_OnBaseCaptured.Invoke(base, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param[in] msgID Message ID (see ECampaignClientNotificationID)
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedback(int msgID)
	{
		PlayerFeedbackImpl(msgID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param[in] msgID Message ID (see ECampaignClientNotificationID)
	//! \param[in] value
	//! \param[in] baseID
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedbackValueBase(int msgID, float value, int baseID)
	{
		// Short delay so replicated values have time to catch up on client's machine
		GetGame().GetCallqueue().CallLater(PlayerFeedbackImpl, SCR_GameModeCampaign.MINIMUM_DELAY, false, msgID, value, -1, baseID);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param[in] msgID Message ID (see ECampaignClientNotificationID)
	//! \param[in] baseID
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayerFeedbackBase(int msgID, int baseID)
	{
		PlayerFeedbackImpl(msgID, 0, -1, baseID);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	//! \param[in] msgID Message ID (see ECampaignClientNotificationID)
	//! \param[in] value
	//! \param[in] assetID
	//! \param[in] baseID
	protected void PlayerFeedbackImpl(int msgID, float value = 0, int assetID = -1, int baseID = -1)
	{
		LocalizedString msg;
		LocalizedString msg2;
		int duration = 2;
		int prio = -1;
		string msg1param1;
		string msg2param1;
		string msg2param2;
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		SCR_CampaignMilitaryBaseComponent base = campaign.GetBaseManager().FindBaseByCallsign(baseID);
		SCR_CampaignFeedbackComponent manager = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!campaign)
			return;
		
		switch (msgID)
		{	
			case ECampaignClientNotificationID.SUPPLIES_LOADED:
			{
				msg = "#AR-Campaign_SuppliesLoaded-UC";
				duration = 6;
				prio = SCR_ECampaignPopupPriority.SUPPLIES_HANDLED;
				msg1param1 = value.ToString();
				
				if (base)
				{
					msg2 = "#AR-Campaign_SuppliesAmountInfo-UC";
					msg2param1 = base.GetBaseName();
					msg2param2 = base.GetSupplies().ToString();
				}
				
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_LOADSUPPLIES);
				break;
			}

			case ECampaignClientNotificationID.SUPPLIES_UNLOADED:
			{
				msg = "#AR-Campaign_SuppliesUnloaded-UC";
				duration = 6;
				prio = SCR_ECampaignPopupPriority.SUPPLIES_HANDLED;
				msg1param1 = value.ToString();
				
				if (base)
				{
					msg2 = "#AR-Campaign_SuppliesAmountInfo-UC";
					msg2param1 = base.GetBaseName();
					msg2param2 = base.GetSupplies().ToString();
				}
				
				if (!campaign.IsTutorial())
				{
					if (manager)
						manager.ShowHint(EHint.CONFLICT_BUILDING);
				}
				
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_UNLOADSUPPLIES);
				break;
			}

			case ECampaignClientNotificationID.RESPAWN:
			{
				//manager.SetIsPlayerInRadioRange(true);
				if (manager)
					manager.OnRespawn();
				
				if (!base)
					return;

				msg = base.GetBaseNameUpperCase();
				ChimeraWorld world = m_PlayerController.GetWorld();
				TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
				
				if (timeManager)
				{
					int hours;
					int minutes;
					int seconds;
					timeManager.GetHoursMinutesSeconds(hours, minutes, seconds);
					string strHours = hours.ToString();
					
					if (hours > 0 && hours < 10)
						strHours = "0" + strHours;
					
					string strMinutes = minutes.ToString();
					
					if (minutes < 10)
						strMinutes = "0" + strMinutes;
					
					msg = string.Format("%1, %2:%3", msg, strHours, strMinutes);
				}
				
				msg2 = SCR_TextsTaskManagerComponent.TASK_HINT_TEXT;
				msg2param1 = SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT;
				duration = 5;
				prio = SCR_ECampaignPopupPriority.RESPAWN;
				
				if (m_bFirstSpawn)
				{
					m_bFirstSpawn = false;
					duration = 15;
				}
				
				break;
			};

			default:
			{
				return;
			};
		}
		
		SCR_PopUpNotification.GetInstance().PopupMsg(msg, duration, msg2, param1: msg1param1, text2param1: msg2param1, text2param2: msg2param2);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hintID
	void SendVehicleSpawnHint(int hintID)
	{
		Rpc(RpcDo_VehicleSpawnHint, hintID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_VehicleSpawnHint(int hintID)
	{
		SCR_CampaignFeedbackComponent feedbackComponent = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!feedbackComponent)
			return;
		
		feedbackComponent.ShowHint(hintID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] baseID
	void RespawnLocationPopup(int baseID)
	{
		Rpc(RpcDo_PlayerFeedbackBase, ECampaignClientNotificationID.RESPAWN, baseID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] alive
	void OnPlayerAliveStateChanged(bool alive)
	{
		IEntity player = m_PlayerController.GetControlledEntity();
		if (alive && player)
		{
			Rpc(RpcDo_UpdatePlayerSpawnHint, alive, player.GetOrigin());	
		}
		else
		{
			Rpc(RpcDo_UpdatePlayerSpawnHint, false, vector.Zero);	
		}

		if (!player)
			return;

		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.Cast(GetOwner().FindComponent(SCR_CampaignFeedbackComponent));
		if (!comp)
			return;

		EventHandlerManagerComponent eventHandlerManagerComponent = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManagerComponent)
			return;

		if (alive)
		{
			comp.OnConsciousnessChanged(true);
			eventHandlerManagerComponent.RegisterScriptHandler("OnConsciousnessChanged", comp, comp.OnConsciousnessChanged);
		}
		else
		{
			eventHandlerManagerComponent.RemoveScriptHandler("OnConsciousnessChanged", comp, comp.OnConsciousnessChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_UpdatePlayerSpawnHint(bool show, vector position)
	{
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
		comp.UpdatePlayerSpawnHint(show, position);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleRadioRespawnTimer(RplId spawnPointId)
	{
		SCR_PlayerRadioSpawnPointCampaign spawnpoint = SCR_PlayerRadioSpawnPointCampaign.Cast(SCR_SpawnPoint.GetSpawnPointByRplId(spawnPointId));
		
		if (!spawnpoint)
			return;

		Rpc(RpcAsk_HandleRadioRespawnTimer, spawnPointId);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_HandleRadioRespawnTimer(RplId selectedSpawnPointId)
	{
		if (!m_PlayerController)
			return;
		
		SCR_PlayerRadioSpawnPointCampaign spawnpoint = SCR_PlayerRadioSpawnPointCampaign.Cast(SCR_SpawnPoint.GetSpawnPointByRplId(selectedSpawnPointId));
		
		if (!spawnpoint)
			return;
		
		IEntity operator = GetGame().GetPlayerManager().GetPlayerControlledEntity(spawnpoint.GetPlayerID());
		
		if (!operator)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		
		if (!fManager)
			return;
		
		SCR_TimedSpawnPointComponent timer = SCR_TimedSpawnPointComponent.Cast(campaign.FindComponent(SCR_TimedSpawnPointComponent));
		
		if (!timer)	
			return;
		
		int playerId = m_PlayerController.GetPlayerId();
		float suicidePenalty;
		
		SCR_CampaignClientData data = campaign.GetClientData(playerId);
		if (data)
			suicidePenalty = data.GetRespawnPenalty();
		
		timer.SetRespawnTime(playerId, fManager.GetRankRadioRespawnCooldown(SCR_CharacterRankComponent.GetCharacterRank(operator)) + suicidePenalty);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Play task notification msg
	//! \param[in] taskNotification
	//! \param[in] msg
	//! \param[in] factionId
	//! \param[in] baseCallsign
	//! \param[in] callerGroupId
	//! \param[in] calledGroupId
	//! \param[in] grid
	//! \param[in] quality
	//! \param[in] playerID
	void PlayTaskNotificationMsg(SCR_ETaskNotification taskNotification, SCR_ETaskNotificationMsg msg, int factionId, int baseCallsign, int callerGroupId, int calledGroupId, int grid, float quality, int playerID)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return;

		if (playerID == 0 || playerID > 0 && playerID == m_PlayerController.GetPlayerId())
			Rpc(RpcDo_PlayTaskNotificationMsg, taskNotification, msg, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_PlayTaskNotificationMsg(SCR_ETaskNotification taskNotification, SCR_ETaskNotificationMsg msg, int factionId, int baseCallsign, int callerGroupId, int calledGroupId, int grid, float quality)
	{
		SCR_TaskNotificationManagerComponent taskNotificationManager = SCR_TaskNotificationManagerComponent.GetInstance();
		if (!taskNotificationManager)
			return;

		taskNotificationManager.PlayTaskNotificationMsg(taskNotification, msg, factionId, baseCallsign, callerGroupId, calledGroupId, grid, quality);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] msg
	//! \param[in] FactionId
	//! \param[in] baseCallsign
	//! \param[in] callerCallsignCompany
	//! \param[in] callerCallsignPlatoon
	//! \param[in] callerCallsignSquad
	//! \param[in] callerCallsignCharacter
	//! \param[in] calledCallsignCompany
	//! \param[in] calledCallsignPlatoon
	//! \param[in] calledCallsignSquad
	//! \param[in] calledCallsignCharacter
	//! \param[in] isPublic
	//! \param[in] param
	//! \param[in] seed
	//! \param[in] quality
	//! \param[in] playerID
	void PlayRadioMsg(SCR_ERadioMsg msg, int FactionId, int baseCallsign, int callerCallsignCompany, int callerCallsignPlatoon, int callerCallsignSquad, int callerCallsignCharacter, int calledCallsignCompany, int calledCallsignPlatoon, int calledCallsignSquad, int calledCallsignCharacter, bool isPublic, int param, float seed, float quality, int playerID)
	{
		if (isPublic || playerID == m_PlayerController.GetPlayerId())
			Rpc(RpcDo_PlayRadioMsg, msg, FactionId, baseCallsign, CompressCallsign(callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, callerCallsignCharacter), CompressCallsign(calledCallsignCompany, calledCallsignPlatoon, calledCallsignSquad, calledCallsignCharacter), param, seed, quality);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] msg
	//! \param[in] factionId
	//! \param[in] baseCallsign
	//! \param[in] callerCallsign
	//! \param[in] calledCallsign
	//! \param[in] param
	//! \param[in] seed
	//! \param[in] quality
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_PlayRadioMsg(SCR_ERadioMsg msg, int factionId, int baseCallsign, int callerCallsign, int calledCallsign, int param, float seed, float quality)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		int callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, callerCallsignCharacter, calledCallsignCompany, calledCallsignPlatoon, calledCallsignSquad, calledCallsignCharacter;
		DecompressCallsign(callerCallsign, callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, callerCallsignCharacter);
		DecompressCallsign(calledCallsign, calledCallsignCompany, calledCallsignPlatoon, calledCallsignSquad, calledCallsignCharacter);
		
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!comp)
			return;
		
		comp.PlayRadioMsg(msg, factionId, baseCallsign, callerCallsignCompany, callerCallsignPlatoon, callerCallsignSquad, callerCallsignCharacter, calledCallsignCompany, calledCallsignPlatoon, calledCallsignSquad, calledCallsignCharacter, param, seed, quality);

//		if (checkHQReached)
//			GetGame().GetCallqueue().CallLater(CheckHQReached, 7000)
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	void CheckHQReached()
	{
//		ChimeraWorld world = GetOwner().GetWorld();
//		if (m_fLastHQRadioMessageTimestamp.PlusMilliseconds(8000).Less(world.GetServerTimestamp()))
//			return;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback
	void OnBeforePlayerInteraction(EResourcePlayerInteractionType interactionType, PlayerController playerController, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		m_bOrphanSuppliesLoaded = false;
		SCR_ResourceConsumer consumer = resourceComponentFrom.GetConsumer(EResourceGeneratorID.VEHICLE_LOAD, resourceType);
		
		if (!consumer && !resourceComponentFrom.GetConsumer(EResourceGeneratorID.DEFAULT, resourceType, consumer))
			return;
		
		SCR_ResourceContainerQueue<SCR_ResourceConsumer> containerQueue = SCR_ResourceContainerQueue<SCR_ResourceConsumer>.Cast(consumer.GetContainerQueue());

		if (containerQueue.GetStorageTypeCount(EResourceContainerStorageType.STORED) == 0)
			m_bOrphanSuppliesLoaded = true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] interactionType
	//! \param[in] playerController
	//! \param[in] resourceComponentFrom
	//! \param[in] resourceComponentTo
	//! \param[in] resourceType
	//! \param[in] resourceValue
	void OnPlayerSuppliesInteraction(EResourcePlayerInteractionType interactionType, PlayerController playerController, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		if (!playerController || !resourceComponentFrom || !resourceComponentTo)
			return;
		
		GetOnTransferSupplies().Invoke(interactionType, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);

		vector pos = resourceComponentFrom.GetOwner().GetOrigin();

		switch (interactionType)
		{
			case EResourcePlayerInteractionType.VEHICLE_LOAD:
			{
				OnSuppliesLoaded(pos, resourceValue, resourceComponentTo);
				break;
			}
			
			case EResourcePlayerInteractionType.VEHICLE_UNLOAD:
			{
				OnSuppliesUnloaded(pos, resourceValue, playerController.GetPlayerId(), resourceComponentFrom);
				break;
			}
			
			case EResourcePlayerInteractionType.INVENTORY_SPLIT:
			{
				Vehicle vehicleFrom = Vehicle.Cast(SCR_EntityHelper.GetMainParent(resourceComponentFrom.GetOwner(), true));
				Vehicle vehicleTo = Vehicle.Cast(SCR_EntityHelper.GetMainParent(resourceComponentTo.GetOwner(), true));
				
				// Ignore vehicle to vehicle transfers
				if (vehicleFrom && vehicleTo)
					break;
				
				if (vehicleFrom)
					OnSuppliesUnloaded(pos, resourceValue, playerController.GetPlayerId(), resourceComponentFrom);
				else
					OnSuppliesLoaded(pos, resourceValue, resourceComponentTo);
				
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSuppliesLoaded(vector position, float amount, notnull SCR_ResourceComponent resourceComponentTo)
	{
		// Allow XP for orphan supplies only when loaded in a base or supply depot
		if (m_bOrphanSuppliesLoaded)
		{
			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
			if (campaign)
			{
				SCR_CampaignMilitaryBaseManager manager = campaign.GetBaseManager();
			
				if (manager)
				{
					SCR_CampaignMilitaryBaseComponent nearestBase = manager.FindClosestBase(position);

					if (!nearestBase || !nearestBase.IsInitialized() || vector.DistanceXZ(position, nearestBase.GetOwner().GetOrigin()) > nearestBase.GetRadius())
					{
						SCR_CampaignSuppliesComponent nearestDepot = manager.FindClosestSupplyDepot(position);
						
						if (!nearestDepot || vector.DistanceXZ(position, nearestDepot.GetOwner().GetOrigin()) > 100)
						{
							ResetSavedSupplies();
							return;
						}
					}
				}
			}
		}

		m_bOrphanSuppliesLoaded = false;
		SCR_ResourceContainer containerTo = resourceComponentTo.GetContainer(EResourceType.SUPPLIES);
		
		if (!containerTo)
		{
			ResetSavedSupplies();
			return;
		}
		
		m_vLastLoadedAt = position;
		m_fLoadedSupplyAmount = containerTo.GetResourceValue();
		m_LastLoadedComponent = resourceComponentTo;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSuppliesUnloaded(vector position, float amount, int playerId, notnull SCR_ResourceComponent resourceComponentFrom, int assistantId = 0)
	{
		// Identify the player who actually loaded the supplies
		if (resourceComponentFrom != m_LastLoadedComponent)
		{
			array<int> allPlayerIds = {};
			PlayerManager pManager = GetGame().GetPlayerManager();
			pManager.GetPlayers(allPlayerIds);
			PlayerController loaderController;
			SCR_CampaignNetworkComponent networkComponent;
			
			foreach (int loaderId : allPlayerIds)
			{
				loaderController = pManager.GetPlayerController(loaderId);
				
				if (!loaderController)
					continue;
				
				networkComponent = SCR_CampaignNetworkComponent.Cast(loaderController.FindComponent(SCR_CampaignNetworkComponent));
				
				if (!networkComponent)
					continue;
				
				if (networkComponent.GetLastLoadedComponent() == resourceComponentFrom)
				{
					networkComponent.OnSuppliesUnloaded(position, amount, loaderId, resourceComponentFrom, playerId);
					return;
				}
			}
			
			return;
		}
		
		if (m_vLastLoadedAt == vector.Zero || amount > m_fLoadedSupplyAmount || vector.DistanceSqXZ(m_vLastLoadedAt, position) <= SUPPLY_DELIVERY_THRESHOLD_SQ)
			return;
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;

		SCR_CampaignMilitaryBaseManager manager = campaign.GetBaseManager();
		
		if (!manager)
			return;
		
		SCR_CampaignMilitaryBaseComponent nearestBase = manager.FindClosestBase(position);
		
		// Only award XP if supplies were unloaded in a base
		if (!nearestBase || !nearestBase.IsInitialized() || vector.DistanceXZ(position, nearestBase.GetOwner().GetOrigin()) > nearestBase.GetRadius())
			return;

		m_fLoadedSupplyAmount -= amount;
		m_iTotalSuppliesDelivered += amount;

		if (s_OnSuppliesDelivered)
			s_OnSuppliesDelivered.Invoke(playerId, (int)amount, m_iTotalSuppliesDelivered);

		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		if (compXP)
		{
			float suppliesXPmultiplier = amount / SUPPLY_DELIVERY_XP_PERCENT;
			compXP.AwardXP(playerId, SCR_EXPRewards.SUPPLIES_DELIVERED, suppliesXPmultiplier);

			if (assistantId > 0)
				compXP.AwardXP(assistantId, SCR_EXPRewards.SUPPLIES_DELIVERED, suppliesXPmultiplier * campaign.GetSupplyOffloadAssistanceReward());
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ResetSavedSupplies()
	{
		m_vLastLoadedAt = vector.Zero;
		m_LastLoadedComponent = null;
		m_fLoadedSupplyAmount = 0;
	}

	//------------------------------------------------------------------------------------------------
	// Init
	override void EOnInit(IEntity owner)
	{
		m_PlayerController = SCR_PlayerController.Cast(PlayerController.Cast(owner));
		
		if (!m_PlayerController)
		{
			Print("SCR_CampaignNetworkComponent must be attached to PlayerController!", LogLevel.ERROR);
			return;
		}
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (m_PlayerController.GetPlayerId() == SCR_PlayerController.GetLocalPlayerId())
			SCR_SpawnPointRequestUIComponent.SGetOnSpawnPointSelected().Insert(HandleRadioRespawnTimer);

		if (IsProxy())
			return;

		SCR_ResourcePlayerControllerInventoryComponent comp = SCR_ResourcePlayerControllerInventoryComponent.Cast(m_PlayerController.FindComponent(SCR_ResourcePlayerControllerInventoryComponent));

		if (comp)
		{
			comp.GetOnPlayerInteraction().Insert(OnPlayerSuppliesInteraction);
			comp.GetOnBeforePlayerInteraction().Insert(OnBeforePlayerInteraction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CampaignNetworkComponent()
	{
		SCR_SpawnPointRequestUIComponent.SGetOnSpawnPointSelected().Remove(HandleRadioRespawnTimer);
	}
}
