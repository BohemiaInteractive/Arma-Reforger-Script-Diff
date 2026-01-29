enum SCR_EAIThreatSectorFlags
{
	// These flags can be used outside, to check state of sector
	DIRECTED_AT_ME = 1<<0,	// The threat is directed at us during this sector's activity
	CAUSED_DAMAGE = 1<<1,	// The threat caused us damage during this sector's activity

	// These flags are internal, probably meaningless outside
	INVOKE_ON_DAMAGE_TAKEN = 1<<2	// Causes the damage taken invoker to be invoked at next update
}

//! Data of one sector of SCR_AISectorThreatFilter
class SCR_AIThreatSector
{
	// Constants
	static const int SECTOR_STATE_IDLE		= 0;
	static const int SECTOR_STATE_ACTIVE	= 1;

	protected static const float ACCUMULATOR_DROP_RATE = 0.05;	// Percents per second
	protected static const float SECTOR_FORGET_S = 45.0;		// Time since last event until sector switches back to IDLE

	// Member variables:

	// Timestamps
	WorldTimestamp m_fTimestampLastEvent;				// Last event
	WorldTimestamp m_fTimestampStart;					// When the sector became active (first event)
	WorldTimestamp m_fTimestampLastUpdateWithEvents;	// Last update when values were moved from m_fDangerSinceLastUpdate to m_fDanger

	vector m_vEstimatedPos;			// Estimated (averaged) position of this sector
	float m_fDanger; 				// Current danger value
	float m_fNextDangerEscalation;	// Threshold of danger value when escalation event will be invoked
	int m_iState;
	SCR_EAIThreatSectorFlags m_eFlags;	// Flags of the sector

	// Accumulator of danger since last update. Events add value here, and on update we transfer them to m_fDanger.
	float m_fDangerSinceLastUpdate;			// Accumulated danger added since last update
	float m_fMaxDangerBumpSinceLastUpdate;	// Max danger since last update
	int m_iEventCountSinceLastUpdate;		// Count of events accumulated since last update

	void Reset()
	{
		m_fTimestampLastEvent = null;
		m_fTimestampStart = null;
		m_fTimestampLastUpdateWithEvents = null;

		m_vEstimatedPos = vector.Zero;
		m_fDanger = 0;
		m_fNextDangerEscalation = 0;
		m_iState = SECTOR_STATE_IDLE;
		m_eFlags = 0;

		m_fDangerSinceLastUpdate = 0;
		m_fMaxDangerBumpSinceLastUpdate = 0;
		m_iEventCountSinceLastUpdate = 0;
	}

	//! Merges data from two sectors into this sector
	void MergeWith(notnull SCR_AIThreatSector other)
	{
		m_vEstimatedPos = 0.5 * (m_vEstimatedPos + other.m_vEstimatedPos);
		m_fDanger = Math.Max(m_fDanger, other.m_fDanger);
		m_fNextDangerEscalation = Math.Max(m_fNextDangerEscalation, other.m_fNextDangerEscalation);
		//m_iSectorState; // Must be rechecked afterwards again
		m_eFlags = m_eFlags | other.m_eFlags;

		// For timestamps, we set this timestamp to greater of the two timestamps
		if (m_fTimestampLastEvent.Less(other.m_fTimestampLastEvent))
			m_fTimestampLastEvent = other.m_fTimestampLastEvent;

		if (m_fTimestampStart.Less(other.m_fTimestampStart))
			m_fTimestampStart = other.m_fTimestampStart;

		if (m_fTimestampLastUpdateWithEvents.Less(other.m_fTimestampLastUpdateWithEvents))
			m_fTimestampLastUpdateWithEvents = other.m_fTimestampLastUpdateWithEvents;

		m_fDangerSinceLastUpdate = m_fDangerSinceLastUpdate + other.m_fDangerSinceLastUpdate;
		m_fMaxDangerBumpSinceLastUpdate = Math.Max(m_fMaxDangerBumpSinceLastUpdate, other.m_fMaxDangerBumpSinceLastUpdate);
		m_iEventCountSinceLastUpdate = m_iEventCountSinceLastUpdate + other.m_iEventCountSinceLastUpdate;
	}
}

// Called when danger value becomes higher than previous max danger value for the sector, during whole lifetime of sector
void SCR_AISectorThreatFilter_OnEscalation(SCR_AISectorThreatFilter ts, int sectorId, float dangerValue);
typedef func SCR_AISectorThreatFilter_OnEscalation;

void SCR_AISectorThreatFilter_OnMajorSectorChanged(SCR_AISectorThreatFilter ts, int newSectorId, int oldSectorId, float dangerValue);
typedef func SCR_AISectorThreatFilter_OnMajorSectorChanged;

void SCR_AISectorThreatFilter_OnDamageTaken(SCR_AISectorThreatFilter ts, int sectorId);
typedef func SCR_AISectorThreatFilter_OnDamageTaken;

//! This object splits incoming events into sectors based on direction. It can maintain up to SECTOR_COUNT sectors.
//! This class meant to solve soldier's awarenes of danger events around him, by tracking not only amount of danger, but also direction to danger.
//! This is designed to be used for very numerous events, such as gunshots.
class SCR_AISectorThreatFilter
{
	// Amount of sectors
	const int SECTOR_COUNT = 2; // Not all code can deal with more sectors than 2! Don't blindly change it! Right now we can really handle only two sectors.

	//------------------------------------------------------------------------------------------------
	// Member variables

	protected ref SCR_AIThreatSector m_aSectors[SECTOR_COUNT];

	// Post-processing of sectors
	protected int m_iSectorMajor = -1; // ID of major sector, or -1
	protected int m_iSectorMinor = -1; // ID of minor sector, or -1

	protected IEntity m_MyEntity;

	//------------------------------------------------------------------------------------------------
	// Events

	// Called when danger value in sector exceeds threshold of next escalation danger value
	// Escalation events represent ability to get used to stream of danger events.
	// Next escalation event happens only when situation gets far more dangerous than we are already subjected to.
	protected ref ScriptInvokerBase<SCR_AISectorThreatFilter_OnEscalation> m_OnEscalation = new ScriptInvokerBase<SCR_AISectorThreatFilter_OnEscalation>();

	// Called when major sector ID switches
	protected ref ScriptInvokerBase<SCR_AISectorThreatFilter_OnMajorSectorChanged> m_OnMajorSectorChanged = new ScriptInvokerBase<SCR_AISectorThreatFilter_OnMajorSectorChanged>();

	// Called every time when we take damage from a sector
	protected ref ScriptInvokerBase<SCR_AISectorThreatFilter_OnDamageTaken> m_OnDamageTaken = new ScriptInvokerBase<SCR_AISectorThreatFilter_OnDamageTaken>();

	//------------------------------------------------------------------------------------------------
	// Constans
	protected static const float COS_SECTOR_HALF_ANGLE = 0.5; // Cos 60 deg -> whole sector is 120 deg

	protected static const float SECTOR_FORGET_S = 45.0;	// Time since last event until sector switches back to IDLE
	protected static const float SECTOR_URGENT_S = 1.5;	// Time since start when sector is considered to require urgent attention
	protected static const float URGENT_SECTOR_PRIORITY_MULTIPLIER = 10; // Used when calculating which sector is major

	protected static const float ACCUMULATOR_DROP_RATE = 0.05; // Percents per second

	protected static const float NEXT_ESCALATION_MULTIPLIER = 1.8; // Danger value must exceed danger at previous escalation event, multiplied by this constant

	//------------------------------------------------------------------------------------------------
	void SCR_AISectorThreatFilter(IEntity myEntity)
	{
		m_MyEntity = myEntity;

		for (int i = 0; i < SECTOR_COUNT; i++)
			m_aSectors[i] = new SCR_AIThreatSector();
	}


	//------------------------------------------------------------------------------------------------
	// Getters
	// Avoid accessing SCR_AIThreatSector object yourself.
	// The fact that sectors are stored as objects is an underlying implementation detail, which might change.
	// Therefore use those getters instead.

	//! Returns sectors to outside. Can return -1 if the sector is inactive.
	void GetActiveSectors(out int outSectorMajor, out int outSectorMinor)
	{
		outSectorMajor = m_iSectorMajor;
		outSectorMinor = m_iSectorMinor;
	}

	vector GetSectorPos(int sectorId)
	{
		return m_aSectors[sectorId].m_vEstimatedPos;
	}

	float GetSectorDanger(int sectorId)
	{
		return m_aSectors[sectorId].m_fDanger;
	}

	SCR_EAIThreatSectorFlags GetSectorFlags(int sectorId)
	{
		return m_aSectors[sectorId].m_eFlags;
	}

	bool IsSectorActive(int sectorId)
	{
		return m_aSectors[sectorId].m_iState == SCR_AIThreatSector.SECTOR_STATE_ACTIVE;
	}

	//------------------------------------------------------------------------------------------------
	//! Primarily used for save/load. Do not call unless you know what you are doing!
	SCR_AIThreatSector GetSector(int sectorId)
	{
		if (!sectorId >= SECTOR_COUNT)
			return null;

		return m_aSectors[sectorId];
	}

	//! See m_OnEscalation
	ScriptInvokerBase<SCR_AISectorThreatFilter_OnEscalation> GetOnEscalationInvoker()
	{
		return m_OnEscalation;
	}

	//! See m_OnMajorSectorChanged
	ScriptInvokerBase<SCR_AISectorThreatFilter_OnMajorSectorChanged> GetOnMajorSectorChangedInvoker()
	{
		return m_OnMajorSectorChanged;
	}

	//! See m_OnDamageTaken
	ScriptInvokerBase<SCR_AISectorThreatFilter_OnDamageTaken> GetOnDamageTaken()
	{
		return m_OnDamageTaken;
	}

	//------------------------------------------------------------------------------------------------
	//! Must be called periodically
	//! Udpates state of each sector
	//! Selects which sector is more relevant (major sector) and the other one (minor sector)
	void Update(float timeSlice_s)
	{		
		vector myPos = GetMyPos();

		// Merge sectors if they are too close directionally. For instance if we moved far away.
		// There are two of them, thus it is simplified.
		if (m_aSectors[0].m_iState != SCR_AIThreatSector.SECTOR_STATE_IDLE && m_aSectors[1].m_iState != SCR_AIThreatSector.SECTOR_STATE_IDLE)
		{
			vector dir0 = (m_aSectors[0].m_vEstimatedPos - myPos).Normalized();
			vector dir1 = (m_aSectors[1].m_vEstimatedPos - myPos).Normalized();
			float cosAngle = vector.Dot(dir0, dir1);
			if (cosAngle > COS_SECTOR_HALF_ANGLE)
			{
				// Merge all data to sector 0, reset sector 1
				m_aSectors[0].MergeWith(m_aSectors[1]);
				m_aSectors[1].Reset();
			}
		}

		// Update state of each sector
		WorldTimestamp worldTimestamp = GetGame().GetWorld().GetTimestamp();
		for (int i = 0; i < SECTOR_COUNT; i++)
		{
			SCR_AIThreatSector s = m_aSectors[i];

			// Idle?
			if (s.m_iState == SCR_AIThreatSector.SECTOR_STATE_IDLE)
				continue;

			// Not idle

			// Forget?
			if (worldTimestamp.DiffSeconds(s.m_fTimestampLastEvent) > SECTOR_FORGET_S)
			{
				// This also sets state to IDLE
				s.Reset();
			}

			float dangerPerSecond = 0;
			if (s.m_iEventCountSinceLastUpdate > 0)
			{
				// If many events happened, we use delta time
				// If one event happened, we use time since last update with events, which is larger, and should be more accurate
				float t;
				if (s.m_iEventCountSinceLastUpdate == 1 && s.m_fTimestampLastUpdateWithEvents != 0)
					t = worldTimestamp.DiffSeconds(s.m_fTimestampLastUpdateWithEvents);
				else
					t = timeSlice_s;
				dangerPerSecond = s.m_fDangerSinceLastUpdate / t;
			}

			if (s.m_iEventCountSinceLastUpdate > 0 || s.m_fMaxDangerBumpSinceLastUpdate > 0)
				s.m_fTimestampLastUpdateWithEvents = worldTimestamp;

			// Danger value is bumped to danger per secont value, or biggest bump value from events since last update, whichever is bigger
			float bumpDangerTo = dangerPerSecond;
			if (s.m_fMaxDangerBumpSinceLastUpdate > bumpDangerTo)
				bumpDangerTo = s.m_fMaxDangerBumpSinceLastUpdate;

			// Bump danger value, or decrease it
			if (bumpDangerTo > s.m_fDanger)
			{
				s.m_fDanger = bumpDangerTo;
				if (bumpDangerTo > s.m_fNextDangerEscalation)
				{
					s.m_fNextDangerEscalation = NEXT_ESCALATION_MULTIPLIER * bumpDangerTo;
					m_OnEscalation.Invoke(this, i, bumpDangerTo);
				}
			}
			else
			{
				s.m_fDanger = s.m_fDanger * (1.0 - timeSlice_s * ACCUMULATOR_DROP_RATE);
				if (s.m_fDanger < 0)
					s.m_fDanger = 0;
			}

			s.m_fDangerSinceLastUpdate = 0;
			s.m_fMaxDangerBumpSinceLastUpdate = 0;
			s.m_iEventCountSinceLastUpdate = 0;

			// Invoke damage taken event
			if (s.m_eFlags & SCR_EAIThreatSectorFlags.INVOKE_ON_DAMAGE_TAKEN)
			{
				m_OnDamageTaken.Invoke(this, i);
				s.m_eFlags &= ~(SCR_EAIThreatSectorFlags.INVOKE_ON_DAMAGE_TAKEN);
			}
		}

		// Select major and minor sector
		float priority[SECTOR_COUNT];
		for (int i = 0; i < SECTOR_COUNT; i++)
		{
			SCR_AIThreatSector s = m_aSectors[i];
			if (s.m_iState == SCR_AIThreatSector.SECTOR_STATE_IDLE)
				continue;

			// Calculate priority
			float p = s.m_fDanger;
			if (worldTimestamp.DiffSeconds(s.m_fTimestampStart) < SECTOR_URGENT_S)
				p *= URGENT_SECTOR_PRIORITY_MULTIPLIER;

			priority[i] = p;
		}

		// Sort sectors by priority
		int sectorIdsSorted[SECTOR_COUNT]; // ID 0 - highest priority
		sectorIdsSorted[0] = 0;
		sectorIdsSorted[1] = 1;
		if (priority[1] > priority[0])
		{
			sectorIdsSorted[0] = 1;
			sectorIdsSorted[1] = 0;
		}

		int newSectorMajor = -1;
		int newSectorMinor = -1;
		if (priority[sectorIdsSorted[0]] > 0)
		{
			newSectorMajor = sectorIdsSorted[0];

			if (priority[sectorIdsSorted[1]] > 0)
				newSectorMinor = sectorIdsSorted[1];
		}

		int prevSectorMajor = m_iSectorMajor;
		m_iSectorMajor = newSectorMajor;
		m_iSectorMinor = newSectorMinor;

		if (m_iSectorMajor != prevSectorMajor)
		{
			float dangerForEvent = 0;
			if (m_iSectorMajor != -1)
				dangerForEvent = m_aSectors[m_iSectorMajor].m_fDanger;
			m_OnMajorSectorChanged.Invoke(this, m_iSectorMajor, prevSectorMajor, dangerForEvent);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Handling of danger events and danger values

	// Input settings to danger calculations
	// If a single gun is firing with REFERENCE_RATE_OF_FIRE, at REFERENCE_DISTANCE, the average danger per second value will be ~1.0.
	// At further range danger decreases as 1/x.
	// At range of 0, danger is increased by DANGER_MULT_ZERO.
	const float REFERENCE_RATE_OF_FIRE = 10.8333; // 650.0 / 60.0 Rate of fire of AK-74, very common weapon
	const float REFERENCE_DISTANCE_INV = 0.01; // 100m
	const float DANGER_MULT_ZERO = 7; // How many times danger at 0 distance is bigger than danger at REFERENCE_DISTANCE

	const float GUNSHOT_DANGER_CONST_C = 1.0 / (DANGER_MULT_ZERO - 1.0);
	const float GUNSHOT_DANGER_CONST_A = (DANGER_MULT_ZERO / (DANGER_MULT_ZERO - 1.0)) / REFERENCE_RATE_OF_FIRE;

	const float DANGER_MULT_FLYBY = 20; // How many times a danger from a bullet flyby is increased

	// To which value danger is bumped when we take some damage.
	// Idea is to have it a reasonably high level, somewhere next to what happens when we are under fire.
	// But not too high, as it would set next escalation threshold too high.
	const float DAMAGE_TAKEN_DANGER_BUMP_TO = 20;

	//------------------------------------------------------------------------------------------------
	//! Must be called from gunshot event
	void OnShotsFired(vector eventPos, int count, bool isFlyby)
	{
		float dist = vector.Distance(GetMyPos(), eventPos);
		float accumulatorDelta = GUNSHOT_DANGER_CONST_A / (REFERENCE_DISTANCE_INV * dist + GUNSHOT_DANGER_CONST_C);

		SCR_EAIThreatSectorFlags flagsToAdd = 0;
		if (isFlyby)
		{
			flagsToAdd = SCR_EAIThreatSectorFlags.DIRECTED_AT_ME;
			accumulatorDelta = DANGER_MULT_FLYBY * accumulatorDelta;
		}

		int sectorId = FindSectorForEvent(eventPos);
		AddNewEventsToSector(sectorId, eventPos, accumulatorDelta, count, 0, flagsToAdd);
	}

	//------------------------------------------------------------------------------------------------
	void OnDamageTaken(vector eventPos)
	{
		int sectorId = FindSectorForEvent(eventPos);
		AddNewEventsToSector(sectorId, eventPos, 0, 0, DAMAGE_TAKEN_DANGER_BUMP_TO, SCR_EAIThreatSectorFlags.DIRECTED_AT_ME | SCR_EAIThreatSectorFlags.CAUSED_DAMAGE | SCR_EAIThreatSectorFlags.INVOKE_ON_DAMAGE_TAKEN);
	}

	//------------------------------------------------------------------------------------------------
	//! Must be called from explosion event
	void OnExplosion(vector eventPos)
	{
		float dist = vector.Distance(GetMyPos(), eventPos);
		float bumpDangerTo = 1.0 / (0.05 *dist + 1.0);
		int sectorId = FindSectorForEvent(eventPos);
		AddNewEventsToSector(sectorId, eventPos, 0, 1, bumpDangerTo);
	}

	//------------------------------------------------------------------------------------------------
	// Internal

	//------------------------------------------------------------------------------------------------
	//! There are two ways which can be used to 'bump' danger values in sector. Both ways can be used in same call as well.
	//! 1. Provide how many events happened (eventCount) and what was increment per each event (dangerDeltaPerEvent).
	//!   At next update the provided values get summed, danger per second is calculated, and sector's danger value is bumped to that value, if it's higher.
	//!   This is meant for frequent events like gunshots.
	//! 2. Provide value to which sector's danger value should be bumped to, if it's higher (bumpDangerTo)
	//!   This is meant for infrequent events like explosions.
	protected void AddNewEventsToSector(int sectorId, vector eventPos, float dangerDeltaPerEvent, int eventCount, float bumpDangerTo, SCR_EAIThreatSectorFlags flagsToAdd = 0)
	{
		WorldTimestamp worldTimestamp = GetGame().GetWorld().GetTimestamp();

		SCR_AIThreatSector s = m_aSectors[sectorId];

		// Move the estimated position
		if (s.m_iState == SCR_AIThreatSector.SECTOR_STATE_IDLE)
		{
			s.m_vEstimatedPos = eventPos;

			// Also switch state to active, set initial timestamp
			s.m_iState = SCR_AIThreatSector.SECTOR_STATE_ACTIVE;
			s.m_fTimestampStart = worldTimestamp;
		}
		else
		{
			vector myPos = GetMyPos();

			// New sector position is weighted average between previous pos and new pos
			// If distances are same, it's average between old and new pos.
			// If new distance is smaller, the new position is taken with much bigger weight
			float ratio = vector.Distance(myPos, eventPos) / vector.Distance(myPos, s.m_vEstimatedPos);
			float weight = 1.0 / (1.0 + ratio); // ratio 0 -> weight 1.0; ratio 1 -> weight 0.5; ratio > 1 -> weight decreases

			for (int i = 0; i < eventCount; i++)
			{
				s.m_vEstimatedPos = weight * eventPos + (1.0 - weight) * s.m_vEstimatedPos;
			}
		}

		// Bump danger value
		if (bumpDangerTo > s.m_fMaxDangerBumpSinceLastUpdate)
			s.m_fMaxDangerBumpSinceLastUpdate = bumpDangerTo;

		// Add danger value to the accumulator, update timestamps
		s.m_fDangerSinceLastUpdate = s.m_fDangerSinceLastUpdate + dangerDeltaPerEvent * eventCount;
		s.m_fTimestampLastEvent = worldTimestamp;
		s.m_eFlags = s.m_eFlags | flagsToAdd;
		s.m_iEventCountSinceLastUpdate = s.m_iEventCountSinceLastUpdate + eventCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds closest (by angle) sector for event, or returns index of an inactive sector, if there's one.
	protected int FindSectorForEvent(vector eventPos)
	{
		vector myPos = GetMyPos();
		vector eventDir = (eventPos - myPos).Normalized();

		float cosAngles[SECTOR_COUNT];

		// Try to find if it belongs to any active sector
		for (int i = 0; i < SECTOR_COUNT; i++)
		{
			SCR_AIThreatSector s = m_aSectors[i];

			cosAngles[i] = -2.0;

			if (s.m_iState == SCR_AIThreatSector.SECTOR_STATE_IDLE)
				continue;

			vector sectorDir = (s.m_vEstimatedPos - myPos).Normalized();
			float cosAngle = vector.Dot(sectorDir, eventDir);

			if (cosAngle > COS_SECTOR_HALF_ANGLE)
				return i;

			cosAngles[i] = cosAngle;
		}

		// Try to find any inactive sector
		for (int i = 0; i < SECTOR_COUNT; i++)
		{
			if (m_aSectors[i].m_iState == SCR_AIThreatSector.SECTOR_STATE_IDLE)
				return i;
		}

		// All sectors are active, and event is not in any of them
		// Try to find closest active sector
		int bestSectorId = -1;
		float bestCosAngle = -2.0;
		for (int i = 0; i < SECTOR_COUNT; i++)
		{
			float cosAngle = cosAngles[i];
			if (cosAngle > bestCosAngle)
			{
				bestSectorId = i;
				bestCosAngle = cosAngle;
			}
		}

		return bestSectorId;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetMyPos() { return m_MyEntity.GetOrigin(); }

	//------------------------------------------------------------------------------------------------
	void EOnDiag(float timeSlice)
	{
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SECTOR_THREAT_FILTER))
			return;

		WorldTimestamp worldTimestamp = GetGame().GetWorld().GetTimestamp();

		vector myPos = GetMyPos();
		vector arrowStartPos = myPos + Vector(0, 2, 0);
		const float arrowLength = 3.0;
		const float aimArrowLength = 4.5;
		for (int i = 0; i < SECTOR_COUNT; i++)
		{
			SCR_AIThreatSector s = m_aSectors[i];

			if (s.m_iState == SCR_AIThreatSector.SECTOR_STATE_IDLE)
				continue;

			vector sectorDir = (s.m_vEstimatedPos - myPos).Normalized();
			vector arrowEndPos = arrowStartPos + arrowLength * sectorDir;
			vector textPos = arrowEndPos + 1.0 * sectorDir;

			Shape.CreateArrow(arrowStartPos, arrowEndPos, 0.2, Color.SPRING_GREEN, ShapeFlags.ONCE);

			string s0 = string.Format("%1 Dngr: %2, NextEscal: %3, Age: %4", i, s.m_fDanger.ToString(5, 3), s.m_fNextDangerEscalation.ToString(5, 3), (worldTimestamp.DiffSeconds(s.m_fTimestampLastEvent)).ToString(4, 1));
			if (i == m_iSectorMajor)
				s0 = s0 + " MAIN";
			if (s.m_eFlags & SCR_EAIThreatSectorFlags.DIRECTED_AT_ME)
				s0 = s0 + " DIRECTED";
			if (s.m_eFlags & SCR_EAIThreatSectorFlags.CAUSED_DAMAGE)
				s0 = s0 + " DMG";

			DebugTextWorldSpace.Create(GetGame().GetWorld(), s0, DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
					textPos[0], textPos[1], textPos[2], color: Color.SPRING_GREEN, bgColor: Color.BLACK,
					size: 13.0);

			Shape.CreateSphere(Color.SPRING_GREEN, ShapeFlags.ONCE | ShapeFlags.WIREFRAME, s.m_vEstimatedPos, 0.5);
		}
	}
}
