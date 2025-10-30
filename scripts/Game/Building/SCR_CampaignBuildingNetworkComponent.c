[EntityEditorProps(category: "GameScripted/Building", description: "Handles client > server communication in Building. Should be attached to PlayerController.")]
class SCR_CampaignBuildingNetworkComponentClass : ScriptComponentClass
{
}

class SCR_CampaignBuildingNetworkComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] buildingValue
	//! \param[in] compositionOutline
	void AddBuildingValue(int buildingValue, notnull IEntity compositionOutline)
	{
		RplComponent comp = RplComponent.Cast(compositionOutline.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_AddBuildingValue, buildingValue, comp.Id());
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] provider
	void RemoveEditorMode(int playerID, IEntity provider)
	{
		RplComponent comp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_RemoveEditorMode, playerID, comp.Id());
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] provider
	//! \param[in] playerID
	//! \param[in] UserActionActivationOnly
	//! \param[in] UserActionUsed
	void RequestEnterBuildingMode(IEntity provider, int playerID, bool UserActionActivationOnly, bool UserActionUsed)
	{
		RplComponent providerRplComp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!providerRplComp)
			return;

		Rpc(RpcAsk_RequestEnterBuildingMode, providerRplComp.Id(), playerID, UserActionActivationOnly, UserActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete composition by a tool
	//! \param[in] composition
	void DeleteCompositionByUserAction(notnull IEntity composition)
	{
		RplComponent comp = RplComponent.Cast(composition.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_DeleteCompositionByUserAction, comp.Id());
	}

	//------------------------------------------------------------------------------------------------
	//! Delete base by a tool
	//! \param[in] IEntity base
	void DeleteBaseByUserAction(notnull IEntity base)
	{
		RplComponent comp = RplComponent.Cast(base.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_DeleteBaseByUserAction, comp.Id());
	}

	//------------------------------------------------------------------------------------------------
	//! Send a notification about deleted composition
	//! \param[in] composition
	//! \param[in] playerId
	//! \param[in] callsign
	void SendDeleteNotification(notnull IEntity composition, int playerId, int callsign)
	{
		RplComponent comp = RplComponent.Cast(composition.FindComponent(RplComponent));
		if (!comp)
			return;

		Rpc(RpcAsk_SendDeleteNotification, comp.Id(), playerId, callsign);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetClientLock(bool lock, IEntity provider)
	{
		RplComponent comp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!comp)
			return;
		
		Rpc(RpcDo_SetClientLock, lock, comp.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set a cooldown lock on client.
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_SetClientLock(bool lock, RplId compId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(compId));
		if (!rplComp)
			return;

		IEntity provider = rplComp.GetEntity();
		if (!provider)
			return;
		
		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (providerComponent)
			providerComponent.SetCooldownClientLock(lock);
	}

	//------------------------------------------------------------------------------------------------
	//! Add a building value to a composition outline. Once the define value is reach, composition will be spawned.
	//! \param[in] buildingValue
	//! \param[in] compId
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddBuildingValue(int buildingValue, RplId compId)
	{
		IEntity compositionOutline = GetProviderFormRplId(compId);
		if (!compositionOutline)
			return;

		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(compositionOutline.FindComponent(SCR_CampaignBuildingLayoutComponent));
		if (!layoutComponent)
			return;

		layoutComponent.AddBuildingValue(buildingValue);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete given player building mode.
	//! \param[in] playerID
	//! \param[in] compId
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemoveEditorMode(int playerID, RplId compId)
	{
		IEntity provider = GetProviderFormRplId(compId);
		if (!provider)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(provider.FindComponent(SCR_CampaignBuildingProviderComponent));
		if (!providerComponent)
			return;

		bool isActiveUser = buildingManagerComponent.RemovePlayerIdFromProvider(playerID, providerComponent);
		buildingManagerComponent.RemoveProvider(playerID, providerComponent, isActiveUser);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn a trigger as a child of the provider entity.
	//! \param[in] rplProviderId
	//! \param[in] playerID
	//! \param[in] UserActionActivationOnly
	//! \param[in] UserActionUse
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RequestEnterBuildingMode(RplId rplProviderId, int playerID, bool UserActionActivationOnly, bool UserActionUsed)
	{
		IEntity provider = GetProviderFormRplId(rplProviderId);
		if (!provider)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		buildingManagerComponent.GetEditorMode(playerID, provider, UserActionActivationOnly, UserActionUsed);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete composition, executed from user action
	//! \param[in] rplCompositionId
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_DeleteCompositionByUserAction(RplId rplCompositionId)
	{
		IEntity composition = GetProviderFormRplId(rplCompositionId);
		if (!composition)
			return;
		
		SCR_CampaignBuildingCompositionComponent buildingComponent = SCR_CampaignBuildingCompositionComponent.Cast(composition.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (buildingComponent)
			buildingComponent.SetCanPlaySoundOnDeletion(true);

		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(composition.GetRootParent().FindComponent(SCR_EditableEntityComponent));
		if (editableEntity)
			editableEntity.Delete(true, true);
		else
		{
			SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
			if (aiWorld)
			{
				array<ref Tuple2<vector, vector>> areas = {}; // min, max
				array<bool> redoAreas = {};
				aiWorld.GetNavmeshRebuildAreas(composition.GetRootParent(), areas, redoAreas);
				GetGame().GetCallqueue().CallLater(aiWorld.RequestNavmeshRebuildAreas, 1000, false, areas, redoAreas); //--- Called *before* the entity is deleted with a delay, ensures the regeneration doesn't accidentally get anything from the entity prior to full destruction
			}
			
			RplComponent.DeleteRplEntity(composition, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Delete base, executed from user action
	//! \param[in] RplId rplBaseId
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_DeleteBaseByUserAction(RplId rplBaseId)
	{
		IEntity base = GetProviderFormRplId(rplBaseId);
		if (!base)
			return;

		RplComponent.DeleteRplEntity(base, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Send delete notification.
	//! \param[in] rplCompositionId
	//! \param[in] playerId
	//! \param[in] callsign
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SendDeleteNotification(RplId rplCompositionId, int playerId, int callsign)
	{
		IEntity composition = GetProviderFormRplId(rplCompositionId);
		if (!composition)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerId));
		if (!faction)
			return;

		SCR_NotificationsComponent.SendToFaction(faction, true, ENotification.EDITOR_SERVICE_DISASSEMBLED, playerId, rplCompositionId, callsign);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increase XP
	//! \param[in] playerId
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AddXPReward(int playerId)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
		if (!compXP)
			return;
		
		compXP.AwardXP(playerId, SCR_EXPRewards.FREE_ROAM_BUILDING_BUILT);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	void AddXPReward(int playerId)
	{
		Rpc(RpcAsk_AddXPReward, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rplProviderId
	//! \return
	IEntity GetProviderFormRplId(RplId rplProviderId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplProviderId));
		if (!rplComp)
			return null;

		return rplComp.GetEntity();
	}
}
