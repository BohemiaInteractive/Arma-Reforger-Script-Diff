[BaseContainerProps()]
class SCR_TimedPositionSave
{
	[Attribute()]
	float m_fTime;

	[Attribute()]
	vector m_vRotX;

	[Attribute()]
	vector m_vRotY;

	[Attribute()]
	vector m_vRotZ;

	[Attribute()]
	vector m_vPos;

	vector m_aTransform[4];

	//------------------------------------------------------------------------------------------------
	static SCR_TimedPositionSave Create(float time, vector trans[4])
	{
		SCR_TimedPositionSave inst = new SCR_TimedPositionSave();
		inst.m_fTime = time;
		inst.m_vRotX = trans[0];
		inst.m_vRotY = trans[1];
		inst.m_vRotZ = trans[2];
		inst.m_vPos = trans[3];
		inst.RebuildTransform();
		return inst;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aTransform from the individual row attributes; call after loading from a .conf resource
	//! (BaseContainerProps fields are populated directly, m_aTransform is not serialized itself).
	void RebuildTransform()
	{
		m_aTransform[0] = m_vRotX;
		m_aTransform[1] = m_vRotY;
		m_aTransform[2] = m_vRotZ;
		m_aTransform[3] = m_vPos;
	}
}

//! Holds one recorded lap. A default (track-authored) ghost is loaded straight from a .conf resource
//! (see SCR_RaceEntity.m_sDefaultGhostFile) and never touches SCR_PersistentRaceData/SaveGameManager.
[BaseContainerProps()]
class SCR_GhostSave
{
	[Attribute()]
	string m_sName;

	//! Vehicle prefab used to record this ghost. Empty on legacy data - playback falls back to the
	//! live player's current vehicle in that case (see SCR_RaceEntity.SpawnGhost callers).
	[Attribute()]
	ResourceName m_sVehiclePrefab;

	[Attribute()]
	ref array<ref SCR_TimedPositionSave> m_aPositions;

	//------------------------------------------------------------------------------------------------
	//! Must be called once after loading from a resource, since m_aTransform is not itself serialized.
	void RebuildTransforms()
	{
		if (!m_aPositions)
			return;

		foreach (SCR_TimedPositionSave pos : m_aPositions)
		{
			if (pos)
				pos.RebuildTransform();
		}
	}
}

class SCR_RaceSave
{
	float m_fGoalReachedTime;
	ref array<ref SCR_GhostSave> m_aGhost = {};
}

class SCR_PersistentRaceData : PersistentState
{
	protected ref array<ref SCR_RaceSave> m_aRaces = {};

	//------------------------------------------------------------------------------------------------
	void OnRaceDone(SCR_RaceSave race)
	{
		m_aRaces.Clear(); //delete the previous ghost
		m_aRaces.Insert(race);
	}

	//------------------------------------------------------------------------------------------------
	static void OnRaceFinishedCode(SCR_RaceSave race)
	{
		SaveRace(race);
		GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.SCRIPTED);
	}

	//------------------------------------------------------------------------------------------------
	static void SaveRace(SCR_RaceSave race)
	{
		SCR_PersistentRaceData raceData = SCR_PersistentRaceData.Cast(PersistenceSystem.GetInstance().GetPersistentState(SCR_PersistentRaceData));

		raceData.OnRaceDone(race);
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_RaceSave> GetRaces()
	{
		return SCR_ArrayHelperRefT<SCR_RaceSave>.GetNonRefCopy(m_aRaces);
	}

	//------------------------------------------------------------------------------------------------
	static void DebugRaces()
	{
		SCR_PersistentRaceData raceData = SCR_PersistentRaceData.Cast(PersistenceSystem.GetInstance().GetPersistentState(SCR_PersistentRaceData));

		foreach (SCR_RaceSave race : raceData.m_aRaces)
		{
			Print("----- RACE RECORD------", LogLevel.NORMAL);
			PrintFormat("Time: %1", race.m_fGoalReachedTime, level: LogLevel.NORMAL);
			PrintFormat("Ghosts:", level: LogLevel.NORMAL);
			foreach (SCR_GhostSave ghost : race.m_aGhost)
			{
				PrintFormat("%1 - %2", ghost.m_sName, ghost.m_aPositions, level: LogLevel.NORMAL);
			}
		}
	}
}

class SCR_PersistentRaceDataSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PersistentRaceData;
	}
}
