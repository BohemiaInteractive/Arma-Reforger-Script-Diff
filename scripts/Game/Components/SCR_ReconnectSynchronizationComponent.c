class SCR_ReconnectSynchronizationComponentClass : ScriptComponentClass
{
}

//! Synchronises reconnect state in order to create a client side informational dialog
//! Attached to SCR_PlayerController prefab
class SCR_ReconnectSynchronizationComponent : ScriptComponent
{
	protected const string DIALOG_RECON_RESTORE = "reconnect_restored";
	protected const string DIALOG_RECON_DISCARD = "reconnect_discarded";
		
	protected ref ScriptInvokerInt m_OnPlayerReconnect = new ScriptInvokerInt();	// param is SCR_EReconnectState
	
	//------------------------------------------------------------------------------------------------
	//! Triggers for this SCR_PlayerController's client only
	//! \return
	ScriptInvokerInt GetOnPlayerReconnect()
	{
		return m_OnPlayerReconnect;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call RPC with reconnect state to controller owner
	//! \param[in] reconState
	void CreateReconnectDialog(SCR_EReconnectState reconState)
	{
		Rpc(RPC_DoSendReconnectState, reconState);
	}
			
	//------------------------------------------------------------------------------------------------
	//! Do RPC from server with reconnect state
	//! \param[in] state is subject SCR_EReconnectState
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
    protected void RPC_DoSendReconnectState(int state)
    {
		m_OnPlayerReconnect.Invoke(state);

		if (state == SCR_EReconnectState.ENTITY_AVAILABLE)
			SCR_CommonDialogs.CreateDialog(DIALOG_RECON_RESTORE);
		else if (state == SCR_EReconnectState.ENTITY_DISCARDED)
			SCR_CommonDialogs.CreateDialog(DIALOG_RECON_DISCARD);

		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystem)
			respawnSystem.DestroyLoadingPlaceholder();
    }	
}
