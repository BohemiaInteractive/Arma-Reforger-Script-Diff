[ComponentEditorProps(category: "GameScripted/Respawn/PlayerController")]
class SCR_FreeSpawnRequestComponentClass : SCR_SpawnRequestComponentClass
{
}

class SCR_FreeSpawnRequestComponent : SCR_SpawnRequestComponent
{
	//------------------------------------------------------------------------------------------------
	override typename GetHandlerType()
	{
		return SCR_FreeSpawnHandlerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override typename GetDataType()
	{
		return SCR_FreeSpawnData;
	}

	//------------------------------------------------------------------------------------------------
	override bool DoCanRequestRespawn(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::DoCanRequestRespawn(playerId: %2, data: %3)", Type().ToString(),
					GetPlayerController().GetPlayerId(), data), LogLevel.NORMAL);
		#endif

		// We can construct whatever we want here on client
		Rpc(Rpc_CanRequestRespawn_S, data.GetPosition(), data.GetAngles(), data.GetPrefab());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_CanRequestRespawn_S(vector position, vector angles, ResourceName prefab)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::Rpc_CanRequestRespawn_S(playerId: %2, pos: %3, ang: %4, pref: %5)", Type().ToString(),
					GetPlayerController().GetPlayerId(),
					position,
					angles,
					prefab), LogLevel.NORMAL);
		#endif

		SCR_FreeSpawnData data = new SCR_FreeSpawnData(prefab, position, angles);
		ProcessCanRequest_S(data);
	}

	//------------------------------------------------------------------------------------------------
	//! Requests respawn with provided data.
	//! \param[in] data
	//! \return true in case success was successfuly dispatched, false otherwise. Note that such request can still be denied by the authority!
	protected override bool DoRequestRespawn(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::DoRequestRespawn(playerId: %2, data: %3)", Type().ToString(),
					GetPlayerController().GetPlayerId(), data), LogLevel.NORMAL);
		#endif

		// We can construct whatever we want here on client
		Rpc(Rpc_RequestRespawn_S, data.GetPosition(), data.GetAngles(), data.GetPrefab());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_RequestRespawn_S(vector position, vector angles, ResourceName prefab)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::Rpc_RequestRespawn_S(playerId: %2, pos: %3, ang: %4, pref: %5)", Type().ToString(),
					GetPlayerController().GetPlayerId(),
					position,
					angles,
					prefab), LogLevel.NORMAL);
		#endif

		SCR_FreeSpawnData data = new SCR_FreeSpawnData(prefab, position, angles);
		ProcessRequest_S(data);
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override SCR_BaseRespawnDiag CreateDiag()
	{
		return new SCR_FreeRespawnDiag();
	}
	#endif
}

#ifdef ENABLE_DIAG
class SCR_FreeRespawnDiag : SCR_RespawnDiag<SCR_FreeSpawnRequestComponent>
{
	protected string m_Prefab = "{7A9EE19AB67B298B}Prefabs/Characters/Factions/INDFOR/FIA/Character_FIA_base.et";
	protected string m_Position = "0 0.1 2";
	protected string m_Angles = "0 180 0";

	//------------------------------------------------------------------------------------------------
	override void DrawContent()
	{
		super.DrawContent();
		DbgUI.InputText("Prefab: ", m_Prefab);
		DbgUI.InputText("Position: ", m_Position);
	}

	//------------------------------------------------------------------------------------------------
	protected override SCR_SpawnData CreateData()
	{
		vector position = m_Position.ToVector();
		vector angles = m_Angles.ToVector();
		return new SCR_FreeSpawnData(m_Prefab, position, angles);
	}
}
#endif
