[ComponentEditorProps(category: "GameScripted/Respawn/PlayerController")]
class SCR_SpawnPointRespawnRequestComponentClass : SCR_SpawnRequestComponentClass
{
}

class SCR_SpawnPointRespawnRequestComponent : SCR_SpawnRequestComponent
{
	//------------------------------------------------------------------------------------------------
	override typename GetHandlerType()
	{
		return SCR_SpawnPointSpawnHandlerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override typename GetDataType()
	{
		return SCR_SpawnPointSpawnData;
	}

	//------------------------------------------------------------------------------------------------
	override bool DoCanRequestRespawn(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::DoCanRequestRespawn(playerId: %2, data: %3)", Type().ToString(),
					GetPlayerController().GetPlayerId(), data), LogLevel.NORMAL);
		#endif

		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		if (!spawnPointData)
			return false;

		Rpc(Rpc_CanRequestRespawn_S, spawnPointData.GetRplId(), spawnPointData.GetPrefab());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_CanRequestRespawn_S(int spawnPointId, ResourceName prefab)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::Rpc_CanRequestRespawn_S(playerId: %2, spawnPointId: %3, pref: %4)", Type().ToString(),
					GetPlayerController().GetPlayerId(),
					spawnPointId,
					prefab), LogLevel.NORMAL);
		#endif

		// Prepare data for handler
		SCR_SpawnPointSpawnData data = new SCR_SpawnPointSpawnData(prefab, spawnPointId);
		ProcessCanRequest_S(data);
	}

	//------------------------------------------------------------------------------------------------
	override bool DoRequestRespawn(SCR_SpawnData data)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::DoRequestRespawn(playerId: %2, data: %3)", Type().ToString(),
					GetPlayerController().GetPlayerId(), data), LogLevel.NORMAL);
		#endif

		SCR_SpawnPointSpawnData spawnPointData = SCR_SpawnPointSpawnData.Cast(data);
		if (!spawnPointData)
			return false;

		Rpc(Rpc_RequestRespawn_S, spawnPointData.GetRplId(), spawnPointData.GetPrefab());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	private void Rpc_RequestRespawn_S(int spawnPointId, ResourceName prefab)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::Rpc_RequestRespawn_S(playerId: %2, spawnPointId: %3, pref: %4)", Type().ToString(),
					GetPlayerController().GetPlayerId(),
					spawnPointId,
					prefab), LogLevel.NORMAL);
		#endif

		// Prepare data for handler
		SCR_SpawnPointSpawnData data = new SCR_SpawnPointSpawnData(prefab, spawnPointId);
		ProcessRequest_S(data);
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override SCR_BaseRespawnDiag CreateDiag()
	{
		return new SCR_SpawnPointRespawnDiag();
	}
	#endif
}

#ifdef ENABLE_DIAG
class SCR_SpawnPointRespawnDiag : SCR_RespawnDiag<SCR_SpawnPointRespawnRequestComponent>
{
	protected string m_Prefab = "{7A9EE19AB67B298B}Prefabs/Characters/Factions/INDFOR/FIA/Character_FIA_base.et";
	protected RplId m_RplId;

	override void DrawContent()
	{
		super.DrawContent();
		DbgUI.InputText("Prefab: ", m_Prefab);
		DbgUI.InputInt("RplId: ", m_RplId);
		
		DbgUI.Spacer(32);
		DbgUI.Text("======== UTILITY ========");
		int index;
		DbgUI.InputInt("Index2Id: ", index);
		SCR_SpawnPoint point = SCR_SpawnPoint.GetSpawnPointByIndex(index);
		if (point)
		{
			string strRplId = point.GetRplId().ToString();
			DbgUI.Text(string.Format("Point index: %1\n\tRplId: %2\n\tFaction: %3\n\tType: %4",
				index, strRplId, point.GetFactionKey(), point.Type().ToString()));
			
			SCR_PlayerSpawnPoint psp = SCR_PlayerSpawnPoint.Cast(point);
			if (psp)
			{
				int pid = psp.GetPlayerID();
				string pnam = GetGame().GetPlayerManager().GetPlayerName(pid);
				DbgUI.Text(string.Format("    playerId: %1\n    playerName: %2", pid, pnam));
			}
			
			DbgUI.Text(string.Format("IsActive: %1", point.IsSpawnPointActive()));
			
			if (DbgUI.Button("Copy RplId to clipboard"))
				System.ExportToClipboard(strRplId);
		}
		else
		{
			DbgUI.Text(string.Format("No spawn point at index: %1", index));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override SCR_SpawnData CreateData()
	{
		return new SCR_SpawnPointSpawnData(m_Prefab, m_RplId);
	}
}
#endif
