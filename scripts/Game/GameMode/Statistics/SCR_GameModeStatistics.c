#ifdef GMSTATS 
#define GM_AI_STATS
#endif
#ifdef AISTATS
#define GM_AI_STATS
#endif

#ifdef GM_AI_STATS

enum SCR_EGameModeRecordType
{
	Death,
	Movement,
	Connection,
	Spawn,
	AILOD,
}

enum ETypeOfAgent // TODO: SCR_ETypeOfAgent / SCR_EAgentType
{
	Group,
	Agent,
	Bird,
	Player
}

//! Defines an event record in the game mode statistics.
class SCR_IGameModeRecord
{
	//! Current timestamp as world time in milliseconds.
	protected float m_fTimestamp;

	//------------------------------------------------------------------------------------------------
	//! Returns type of this record.
	SCR_EGameModeRecordType GetRecordType();

	//------------------------------------------------------------------------------------------------
	//! Return time (since load of world) when this record occured
	float GetTimestamp()
	{
		return m_fTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	//! Parses shared tokens and calls derived save method.
	string SaveToString()
	{
		return string.Format("%1;%2;%3",
			GetRecordType().ToString(),
			GetTimestamp().ToString(),
			Save()
		);
	}

	//------------------------------------------------------------------------------------------------
	//! Saves record to a string. Internal tokens are ommited.
	protected string Save();

	//------------------------------------------------------------------------------------------------
	//! Loads record from tokens. See LoadFromString. Internal tokens are ommited.
	protected void Load(array<string> tokens);

	//------------------------------------------------------------------------------------------------
	//! Parses a record from provided string.
	static ref SCR_IGameModeRecord LoadFromString(string str)
	{
		array<string> tokens = {};
		str.Split(";", tokens, false);

		int type = tokens[0].ToInt();
		tokens.RemoveOrdered(0);

		float timeStamp = tokens[0].ToFloat();
		tokens.RemoveOrdered(0);

		switch (type)
		{
			case SCR_EGameModeRecordType.Death:
			{
				ref SCR_IGameModeRecord record = new SCR_DeathRecord();
				record.m_fTimestamp = timeStamp;
				record.Load(tokens);
				return record;
			}
			break;

			case SCR_EGameModeRecordType.Movement:
			{
				ref SCR_IGameModeRecord record = new SCR_MovementRecord();
				record.m_fTimestamp = timeStamp;
				record.Load(tokens);
				return record;
			}
			break;

			case SCR_EGameModeRecordType.Connection:
			{
				ref SCR_IGameModeRecord record = new SCR_ConnectionRecord();
				record.m_fTimestamp = timeStamp;
				record.Load(tokens);
				return record;
			}
			break;
			
			case SCR_EGameModeRecordType.Spawn:
			{
				ref SCR_IGameModeRecord record = new SCR_SpawnRecord();
				record.m_fTimestamp = timeStamp;
				record.Load(tokens);
				return record;
			}
			break;
			
			case SCR_EGameModeRecordType.AILOD:
			{
				ref SCR_IGameModeRecord record = new SCR_AILODRecord();
				record.m_fTimestamp = timeStamp;
				record.Load(tokens);
				return record;
			}
		}

		return null;
	}
}

//! Object that defines a single player death event with corresponding data.
sealed class SCR_DeathRecord : SCR_IGameModeRecord
{
	vector m_Position;
	vector m_InstigatorPosition;
	FactionKey m_Faction;
	FactionKey m_InstigatorFaction;
	int m_InstigatorId;
	int m_PlayerId;
	EWeaponType m_WeaponType = EWeaponType.WT_NONE;

	//------------------------------------------------------------------------------------------------
	sealed override SCR_EGameModeRecordType GetRecordType()
	{
		return SCR_EGameModeRecordType.Death;
	}

	//------------------------------------------------------------------------------------------------
	override string Save()
	{
		string pos = string.Format("%1,%2,%3", m_Position[0], m_Position[1], m_Position[2]);
		string ipos = string.Format("%1,%2,%3", m_InstigatorPosition[0], m_InstigatorPosition[1], m_InstigatorPosition[2]);
		return string.Format("%1;%2;%3;%4;%5;%6;%7", pos, ipos, m_Faction, m_InstigatorFaction, (int)m_WeaponType, m_PlayerId, m_InstigatorId);
	}

	//------------------------------------------------------------------------------------------------
	protected sealed override void Load(array<string> tokens)
	{
		array <string> pos = {};
		tokens[0].Split(",", pos, true);
		m_Position = Vector(pos[0].ToFloat(), pos[1].ToFloat(), pos[2].ToFloat());

		array <string> ipos = {};
		tokens[1].Split(",", ipos, true);
		m_InstigatorPosition = Vector(ipos[0].ToFloat(), ipos[1].ToFloat(), ipos[2].ToFloat());

		m_Faction = tokens[2];
		m_InstigatorFaction = tokens[3];

		m_WeaponType = tokens[4].ToInt();
		m_PlayerId = tokens[5].ToInt();
		m_InstigatorId = tokens[6].ToInt();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \param[in] playerEntity
	//! \param[in] killerEntity
	//! \param[in] killer
	//! \return
	static ref SCR_DeathRecord CreateNew(int playerId, IEntity playerEntity, IEntity killerEntity, Instigator killer)
	{
		SCR_DeathRecord record = new SCR_DeathRecord();
		record.m_Position = playerEntity.GetOrigin();
		record.m_fTimestamp = playerEntity.GetWorld().GetWorldTime();
		record.m_PlayerId = playerId;
		record.m_InstigatorId = killer.GetInstigatorPlayerID();
		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(playerEntity.FindComponent(FactionAffiliationComponent));
		if (factionComponent)
		{
			Faction faction = factionComponent.GetAffiliatedFaction();
			if (faction)
				record.m_Faction = faction.GetFactionKey();
		}

		if (killerEntity)
		{
			record.m_InstigatorPosition = killerEntity.GetOrigin();

			ChimeraCharacter ch = ChimeraCharacter.Cast(killerEntity);
			if (ch)
			{
				BaseWeaponManagerComponent wpm = ch.GetCharacterController().GetWeaponManagerComponent();
				if (wpm)
				{
					BaseWeaponComponent weapon = wpm.GetCurrentWeapon();
					if (weapon)
						record.m_WeaponType = weapon.GetWeaponType();
				}
			}

			FactionAffiliationComponent instigatorFactionComponent = FactionAffiliationComponent.Cast(killerEntity.FindComponent(FactionAffiliationComponent));
			if (instigatorFactionComponent)
			{
				Faction instigatorFaction = instigatorFactionComponent.GetAffiliatedFaction();
				if (instigatorFaction)
					record.m_InstigatorFaction = instigatorFaction.GetFactionKey();
			}
		}

		// Keep track of instigator stuff..
		return record;
	}
}

//! Object that defines a single player movement event with corresponding data.
sealed class SCR_MovementRecord : SCR_IGameModeRecord
{
	vector m_Position;
	int m_PlayerId;

	//------------------------------------------------------------------------------------------------
	sealed override SCR_EGameModeRecordType GetRecordType()
	{
		return SCR_EGameModeRecordType.Movement;
	}

	//------------------------------------------------------------------------------------------------
	override string Save()
	{
		string pos = string.Format("%1,%2,%3", m_Position[0], m_Position[1], m_Position[2]);
		return string.Format("%1;%2;", pos, m_PlayerId);
	}

	//------------------------------------------------------------------------------------------------
	protected sealed override void Load(array<string> tokens)
	{
		array <string> pos = {};
		tokens[0].Split(",", pos, true);
		m_Position = Vector(pos[0].ToFloat(), pos[1].ToFloat(), pos[2].ToFloat());
		m_PlayerId = tokens[1].ToInt();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] player
	//! \param[in] playerId
	//! \return
	static ref SCR_MovementRecord CreateNew(IEntity player, int playerId)
	{
		SCR_MovementRecord record = new SCR_MovementRecord();
		record.m_Position = player.GetOrigin();
		record.m_PlayerId = playerId;
		record.m_fTimestamp = player.GetWorld().GetWorldTime();
		return record;
	}
}

//! Object that defines a single player connection or disconnection event.
sealed class SCR_ConnectionRecord : SCR_IGameModeRecord
{
	int m_PlayerId;
	string m_PlayerName;

	//------------------------------------------------------------------------------------------------
	sealed override SCR_EGameModeRecordType GetRecordType()
	{
		return SCR_EGameModeRecordType.Connection;
	}

	//------------------------------------------------------------------------------------------------
	override string Save()
	{
		return string.Format("%1;%2;", m_PlayerId, m_PlayerName);
	}

	//------------------------------------------------------------------------------------------------
	protected sealed override void Load(array<string> tokens)
	{
		m_PlayerId = tokens[0].ToInt();
		m_PlayerName = tokens[1];
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \param[in] playerName
	//! \return
	static ref SCR_ConnectionRecord CreateNew(int playerId, string playerName)
	{
		SCR_ConnectionRecord record = new SCR_ConnectionRecord();
		record.m_PlayerId = playerId;
		record.m_PlayerName = playerName;
		record.m_fTimestamp = GetGame().GetWorld().GetWorldTime();
		return record;
	}
}

sealed class SCR_SpawnRecord : SCR_IGameModeRecord 
{
	int m_PlayerId;
	int m_FactionColor;
	
	sealed override SCR_EGameModeRecordType GetRecordType()
	{
		return SCR_EGameModeRecordType.Spawn;
	}
	
	override string Save()
	{
		return string.Format("%1;%2;", m_PlayerId, m_FactionColor);
	}
	
	protected sealed override void Load(array<string> tokens)
	{
		m_PlayerId = tokens[0].ToInt();
		m_FactionColor = tokens[1].ToInt();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SCR_SpawnRecord()
	{}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \param[in] factionColor
	//! \return
	static ref SCR_SpawnRecord CreateNew(int playerId, int factionColor)
	{
		SCR_SpawnRecord record = new SCR_SpawnRecord();
		record.m_PlayerId = playerId;
		record.m_FactionColor = factionColor;
		record.m_fTimestamp = GetGame().GetWorld().GetWorldTime();
		return record;		
	}
}

sealed class SCR_AILODRecord : SCR_IGameModeRecord
{
	string m_ID;
	vector m_Position;
	int m_LOD;
	ETypeOfAgent m_Type;
	int m_BTCounter = 0;
	int m_PerceptionCounter = 0;
	
	sealed override SCR_EGameModeRecordType GetRecordType()
	{
		return SCR_EGameModeRecordType.AILOD;
	}
	
	override string Save()
	{
		string pos = string.Format("%1,%2,%3", m_Position[0], m_Position[1], m_Position[2]);
		return string.Format("%1;%2;%3;%4;%5;%6", m_ID, pos, m_LOD, m_Type, m_BTCounter, m_PerceptionCounter);
	}
	
	protected sealed override void Load(array<string> tokens)
	{
		array <string> pos = {};
		m_ID = tokens[0];
		tokens[1].Split(",", pos, true);
		m_Position = Vector(pos[0].ToFloat(), pos[1].ToFloat(), pos[2].ToFloat());
		m_LOD = tokens[2].ToInt();
		m_Type = tokens[3].ToInt();
		m_BTCounter = tokens[4].ToInt();
		m_PerceptionCounter = tokens[5].ToInt();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] agent
	//! \return
	static ref SCR_AILODRecord CreateNew(AIAgent agent)
	{
		SCR_AILODRecord record = new SCR_AILODRecord();
		record.m_fTimestamp = GetGame().GetWorld().GetWorldTime();
		record.m_LOD = agent.GetLOD();
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		IEntity ent = agent.GetControlledEntity();
		if (group)
		{
			record.m_Type = ETypeOfAgent.Group;
			record.m_Position = group.GetCenterOfMass();
		} 
		else 
		{			
			if (ent)
				record.m_Position = ent.GetOrigin();
			if (AIFlock.Cast(agent))
				record.m_Type = ETypeOfAgent.Bird;
			else if (EntityUtils.IsPlayer(ent)) 
				record.m_Type = ETypeOfAgent.Player;
			else	
				record.m_Type = ETypeOfAgent.Agent;			
		}
		
		AIChimeraBehaviorTreeComponent btComp = AIChimeraBehaviorTreeComponent.Cast(agent.FindComponent(AIChimeraBehaviorTreeComponent));
		if (btComp)
			record.m_BTCounter = btComp.GetSimulateCounter();
		
		if (ent)
		{
			PerceptionComponent percComp = PerceptionComponent.Cast(ent.FindComponent(PerceptionComponent));
			if (percComp)
				record.m_PerceptionCounter = percComp.GetSimulateCounter();
		}
		record.m_ID = agent.GetID().ToString().Substring(2,16);
		return record;
	}
}

//! Utility class that can store and manipulate various game mode statistic records.
class SCR_GameModeStatistics
{
	//! Interval of continuous records in ms
	static const int RECORD_INTERVAL_MS = 1000;
	
	//! List of pending records
	private static ref array<ref SCR_IGameModeRecord> s_aRecordBuffer = {};

	//! File to which records should be written
	private static ref FileHandle s_pFileHandle;

	//! File extension used by SCR_GameModeStatistics
	private static const string k_sExtension = ".txt";

	//------------------------------------------------------------------------------------------------
	//! Returns true if statistics are being recorded.
	static bool IsRecording()
	{
		if (s_pFileHandle)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Starts recording statistics.
	static void StartRecording()
	{
		if (IsRecording())
		{
			Debug.Error("Already recording!");
			return;
		}

		string filename = "GameStatistics_" + FormatTimestamp() + k_sExtension;
		s_pFileHandle = FileIO.OpenFile("$logs:/" + filename, FileMode.WRITE);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns whether any content is available to be flushed into output file.
	static bool CanFlush()
	{
		return s_aRecordBuffer.Count() > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Flushes currently recorded data into the open file.
	static void Flush()
	{
		if (!IsRecording())
		{
			Debug.Error("Cannot flush buffer, statistics are not being recorded!");
			return;
		}

		// Empty
		if (!CanFlush())
			return;

		for (int i = 0, count = s_aRecordBuffer.Count(); i < count; i++)
		{
			SCR_IGameModeRecord record = s_aRecordBuffer[i];
			if (!record) // Skip broken record(s)
				continue;

			string entry = record.SaveToString();
			if (entry.IsEmpty()) // Keep the file tidy
				continue;

			s_pFileHandle.WriteLine(entry);
		}

		// Clear the buffer
		s_aRecordBuffer.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Stops recording statistics.
	static void StopRecording()
	{
		if (!IsRecording())
		{
			Debug.Error("Not recording!");
			return;
		}

		// Write remaining data, if any
		if (CanFlush())
			Flush();

		s_pFileHandle.Close();
		s_pFileHandle = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a death record.
	//! \param[in] playerId
	//! \param[in] playerEntity Dead/destroyed entity
	//! \param[in] killerEntity Killer entity
	//! \param[in] killer
	static void RecordDeath(int playerId, IEntity playerEntity, IEntity killerEntity, Instigator killer)
	{
		SCR_DeathRecord rec = SCR_DeathRecord.CreateNew(playerId, playerEntity, killerEntity, killer);
		s_aRecordBuffer.Insert(rec);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a movement record.
	//! \param[in] player Player to record info for.
	//! \param[in] playerId
	static void RecordMovement(IEntity player, int playerId)
	{
		SCR_MovementRecord rec = SCR_MovementRecord.CreateNew(player, playerId);
		s_aRecordBuffer.Insert(rec);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a new connection record.
	//! \param[in] playerId
	//! \param[in] playerName
	static void RecordConnection(int playerId, string playerName)
	{
		SCR_ConnectionRecord rec = SCR_ConnectionRecord.CreateNew(playerId, playerName);
		s_aRecordBuffer.Insert(rec);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates a new spawn record.
	//! \param[in] playerId
	//! \param[in] factionColor
	static void RecordSpawn(int playerId, int factionColor)
	{
		SCR_SpawnRecord rec = SCR_SpawnRecord.CreateNew(playerId, factionColor);
		s_aRecordBuffer.Insert(rec);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] aiagent
	static void RecordAILOD(AIAgent aiagent)
	{
		SCR_AILODRecord rec = SCR_AILODRecord.CreateNew(aiagent);
		s_aRecordBuffer.Insert(rec);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens and outputs parsed statistics from a file. Returns number of entries loaded or 0 if none.
	static int OpenStatistics(string filename, notnull array<ref SCR_IGameModeRecord> outRecords)
	{
		outRecords.Clear();
		if (!filename.EndsWith(k_sExtension))
			filename += k_sExtension;

		if (!FileIO.FileExists(filename))
			return 0;

		FileHandle fileHnd = FileIO.OpenFile(filename, FileMode.READ);
		if (!fileHnd)
			return 0;

		int count = 0;
		string temp = "";
		while (fileHnd.ReadLine(temp) > 0)
		{
			ref SCR_IGameModeRecord record = SCR_IGameModeRecord.LoadFromString(temp);
			if (record != null)
			{
				outRecords.Insert(record);
				++count;
			}
		}

		// Finally dispose of handle
		fileHnd.Close();
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Iterates through records collection and outputs records that match predicate to filteredRecords.
	static void FilterRecords(typename predicate, notnull array<SCR_IGameModeRecord> records, out array<SCR_IGameModeRecord> filteredRecords)
	{
		foreach (SCR_IGameModeRecord record : records)
		{
			if (record.Type().IsInherited(predicate))
				filteredRecords.Insert(record);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Iterates through records collection and outputs records that match predicate to filteredRecords.
	static void FilterRecordsRefRef(typename predicate, notnull array<ref SCR_IGameModeRecord> records, out array<ref SCR_IGameModeRecord> filteredRecords)
	{
		foreach (SCR_IGameModeRecord record : records)
		{
			if (record.Type().IsInherited(predicate))
				filteredRecords.Insert(record);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Iterates through records collection and outputs records that match predicate to filteredRecords.
	static void FilterRecordsWeakRef(typename predicate, notnull array<SCR_IGameModeRecord> records, out array<ref SCR_IGameModeRecord> filteredRecords)
	{
		foreach (SCR_IGameModeRecord record : records)
		{
			if (record.Type().IsInherited(predicate))
				filteredRecords.Insert(record);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current timestamp in yymmdd-hhmmss format.
	private static string FormatTimestamp()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		string smonth, sday;
		if (month < 10)
			smonth = string.Format("0%1", month);
		else
			smonth = string.Format("%1", month);

		if (day < 10)
			sday = string.Format("0%1", day);
		else
			sday = string.Format("%1", day);

		int h, m, s;
		System.GetHourMinuteSecond(h, m, s);
		string sh, sm, ss;
		if (h < 10)
			sh = string.Format("0%1", h);
		else
			sh = string.Format("%1", h);

		if (m < 10)
			sm = string.Format("0%1", m);
		else
			sm = string.Format("%1", m);
		if (s < 10)
			ss = string.Format("0%1", s);
		else
			ss = string.Format("%1", s);

		return string.Format("%1%2%3-%4%5%6", year, smonth, sday, sh, sm, ss);
	}
}

[EntityEditorProps(category: "GameDiag", description: "")]
class SCR_StatisticsDrawerEntityClass : GenericEntityClass
{
}

class SCR_StatisticsDrawerEntity : GenericEntity
{
	//! Current statistics data
	protected ref array<ref SCR_IGameModeRecord> m_aRecords;

	//! If false, history is skipped and all data is drawn simultaneously
	protected bool m_bUseHistory = false;

	//! If false, history is skipped and all data is drawn simultaneously
	protected bool m_bNoZBuffer = false;

	//! If false, history is skipped and all data is drawn simultaneously
	protected float m_fGizmosScaleMultiplier = 1.0;

	//! Number of unique clients
	protected int m_iUniquePlayerCount = 0;

	//! Target player id to filter movement for or -1 if none
	protected int m_iTargetPlayer = 0;

	//! Should "anything" be drawn?
	protected bool m_bDraw = true;

	//! Should SCR_DeathRecord(s) be drawn?
	protected bool m_bDrawDeaths = true;

	//! Should deaths text boxes be drawn?
	protected bool m_bDrawDeathsInfo = true;

	//! Should SCR_MovementRecord(s) be drawn?
	protected bool m_bDrawMovement = true;

	//! Should SCR_ConnectionRecord(s) be drawn?
	protected bool m_bDrawPlayerNames = true;

	//! Should SCR_AILODRecord(s) be drawn?
	protected bool m_bDrawAILODs = true;

	//! If enabled, playback is enabled
	protected bool m_bAutoPlay;

	//! Speed multiplier of platback
	protected float m_fPlaybackSpeed = 1.0;

	//! Current playback time
	protected float m_fCurrentTime;

	//! Time of last record
	protected float m_fMaxTime;

	//! History length in milliseconds
	protected float m_fHistoryLength = 10000;

	//! Filtered death records
	protected ref map<int, ref array<ref SCR_IGameModeRecord>> m_aDeathRecords = new map<int, ref array<ref SCR_IGameModeRecord>>();

	//! Filtered movement records
	protected ref map<int, ref array<ref SCR_IGameModeRecord>> m_aMovementRecords = new map<int, ref array<ref SCR_IGameModeRecord>>();

	//! Filtered spawn records
	protected ref map<int, ref array<ref SCR_IGameModeRecord>> m_aSpawnRecords = new map<int, ref array<ref SCR_IGameModeRecord>>();

	//! Filtered connection records
	protected ref array<ref SCR_IGameModeRecord> m_aConnectionRecords = {};

	//! Filtered AILOD records
	protected ref map<string, ref array<ref SCR_IGameModeRecord>> m_aAILODRecords = new map<string, ref array<ref SCR_IGameModeRecord>>();
	
	protected ref array<ref Shape> m_aShapes = {};
	protected ref array<ref DebugText> m_aTexts = {};

	//! Map of all available player names
	protected ref map<int, string> m_mPlayerNames = new map<int, string>();

	protected vector m_vMins;
	protected vector m_vMaxs;
	
	private static const float MAX_MOVEMENT_SPEED = 11.1;
	//! Any movement over this distance is deemed teleport and not drawn
	protected static const float TELEPORT_DISTANCE = MAX_MOVEMENT_SPEED * (SCR_GameModeStatistics.RECORD_INTERVAL_MS / 1000.0);
	protected static const float TELEPORT_DISTANCE_SQ = TELEPORT_DISTANCE * TELEPORT_DISTANCE;
	
	//! Pseudorandom colors used for players to differentiate them
	protected static const int PLAYER_COLORS_COUNT = 27;
	protected static const vector PLAYER_COLORS_ARRAY[PLAYER_COLORS_COUNT] = {
		"255 255 255", "169 244 0", "244 67 54", "233 30 99", "156 39 176", "63 81 181", "0 188 212", "0 150 136", "76 175 80", "205 220 57",
		"255 235 59", "255 152 0", "121 85 72", "95 125 139","62 69 81","3 159 244","244 67 54","233 30 99", "156 39 176",
		"63 81 181", "0 188 212", "0 150 136", "76 175 80", "205 220 57", "255 235 59", "255 157 0", "121 85 139"
	};
	
	protected static const vector AILOD_COLORS_ARRAY[11] = {
		"255 255 255",
		"244 225 244",
		"233 195 233",
		"222 165 222",
		"211 135 211",
		"200 105 200",
		"189 75 189",
		"178 45 178",
		"167 15 167",
		"156 0 156",
		"0 0 0"
	};
	
	protected bool m_bFactionColors = false;
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if no records are available
	protected bool IsEmpty()
	{
		if (!m_aRecords || m_aRecords.Count() < 1)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to open provided file name with statistics. Returns true on success.
	protected bool OpenStatisticsFile(string filename)
	{
		m_aRecords = {};
		int count = SCR_GameModeStatistics.OpenStatistics(filename, m_aRecords);
		return count > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Clears all <runtime> data, not <loaded> data. Use Close() to close entirely.
	protected void Clear()
	{
		m_aShapes.Clear();
		m_aTexts.Clear();
		m_aDeathRecords.Clear();
		m_aMovementRecords.Clear();
		m_aConnectionRecords.Clear();
		m_aAILODRecords.Clear();
		m_vMins = vector.Zero;
		m_vMaxs = vector.Zero;
		m_iTargetPlayer = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! "Closes open file" by clearing <loaded> data.
	protected void Close()
	{
		Clear();
		m_aRecords.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Filters and builds runtime data from provided records.
	protected void LoadRecords(array<ref SCR_IGameModeRecord> records)
	{
		// Remove any previous entry
		Clear();

		array<ref SCR_IGameModeRecord> deaths = {};
		SCR_GameModeStatistics.FilterRecordsRefRef(SCR_DeathRecord, records, deaths);

		array<ref SCR_IGameModeRecord> movement = {};
		SCR_GameModeStatistics.FilterRecordsRefRef(SCR_MovementRecord, records, movement);

		array<ref SCR_IGameModeRecord> lodRecords = {};
		SCR_GameModeStatistics.FilterRecordsRefRef(SCR_AILODRecord, records, lodRecords);

		SCR_GameModeStatistics.FilterRecordsRefRef(SCR_ConnectionRecord, records, m_aConnectionRecords);
		
		array<ref SCR_IGameModeRecord> spawn = {};
		SCR_GameModeStatistics.FilterRecordsRefRef(SCR_SpawnRecord, records, spawn);
		// Values used for finding "bounding box"
		vector max = "-999999 -999999 -999999";
		vector min = "999999 999999 999999";

		// Sort movement per player id
		m_aMovementRecords.Clear();
		foreach (SCR_IGameModeRecord record : movement)
		{
			SCR_MovementRecord movementRecord = SCR_MovementRecord.Cast(record);
			if (!m_aMovementRecords.Contains(movementRecord.m_PlayerId))
			{
				// Initialize the array inside first
				m_aMovementRecords.Insert(movementRecord.m_PlayerId, new array<ref SCR_IGameModeRecord>());
			}

			// Insert record for that player
			m_aMovementRecords[movementRecord.m_PlayerId].Insert(movementRecord);

			// Additionally check the current position against previously known mins/maxes
			for (int i = 0; i < 3; i++)
			{
				if (movementRecord.m_Position[i] < min[i])
					min[i] = movementRecord.m_Position[i];

				if (movementRecord.m_Position[i] > max[i])
					max[i] = movementRecord.m_Position[i];
			}
		}

		// Sort deaths per player id
		m_aDeathRecords.Clear();
		foreach (SCR_IGameModeRecord record : deaths)
		{
			SCR_DeathRecord deathRecord = SCR_DeathRecord.Cast(record);
			if (!m_aDeathRecords.Contains(deathRecord.m_PlayerId))
			{
				// Initialize the array inside first
				m_aDeathRecords.Insert(deathRecord.m_PlayerId, new array<ref SCR_IGameModeRecord>());
			}

			// Insert record for that player
			m_aDeathRecords[deathRecord.m_PlayerId].Insert(deathRecord);
		}
		
				
		// Sort deaths per player id
		m_aSpawnRecords.Clear();
		foreach (SCR_IGameModeRecord record : spawn)
		{
			SCR_SpawnRecord spawnRecord = SCR_SpawnRecord.Cast(record);
			if (!m_aSpawnRecords.Contains(spawnRecord.m_PlayerId))
			{
				// Initialize the array inside first
				m_aSpawnRecords.Insert(spawnRecord.m_PlayerId, new array<ref SCR_IGameModeRecord>());
			}

			// Insert record for that player
			m_aSpawnRecords[spawnRecord.m_PlayerId].Insert(spawnRecord);
		}

		// Sort LODs per EntityID
		m_aAILODRecords.Clear();
		foreach (SCR_IGameModeRecord record : lodRecords)
		{
			SCR_AILODRecord lodRecord = SCR_AILODRecord.Cast(record);
			if (!m_aAILODRecords.Contains(lodRecord.m_ID))
			{
				// Initialize the array inside first
				m_aAILODRecords.Insert(lodRecord.m_ID, new array<ref SCR_IGameModeRecord>());
			}

			// Insert record for that player
			m_aAILODRecords[lodRecord.m_ID].Insert(lodRecord);
		}

		// Get records duration
		if (records.Count() > 0)
			m_fMaxTime = records[records.Count() - 1].GetTimestamp();
		else
			m_fMaxTime = 0.0;

		// Fill player names map
		m_iUniquePlayerCount = 0;
		m_mPlayerNames.Clear();
		for (int i = 0, count = m_aConnectionRecords.Count(); i < count; ++i)
		{
			SCR_ConnectionRecord record = SCR_ConnectionRecord.Cast(m_aConnectionRecords[i]);
			int playerId = record.m_PlayerId;
			if (playerId <= 0)
				continue;

			if (!m_mPlayerNames.Contains(playerId))
			{
				string name = "Unnamed";
				if (!record.m_PlayerName.IsEmpty())
					name = record.m_PlayerName;

				m_mPlayerNames.Insert(playerId, name);
				++m_iUniquePlayerCount;
			}
		}

		m_vMins = min;
		m_vMaxs = max;
	}

	//------------------------------------------------------------------------------------------------
	//! Draws a check using the DbgUI.Check and returns true if value has changed
	private bool DrawCheck(string label, inout bool outValue)
	{
		bool previousValue = outValue;
		DbgUI.Check(label, outValue);
		return previousValue != outValue;
	}

	//------------------------------------------------------------------------------------------------
	//! Draws a slider using the DbgUI.SliderFloat and returns true if value has changed
	private bool DrawSlider(string label, inout float outValue, float min, float max, int pxWidth)
	{
		float previousValue = outValue;
		DbgUI.SliderFloat(label, outValue, min, max, pxWidth);
		return previousValue != outValue;
	}

	//------------------------------------------------------------------------------------------------
	//! Draw two buttons (single line) that subtract and add addValue to refValue.
	private bool DrawAdvancementButton(string negativeLabel, string positiveLabel, inout float refValue, float addValue)
	{
		float previousValue = refValue;
		if (DbgUI.Button(negativeLabel)) refValue = refValue - addValue;
		DbgUI.SameLine();
		if (DbgUI.Button(positiveLabel)) refValue = refValue + addValue;
		return (previousValue != refValue);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		bool isDirty = false;
		int pxWidth = 300;
		int spacerHeight = 32;

		DbgUI.Begin("GameMode Statistics");
		{
			// File selection
			if (IsEmpty())
			{
				DbgUI.Text("File to open: ");
				string filename = "MyFile.gmstats";
				DbgUI.InputText("", filename, pxWidth);
				DbgUI.SameLine();
				if (DbgUI.Button("Open")) 
				{
					// Open file and create runtime data.
					// Mark as dirty so all items are drawn
					OpenStatisticsFile(filename);
					LoadRecords(m_aRecords);
					
					// Usually there would be isDirty = true,
					// but to prevent crazy scopes, we directly repaint and leave
					Repaint();
				}

				DbgUI.Text("No data, load statistics first!");

				DbgUI.End();
				return;
			}

			// Closing file
			if (DbgUI.Button("Close"))
			{
				Close();

				// Reset playback
				m_fCurrentTime = 0.0;
				m_bAutoPlay = false;

				// No need to repaint, items are cleared
				DbgUI.End();
				return;
			}

			DbgUI.Text(string.Format("Total Records: %1", m_aRecords.Count()));

			// Individual filters and properties
			DbgUI.Spacer(spacerHeight);
			DbgUI.Text("Properties");
			if (DrawCheck("Enabled", m_bDraw)) isDirty = true;
			if (DrawCheck("Draw Death Records", m_bDrawDeaths)) isDirty = true;
			if (m_bDrawDeaths && DrawCheck("Draw Death Infoboxes", m_bDrawDeathsInfo)) isDirty = true;
			if (DrawCheck("Draw Movement Records", m_bDrawMovement)) isDirty = true;
			if (DrawCheck("Draw Player List", m_bDrawPlayerNames)) isDirty = true;
			if (DrawCheck("Draw AILODs", m_bDrawAILODs)) isDirty = true;
			if (DrawCheck("Limit History Length", m_bUseHistory)) isDirty = true;
			if (DrawCheck("No Z Buffer", m_bNoZBuffer)) isDirty = true;
			if (DrawCheck("Faction Colors", m_bFactionColors)) isDirty = true;
			if (DrawSlider("Gizmos Scale", m_fGizmosScaleMultiplier, 0.1, 25.0, pxWidth)) isDirty = true;
			if (DbgUI.Button("Re-center camera"))
				FocusCamera(m_vMins, m_vMaxs);

			// Draw playback and history specific properties
			if (m_bUseHistory)
			{
				DbgUI.Text("History Length = " + m_fHistoryLength * 0.001 + "s");
				if (DrawSlider("History Length", m_fHistoryLength, 100, 1000000, pxWidth)) isDirty = true;

				DbgUI.Spacer(spacerHeight);
				DbgUI.Text("Playback");
				DbgUI.Text(string.Format("Time: %1 / %2 s", m_fCurrentTime * 0.001, m_fMaxTime * 0.001));

				string autoPlayText;
				if (m_bAutoPlay)
					autoPlayText = "Pause";
				else
					autoPlayText = "Play";

				// Play/Pause
				if (DbgUI.Button(autoPlayText))
					m_bAutoPlay = !m_bAutoPlay;
				// Stop
				DbgUI.SameLine();
				if (DbgUI.Button("Stop"))
				{
					m_bAutoPlay = false;
					m_fCurrentTime = 0.0;
					isDirty = true;
				}

				// Playback speed
				if (DrawSlider("Playback Speed: " + m_fPlaybackSpeed + "x", m_fPlaybackSpeed, 0.0, 16.0, pxWidth)) {}

				// Seeking
				int seekTime;
				DbgUI.InputInt("Seek Time: (s)", seekTime, pxWidth);
				DbgUI.SameLine();
				if (DbgUI.Button("Seek"))
				{
					m_fCurrentTime = seekTime * 1000.0;
					isDirty = true;
				}

				// Draw manual advancement buttons
				if (DrawAdvancementButton("-1s", "+1s", m_fCurrentTime, 1000)) isDirty = true;
				DbgUI.SameLine();
				if (DrawAdvancementButton("-2s", "+2s", m_fCurrentTime, 2000)) isDirty = true;
				DbgUI.SameLine();
				if (DrawAdvancementButton("-5s", "+5s", m_fCurrentTime, 5000)) isDirty = true;

				if (DrawAdvancementButton("-10s", "+10s", m_fCurrentTime, 10000)) isDirty = true;
				DbgUI.SameLine();
				if (DrawAdvancementButton("-30s", "+30s", m_fCurrentTime, 30000)) isDirty = true;
				DbgUI.SameLine();
				if (DrawAdvancementButton("-60s", "+60s", m_fCurrentTime, 60000)) isDirty = true;

				if (DrawAdvancementButton("-100s", "+100s", m_fCurrentTime, 100000)) isDirty = true;
				DbgUI.SameLine();
				if (DrawAdvancementButton("-300s", "+300s", m_fCurrentTime, 300000)) isDirty = true;
				DbgUI.SameLine();
				if (DrawAdvancementButton("-600s", "+600s", m_fCurrentTime, 600000)) isDirty = true;

				// Clamp time to reasonable values
				m_fCurrentTime = Math.Clamp(m_fCurrentTime, 0, m_fMaxTime);

				// Advance playback if enabled
				if (m_bAutoPlay)
				{
					m_fCurrentTime += timeSlice * m_fPlaybackSpeed * 1000.0; // 1 second per second
					isDirty = true;
				}
			}

		}
		DbgUI.End();

		// Draw player list with utilities
		if (m_bDrawPlayerNames)
			if (DrawPlayerList(pxWidth, spacerHeight)) isDirty = true;

		// When dirty, re-load and filter records + repaint all shapes
		if (isDirty)
		{
			Repaint();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Repaint()
	{
		// Let all shapes begone!
		m_aShapes.Clear();
		m_aTexts.Clear();
		// And do not create new ones
		if (!m_bDraw)
			return;

		// Unless drawing something...
		if (m_bDrawMovement)
			RepaintMovement(m_iTargetPlayer);

		if (m_bDrawDeaths)
			RepaintDeaths(m_iTargetPlayer);
		
		if (m_bDrawAILODs)
			RepaintAILODs();
		
	}

	//------------------------------------------------------------------------------------------------
	protected bool DrawPlayerList(int pxWidth = 150, int spacerHeight = 16)
	{
		bool isDirty = false;
		DbgUI.Begin("Player List");
		{
			// Build text to display based on filtering
			string filterText = "Filter : ";
			if (m_iTargetPlayer <= 0)
				filterText += "All players";
			else
			{
				// Use target player name
				if (m_mPlayerNames.Contains(m_iTargetPlayer))
					filterText += m_mPlayerNames[m_iTargetPlayer];
				else
					filterText += "N/A";

				// Append id to text
				filterText = string.Format("%1 (Id: %2)", filterText, m_iTargetPlayer);
			}

			// Make sure to propagate changes to caller
			int previousValue = m_iTargetPlayer;
			DbgUI.Text(filterText);
			if (DrawSlider("", m_iTargetPlayer, 0, m_iUniquePlayerCount, pxWidth)) isDirty = true;

			// Offset player list
			DbgUI.Spacer(spacerHeight);
			DbgUI.Text("Players: ");
			foreach (int playerId, string playerName : m_mPlayerNames)
			{
				string playerText = string.Format("%1: %2", playerId, playerName);

				// Draw color panel to
				vector rgb = PLAYER_COLORS_ARRAY[ playerId % PLAYER_COLORS_COUNT ];
				int color = ARGB(255, rgb[0], rgb[1], rgb[2]);
				DbgUI.Panel(playerText+"_panel", 16, 16, color);
				DbgUI.SameLine();
				// Player id/name itself
				DbgUI.Text(playerText);
			}
		}
		DbgUI.End();
		return isDirty;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetAlpha(float timeStamp)
	{
		return Math.InverseLerp(m_fCurrentTime - m_fHistoryLength, m_fCurrentTime, timeStamp);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetShapeFlags()
	{
		int flags = ShapeFlags.TRANSP;
		if (m_bNoZBuffer)
			flags |= ShapeFlags.NOZBUFFER;

		return flags;
	}

	//------------------------------------------------------------------------------------------------
	protected void RepaintAILODs()
	{			
		vector characterRGB = PLAYER_COLORS_ARRAY[0];
		vector groupRGB = PLAYER_COLORS_ARRAY[1];
		int color;
		int shapeFlags = GetShapeFlags();
		float baseSphereScale = m_fGizmosScaleMultiplier * 0.25;
		float baseCylinderScale = m_fGizmosScaleMultiplier * 2.5;
		foreach(array<ref SCR_IGameModeRecord> lodRecords : m_aAILODRecords)
		{	
			
			for (int j = 0; j < lodRecords.Count(); j++)
			{
				SCR_AILODRecord aiRecord = SCR_AILODRecord.Cast(lodRecords[j]);
				SCR_AILODRecord prevAiRecord;
				if (j)
					prevAiRecord = SCR_AILODRecord.Cast(lodRecords[j-1]);
				float timeStamp = aiRecord.GetTimestamp();
				float alpha = 1.0;
			
				if (m_bUseHistory)
				{
					// We can skip if record is in the future
					if (timeStamp > m_fCurrentTime)
						break;
	
					// Otherwise blend out
					alpha = GetAlpha(timeStamp);
					if (alpha <= 0.0 || alpha > 1.0)  // If history is used and item is <almost> completely transparent, skip drawing it
					{
						continue;					 // additionally skip items which are further than now
					}
				}
				vector LODcolor = AILOD_COLORS_ARRAY[aiRecord.m_LOD];
				color = ARGB(255 * alpha, LODcolor[0], LODcolor[1], LODcolor[2]);
				
				float scaleAlpha = Math.Lerp(0.7, 1.0, alpha); // 70-100% scale based on age
				if (aiRecord.m_Type == ETypeOfAgent.Group)
				{
					ref Shape cylinder = Shape.CreateCylinder(color, shapeFlags | ShapeFlags.NOOUTLINE, aiRecord.m_Position, baseSphereScale * scaleAlpha, baseCylinderScale * scaleAlpha);
					m_aShapes.Insert(cylinder);
				}
				else if ((aiRecord.m_Type == ETypeOfAgent.Agent))
				{
					ref Shape sphere = Shape.CreateSphere(color, shapeFlags | ShapeFlags.NOOUTLINE, aiRecord.m_Position, baseSphereScale * scaleAlpha);
					m_aShapes.Insert(sphere);
				}
				else if ((aiRecord.m_Type == ETypeOfAgent.Bird))
				{
					vector points[7];
					points[0] = aiRecord.m_Position - Vector(m_fGizmosScaleMultiplier, 0, 0);
					points[1] = aiRecord.m_Position + Vector(m_fGizmosScaleMultiplier, 0, 0);
					points[2] = aiRecord.m_Position - Vector(0, m_fGizmosScaleMultiplier, 0);
					points[3] = aiRecord.m_Position + Vector(0, m_fGizmosScaleMultiplier, 0);
					points[4] = aiRecord.m_Position - Vector(0, 0, m_fGizmosScaleMultiplier);
					points[5] = aiRecord.m_Position + Vector(0, 0, m_fGizmosScaleMultiplier);
					points[6] = aiRecord.m_Position - Vector(m_fGizmosScaleMultiplier, 0, 0);
					ref Shape sphere = Shape.CreateLines(color, shapeFlags | ShapeFlags.NOOUTLINE, points, 7);
					m_aShapes.Insert(sphere);
				} 
				else 
				{
					ref Shape cylinder = Shape.CreateCylinder(ARGB(255 * alpha, 0xff, 0xff, 0 ),
					 shapeFlags | ShapeFlags.NOOUTLINE, aiRecord.m_Position, baseSphereScale * scaleAlpha, baseCylinderScale * scaleAlpha);
					m_aShapes.Insert(cylinder);
				}
				int btCounterNum = aiRecord.m_BTCounter;
				int percCounterNum = aiRecord.m_PerceptionCounter;
				if (!btCounterNum && !percCounterNum)
					continue;
				if (prevAiRecord)
				{
					btCounterNum -= prevAiRecord.m_BTCounter;
					percCounterNum -= prevAiRecord.m_PerceptionCounter;
				}
				string text = string.Format("%1,%2", btCounterNum, percCounterNum);
				vector mat[4];
				mat[0] = "1 0 0";
				mat[1] = "0 1 0";
				mat[2] = "0 0 1";
				mat[3] = aiRecord.m_Position + 3 * vector.Up;
				
				ref DebugText textShape = DebugTextWorldSpace.CreateInWorld(GetWorld(), text, DebugTextFlags.FACE_CAMERA | DebugTextFlags.CENTER, mat, 0.5, color, 0xFF111111);	
				m_aTexts.Insert(textShape);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RepaintDeaths(int filterPlayerId = -1)
	{
		const int black50A = ARGB(100, 0, 0, 0); // fallback color, so we don't call ARGB for each iteration
		int shapeFlags = GetShapeFlags();

		foreach (int playerId, array<ref SCR_IGameModeRecord> playerRecords : m_aDeathRecords)
		{
			// Draw elements only for target player
			if (filterPlayerId > 0)
			{
				if (filterPlayerId != playerId)
					continue;
			}
			
			array<ref SCR_IGameModeRecord> spawnRecords= m_aSpawnRecords.Get(playerId);
			int spawnId = 0;			
			
			const vector extents = "0.25 0.25 0.25";
			vector scaledExtents = m_fGizmosScaleMultiplier * extents;
			foreach (SCR_IGameModeRecord genericRecord : playerRecords)
			{
				SCR_DeathRecord record = SCR_DeathRecord.Cast(genericRecord); // We assume filtering is proper
				float timeStamp = record.GetTimestamp();
				float alpha = 1.0;

				if (m_bUseHistory)
				{
					// We can skip if record is in the future
					if (timeStamp > m_fCurrentTime)
						continue;

					// Otherwise blend out
					alpha = GetAlpha(timeStamp);
					if (alpha <= 0.0 || alpha > 1.0) // If history is used and item is <almost> completely transparent, skip drawing it
						continue;					 // additionally skip items which are further than now
				}

				int instigatorColor = black50A;
				
				vector victimRGB = PLAYER_COLORS_ARRAY[ record.m_PlayerId % PLAYER_COLORS_COUNT ];
				int victimColor;
				if (m_bFactionColors && spawnRecords)
				{
					while(spawnId > -1 && spawnRecords[spawnId].GetTimestamp() > timeStamp)
						spawnId--;
					while(spawnId+1 < spawnRecords.Count() && spawnRecords[spawnId+1].GetTimestamp() < timeStamp)
						spawnId++;
					victimColor = SCR_SpawnRecord.Cast(spawnRecords[spawnId]).m_FactionColor;
				}
				else 
					victimColor = ARGB(255 * alpha, victimRGB[0], victimRGB[1], victimRGB[2]);
			
				// The position at which kill was instigated from or self position if no instigator
				vector instigatorPosition;
				
				if (m_bDrawDeathsInfo)
				{
	
					// Text communicating the means of death
					string deathText;
	
					// Try to fetch player name for victim
					string victimName = "Unknown";
					if (m_mPlayerNames.Contains(record.m_PlayerId))
						victimName = m_mPlayerNames[record.m_PlayerId];
	
					// Try to fetch player name for instigator
					string instigatorName = "Unknown";
					if (record.m_InstigatorId > 0)
						if (m_mPlayerNames.Contains(record.m_InstigatorId))
							instigatorName = m_mPlayerNames[record.m_InstigatorId];
	
					// Draw an arrow from instigator origin towards victim origin, if posible
					// also skip suicidal cases, these do not need any
					if (record.m_InstigatorId != record.m_PlayerId &&
						record.m_InstigatorPosition != record.m_Position)
					{
						vector instigatorRGB = PLAYER_COLORS_ARRAY[ record.m_InstigatorId % PLAYER_COLORS_COUNT ];
						instigatorColor = ARGB(alpha * 255, instigatorRGB[0], instigatorRGB[1], instigatorRGB[2]);
	
						ref Shape arrowShape = Shape.CreateArrow(record.m_InstigatorPosition, record.m_Position, 0.5, instigatorColor, shapeFlags);
						m_aShapes.Insert(arrowShape);
	
						float distance = vector.Distance(record.m_InstigatorPosition, record.m_Position);
						deathText = string.Format("Victim: %1\nFaction: %2\n\nInstigator: %3\nFaction: %4\n\nDistance: %5m\nWeapon: %6\nTime: %7s",
							victimName,
							record.m_Faction,
							instigatorName,
							record.m_InstigatorFaction,
							distance,
							typename.EnumToString(EWeaponType, record.m_WeaponType),
							timeStamp * 0.001
						); // for ms -> s
	
						instigatorPosition = record.m_InstigatorPosition;
					}
					// In other case we explain that the user most likely died of unknown means or suicide
					else
					{
						deathText = string.Format("Victim: %1\nFaction:(%2)\n\nInstigator: (Self/Unknown)\n\nTime: %3s",
							victimName,
							record.m_Faction,
							timeStamp * 0.001
						);
	
						// Instigator is self
						instigatorPosition = record.m_Position;
						instigatorColor = victimColor;
					}
	
					// Background needs to interpolate color based on history as well
					int backgroundColor = ARGB(100 * alpha, 0.0, 0.0, 0.0);
	
					// Create text halway between instigator and victim position
					vector textPosition = vector.Lerp(record.m_Position, instigatorPosition, 0.5) + "0 0.1 0"; // Add slight offset upwards to prevent certain clipping issues
					ref DebugText textShape = DebugTextWorldSpace.Create(GetWorld(), deathText, DebugTextFlags.FACE_CAMERA | DebugTextFlags.CENTER, textPosition[0], textPosition[1], textPosition[2], 16.0, color: instigatorColor, bgColor: backgroundColor);
					m_aTexts.Insert(textShape);
				}

				// Draw an obvious shape (box) at the position of death				
				ref Shape deathShape = Shape.Create(ShapeType.BBOX, victimColor, shapeFlags,  -scaledExtents, scaledExtents);
				vector transformation[4];
				Math3D.MatrixIdentity3(transformation);
				transformation[3] = record.m_Position;
				deathShape.SetMatrix(transformation);

				m_aShapes.Insert(deathShape);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RepaintMovement(int filterPlayerId = -1)
	{
		int shapeFlags = GetShapeFlags();
		float baseSphereScale = m_fGizmosScaleMultiplier * 0.125;
		float baseArrowScale = m_fGizmosScaleMultiplier * 0.25;
		foreach (int playerId, array<ref SCR_IGameModeRecord> playerRecords : m_aMovementRecords)
		{
			// Draw elements only for target player
			if (filterPlayerId > 0)
			{
				if (filterPlayerId != playerId)
					continue;
			}
			
			
			array<ref SCR_IGameModeRecord> spawnRecords= m_aSpawnRecords.Get(playerId);
			int spawnId = 0;
			
			for (int i = 0, count = playerRecords.Count(); i < count - 1; i++)
			{
				SCR_MovementRecord record = SCR_MovementRecord.Cast(playerRecords[i]);
				float timeStamp = record.GetTimestamp();
				float alpha = 1.0;
				
				SCR_MovementRecord nextRecord = SCR_MovementRecord.Cast(playerRecords[i+1]);

				if (m_bUseHistory)
				{
					// We can skip if record is in the future
					if (timeStamp > m_fCurrentTime)
						continue;

					// Otherwise blend out
					alpha = GetAlpha(timeStamp);
					if (alpha <= 0.0 || alpha > 1.0) // If history is used and item is <almost> completely transparent, skip drawing it
						continue;					 // additionally skip items which are further than now
				}
				
				
				// Skip teleports
				if (vector.DistanceSqXZ(record.m_Position, nextRecord.m_Position) >= TELEPORT_DISTANCE_SQ)
					continue;
				
				vector playerRGB = PLAYER_COLORS_ARRAY[ record.m_PlayerId % PLAYER_COLORS_COUNT ];
				int playerColor;
				if (m_bFactionColors && spawnRecords)
				{
					while(spawnId > -1 && spawnRecords[spawnId].GetTimestamp() > timeStamp)
						spawnId--;
					while(spawnId+1 < spawnRecords.Count() && spawnRecords[spawnId+1].GetTimestamp() < timeStamp)
						spawnId++;
					playerColor = SCR_SpawnRecord.Cast(spawnRecords[spawnId]).m_FactionColor;
				}
				else 
					playerColor = ARGB(255 * alpha, playerRGB[0], playerRGB[1], playerRGB[2]);
				
				// Try to differentiate players by picking different colors

				// Draw balls on visited position
				float scaleAlpha = Math.Lerp(0.7, 1.0, alpha); // 70-100% scale based on age
				ref Shape arrowShape = Shape.CreateArrow(record.m_Position, nextRecord.m_Position, baseArrowScale * scaleAlpha, playerColor, shapeFlags);
				ref Shape sphereShape = Shape.CreateSphere(playerColor, shapeFlags | ShapeFlags.NOOUTLINE, record.m_Position, baseSphereScale * scaleAlpha);
				m_aShapes.Insert(arrowShape);
				m_aShapes.Insert(sphereShape);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusCamera(vector mins, vector maxs, float height = 100.0)
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		CameraBase camera = cameraManager.CurrentCamera();
		if (!camera)
			return;

		vector center = vector.Lerp(mins, maxs, 0.5);
		vector position = Vector(center[0], center[1] + height, center[2]);

		vector transform[4];
		Math3D.AnglesToMatrix("0 -89.9 0", transform);
		transform[3] = position;

		camera.SetWorldTransform(transform);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_StatisticsDrawerEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		Activate();
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_StatisticsDrawerEntity()
	{
		if (m_aRecords)
			Close();
	}
}
#endif
