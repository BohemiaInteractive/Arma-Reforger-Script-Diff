class SCR_ReconnectSynchronizationComponentClass : ScriptComponentClass
{
}

//! Synchronises reconnect state in order to create a client side informational dialog
//! Attached to SCR_PlayerController prefab
class SCR_ReconnectSynchronizationComponent : ScriptComponent
{
	protected const string DIALOG_RECON_RESTORE = "reconnect_restored";
	protected const string DIALOG_RECON_DISCARD = "reconnect_discarded";

	protected ref ScriptInvokerVoid m_OnPlayerReconnect;

	//------------------------------------------------------------------------------------------------
	//! Triggers for this SCR_PlayerController's client only
	ScriptInvokerVoid GetOnPlayerReconnect()
	{
		if (!m_OnPlayerReconnect)
			m_OnPlayerReconnect = new ScriptInvokerVoid();

		return m_OnPlayerReconnect;
	}

	//------------------------------------------------------------------------------------------------
	//! Call RPC with reconnect state to controller owner
	void CreateReconnectDialog()
	{
		Rpc(RPC_DoSendReconnectState);
	}

	//------------------------------------------------------------------------------------------------
	//! Do RPC from server with reconnect state
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RPC_DoSendReconnectState()
	{
		SCR_CommonDialogs.CreateDialog(DIALOG_RECON_RESTORE);

		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystem)
			respawnSystem.DestroyLoadingPlaceholder();

		if (m_OnPlayerReconnect)
			m_OnPlayerReconnect.Invoke();
	}
}
