[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_PlayerActionsComponentClass : ScriptComponentClass
{
	protected ref SCR_PlayerListActions m_PlayerListActions;

	protected const ResourceName ACTION_LIST_CONFIG = "{B0E693F0D7A70222}Configs/System/Actions/PlayerListActions.conf";

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] componentSource
	//! \param[in] parentSource
	//! \param[in] prefabSource
	void SCR_PlayerActionsComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource)
	{
		Resource holder = BaseContainerTools.LoadContainer(ACTION_LIST_CONFIG);
		if (!holder)
			return;

		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return;

		m_PlayerListActions = SCR_PlayerListActions.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		if (!m_PlayerListActions)
		{
			Debug.Error("SCR_PlayerActionsComponentClass was unable to load action list from " + ACTION_LIST_CONFIG);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_PlayerListActions GetPlayerListActions()
	{
		return m_PlayerListActions;
	}
}

class SCR_PlayerActionsComponent : ScriptComponent
{
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] combo
	//! \param[in] targetPlayerId
	void CreatePlayerListActionEntries(notnull SCR_ComboBoxComponent combo, int targetPlayerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		int userPlayerId = playerController.GetPlayerId();
		SCR_PlayerActionsComponentClass data = SCR_PlayerActionsComponentClass.Cast(GetComponentData(playerController));
		data.GetPlayerListActions().CreateActionEntries(combo, targetPlayerId, userPlayerId, playerController);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] targetPlayerId
	//! \return
	bool CanUseAnyPlayerListAction(int targetPlayerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		int userPlayerId = playerController.GetPlayerId();
		SCR_PlayerActionsComponentClass data = SCR_PlayerActionsComponentClass.Cast(GetComponentData(playerController));
		return data.GetPlayerListActions().CanUseAnyAction(targetPlayerId, userPlayerId, playerController);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerListOpened()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		SCR_PlayerActionsComponentClass data = SCR_PlayerActionsComponentClass.Cast(GetComponentData(playerController));
		data.GetPlayerListActions().OnPlayerListOpened(playerController);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerListClosed()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		SCR_PlayerActionsComponentClass data = SCR_PlayerActionsComponentClass.Cast(GetComponentData(playerController));
		data.GetPlayerListActions().OnPlayerListClosed(playerController);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] actionId
	//! \param[in] targetPlayerId
	void PerformPlayerListAction(int actionId, int targetPlayerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		int userPlayerId = playerController.GetPlayerId();
		SCR_PlayerActionsComponentClass data = SCR_PlayerActionsComponentClass.Cast(GetComponentData(playerController));
		bool executeOnServer;
		if (data.GetPlayerListActions().PerformActionClient(actionId, targetPlayerId, userPlayerId, playerController, executeOnServer) && executeOnServer)
			Rpc(RpcAsk_ExecutePlayerListAction, actionId, targetPlayerId);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] actionId
	//! \param[in] targetPlayerId
	[RplRpc(channel: RplChannel.Reliable, rcver: RplRcver.Server)]
	protected void RpcAsk_ExecutePlayerListAction(int actionId, int targetPlayerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		int userPlayerId = playerController.GetPlayerId();
		SCR_PlayerActionsComponentClass data = SCR_PlayerActionsComponentClass.Cast(GetComponentData(playerController));
		data.GetPlayerListActions().PerformActionServer(actionId, targetPlayerId, userPlayerId, playerController)
	}
}
