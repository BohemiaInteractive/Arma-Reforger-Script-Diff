[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Restore editor modes when world changes in multiplayer.")]
class SCR_RestoreEditorModesComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_RestoreEditorModesComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc: "When enabled, the system will remember and restore also modes of limited editor managers (e.g., those with only PHOTO mode, not EDIT).")]
	protected bool m_bRestoreLimitedEditors;
	
	protected static const string GAME_SESSION_STORAGE_MODES = "SCR_RestoreEditorModesComponent_ModesMap";
	protected static const string DELIMITER_PLAYERS = " ";
	protected static const string DELIMITER_VALUES = ":";
	
	protected ref map<string, EEditorMode> m_mModes = new map<string, EEditorMode>();
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorManagerCreatedServer(SCR_EditorManagerEntity editorManager)
	{
		const int playerID = editorManager.GetPlayerID();
		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerID);
		EEditorMode modes;
		if (m_mModes.Find(playerUID, modes))
		{
			Print(string.Format("SCR_RestoreEditorModesComponent: Restoring editor modes %1 for playerId=%2 ('%3')", SCR_Enum.FlagsToString(EEditorMode, modes), playerID, GetGame().GetPlayerManager().GetPlayerName(playerID)), LogLevel.NORMAL);
			editorManager.AddEditorModes(EEditorModeAccess.BASE, modes);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		if (!Replication.IsRunning() || !Replication.IsServer())
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		BackendApi backendApi = GetGame().GetBackendApi();
		
		string modes;
		array<SCR_EditorManagerEntity> managers = {};
		core.GetEditorEntities(managers);
		foreach (SCR_EditorManagerEntity editorManager: managers)
		{
			if (m_bRestoreLimitedEditors || !editorManager.IsLimited())
				modes += SCR_PlayerIdentityUtils.GetPlayerIdentityId(editorManager.GetPlayerID()) + DELIMITER_VALUES + editorManager.GetEditorModes() + DELIMITER_PLAYERS;
		}
		
		GameSessionStorage.s_Data.Insert(GAME_SESSION_STORAGE_MODES, modes);
		
		core.Event_OnEditorManagerCreatedServer.Remove(OnEditorManagerCreatedServer);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsRunning() || !Replication.IsServer())
			return;
		
		string modesMap;
		if (!GameSessionStorage.s_Data.Find(GAME_SESSION_STORAGE_MODES, modesMap))
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		array<string> players = {};
		array<string> values = {};
		modesMap.Split(DELIMITER_PLAYERS, players, true);
		for (int i = 0, count = players.Count(); i < count; i++)
		{
			players[i].Split(DELIMITER_VALUES, values, false);
			m_mModes.Insert(values[0], values[1].ToInt());
		}
		
		GameSessionStorage.s_Data.Remove(GAME_SESSION_STORAGE_MODES);
		
		core.Event_OnEditorManagerCreatedServer.Insert(OnEditorManagerCreatedServer);
	}
}
