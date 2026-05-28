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
	//! \param[in] useAllAvailableProviders true if game should use all available providers from that base
	void RequestEnterBuildingMode(IEntity provider, int playerID, bool UserActionActivationOnly, bool UserActionUsed, bool useAllAvailableProviders = false)
	{
		RplComponent providerRplComp = RplComponent.Cast(provider.FindComponent(RplComponent));
		if (!providerRplComp)
			return;

		SCR_PlayerController owner = SCR_PlayerController.Cast(GetOwner());
		if (!owner || playerID != owner.GetPlayerId())
			return; // requesting building mode for someone esle is not allowed!

		Rpc(RpcAsk_RequestEnterBuildingMode, providerRplComp.Id(), playerID, UserActionActivationOnly, UserActionUsed, useAllAvailableProviders);
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
	//! \param[in] useAllAvailableProviders true if game should use all available providers from that base
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RequestEnterBuildingMode(RplId rplProviderId, int playerID, bool UserActionActivationOnly, bool UserActionUsed, bool useAllAvailableProviders)
	{
		SCR_PlayerController owner = SCR_PlayerController.Cast(GetOwner());
		if (!owner || playerID != owner.GetPlayerId())
			return;

		IEntity provider = GetProviderFormRplId(rplProviderId);
		if (!provider)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		buildingManagerComponent.EnterEditorMode(playerID, provider, UserActionActivationOnly, UserActionUsed, useAllAvailableProviders);
	}

	//------------------------------------------------------------------------------------------------
	//! Client method which asks the server for the information about providers used for by current build mode editor
	void RequestBuildModeProvider()
	{
		Rpc(RpcAsk_RequestBuildModeProvider);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RequestBuildModeProvider()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;

		PlayerController playerController = PlayerController.Cast(GetOwner());
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerController.GetPlayerId());
		if (!editorManager)
			return;

		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		if (!modeEntity)
		{
			editorManager.Close();
			return;
		}

		SCR_CampaignBuildingEditorComponent buildingComp = SCR_CampaignBuildingEditorComponent.Cast(modeEntity.FindComponent(SCR_CampaignBuildingEditorComponent));
		if (buildingComp)
		{
			buildingComp.SendProviders_S();
		}
		else
		{
			editorManager.Close();
			return;
		}
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
