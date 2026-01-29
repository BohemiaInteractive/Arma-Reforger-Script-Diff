[ComponentEditorProps(category: "GameScripted/Respawn/PlayerController")]
class SCR_PossessSpawnRequestComponentClass : SCR_SpawnRequestComponentClass
{
}

class SCR_PossessSpawnRequestComponent : SCR_SpawnRequestComponent
{
	//------------------------------------------------------------------------------------------------
	override typename GetHandlerType()
	{
		return SCR_PossessSpawnHandlerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override typename GetDataType()
	{
		return SCR_PossessSpawnData;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool DoCanRequestRespawn(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::DoCanRequestRespawn(playerId: %2, data: %3)", Type().ToString(),
					GetPlayerController().GetPlayerId(), data), LogLevel.NORMAL);
		#endif

		SCR_PossessSpawnData possessData = SCR_PossessSpawnData.Cast(data);
		if (!possessData)
			return false;

		Rpc(Rpc_CanRequestRespawn_S, possessData.GetRplId());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_CanRequestRespawn_S(RplId entityRplId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::Rpc_CanRequestRespawn_S(playerId: %2, entityRplId: %3)", Type().ToString(),
					GetPlayerController().GetPlayerId(),
					entityRplId), LogLevel.NORMAL);
		#endif

		SCR_PossessSpawnData data = SCR_PossessSpawnData.FromRplId(entityRplId);
		ProcessCanRequest_S(data);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool DoRequestRespawn(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::DoRequestRespawn(playerId: %2, data: %3)", Type().ToString(),
					GetPlayerController().GetPlayerId(), data), LogLevel.NORMAL);
		#endif

		SCR_PossessSpawnData possessData = SCR_PossessSpawnData.Cast(data);
		if (!possessData)
			return false;

		Rpc(Rpc_RequestRespawn_S, possessData.GetRplId(), possessData.GetSkipPreload());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_RequestRespawn_S(RplId entityRplId, bool skipPreload)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::Rpc_RequestRespawn_S(playerId: %2, entityRplId: %3)", Type().ToString(),
					GetPlayerController().GetPlayerId(),
					entityRplId), LogLevel.NORMAL);
		#endif

		SCR_PossessSpawnData possessData = SCR_PossessSpawnData.FromRplId(entityRplId);
		possessData.SetSkipPreload(skipPreload);
		ProcessRequest_S(possessData);
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override SCR_BaseRespawnDiag CreateDiag()
	{
		return new SCR_PossessRespawnDiag();
	}
	#endif
}

#ifdef ENABLE_DIAG
class SCR_PossessRespawnDiag : SCR_RespawnDiag<SCR_PossessSpawnRequestComponent>
{
	protected int m_RplId;

	//------------------------------------------------------------------------------------------------
	override void DrawContent()
	{
		super.DrawContent();
		DbgUI.InputInt("Entity RplId: ", m_RplId);
	}

	//------------------------------------------------------------------------------------------------
	protected override SCR_SpawnData CreateData()
	{
		return SCR_PossessSpawnData.FromRplId(m_RplId);
	}
}
#endif
