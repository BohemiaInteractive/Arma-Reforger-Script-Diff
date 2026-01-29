//! Behavior tree node which controls looking.
//! It takes data from threat system and alternates looking at major and minor threat sector, with a pause in between.
//! The loop is this: look at major sector -> intermission -> look at minor sector -> intermission -> repeat
class SCR_AILookAtThreatSystemLogic : AITaskScripted
{
	// Outputs
	protected static const string PORT_CURRENT_POSITION = "CurrentPos";	// Position where we currently want to look at (major or minor threat)
	protected static const string PORT_RESET_LOOK = "ResetLook";
	protected static const string PORT_ACTIVE = "Active";
	protected static const string PORT_RAISE_WEAPON = "RaiseWeapon";
	protected static const string PORT_USE_OPTICS = "UseOptics";
	
	protected static const int STATE_IDLE = 0; 				// Nothing to look at, although this shouldn't be possible.
	protected static const int STATE_OBSERVING_MAJOR = 1;	// Looking at major sector (most dangerous)
	protected static const int STATE_OBSERVING_MINOR = 2;	// Looking at minor sector
	protected static const int STATE_INTERMISSION = 3;		// A pause after looking
	
	protected static const float DANGER_THRESHOLD_RAISE_WEAPON = 0.5; // Danger of sector beyond which we raise weapon
	protected static const float DISTANCE_USE_OPTICS_SQ = 200*200;
	
	protected SCR_AISectorThreatFilter m_ThreatFilter;
	protected IEntity m_MyEntity;
	protected int m_iSectorMajor;
	protected int m_iSectorMinor;
	
	protected int m_iState;
	protected int m_iStateAfterIntermission;
	protected float m_fTimer;
	protected float m_fTimerThreshold;
	
	protected WorldTimestamp m_TimestampLastUpdate;
	
	[Attribute("1", UIWidgets.EditBox)]
	protected float m_fLookDurationMajor;
	
	[Attribute("1", UIWidgets.EditBox)]
	protected float m_fLookDurationMinor;
	
	[Attribute("0", UIWidgets.EditBox, "Duration of waiting after looking")]
	protected float m_fIntermissionDuration;
	
	[Attribute("1", UIWidgets.EditBox, "Timers will be randomized between 1.0 and this value")]
	protected float m_fTimerRandomizationCoefficient;
	
	[Attribute("0", UIWidgets.ComboBox, "Evaluate if we should raise weapon, and use optics?")]
	protected bool m_bEvaluateEquipmentUsage;
	
	//---------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		m_ThreatFilter = utility.m_SectorThreatFilter;
		m_MyEntity = utility.m_OwnerEntity;
		
		Reset();
	}
	
	//---------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		Reset();
	}
	
	//---------------------------------------------------------------------------
	protected void Reset()
	{
		m_iSectorMajor = -1;
		m_iSectorMinor = -1;
		m_iState = STATE_IDLE;
		m_fTimer = 0;
		m_fTimerThreshold = 0;
	}
	
	//---------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_ThreatFilter)
			return ENodeResult.FAIL;
		
		// Update timestamp, calculate timeSlice
		WorldTimestamp worldTimestamp = GetGame().GetWorld().GetTimestamp();
		float timeSlice_s = worldTimestamp.DiffSeconds(m_TimestampLastUpdate);
		m_TimestampLastUpdate = worldTimestamp;
		
		bool resetLook = false;		// We reset it whenever we alternate sectors
		int prevState = m_iState;
		
		//-------------------------------------------------
		/*
		The logic of the state machine:
		Alternate looking at major sector, and then at minor sector, with different duration.
		If major sector from threat system changes, switch to looking at it.
		*/
		
		int sectorMajor, sectorMinor;
		m_ThreatFilter.GetActiveSectors(sectorMajor, sectorMinor);
		
		switch (m_iState)
		{
			// Nothing to look at right now
			case STATE_IDLE:
			{
				if (sectorMajor != -1) // There is now something to look at
					SwitchState_ObserveMajorSector();
				break;
			}
			
			case STATE_OBSERVING_MAJOR:
			{
				m_fTimer += timeSlice_s;
				
				if (sectorMajor == -1)
					SwitchState_Idle(); // Nothing to look at any more
				else if (sectorMajor != m_iSectorMajor)
				{
					// Major sector changed, so something important has happened
					// Reset the looking in BT, restart the timer
					resetLook = true;
					SwitchState_ObserveMajorSector();
				}
				else if (m_fTimer > m_fTimerThreshold)
				{
					// Looked at it too much
					if (m_fIntermissionDuration != 0)
						SwitchState_Intermission(STATE_OBSERVING_MINOR);
					else if (sectorMinor != -1)
						SwitchState_ObserveMinorSector();
				}
				
				break;
			}
			
			case STATE_OBSERVING_MINOR:
			{
				m_fTimer += timeSlice_s;
				
				if (sectorMajor != m_iSectorMajor && sectorMajor != -1)
				{
					// Major sector changed, so something important has happened
					// Start looking at major threat sector again
					resetLook = true;
					SwitchState_ObserveMajorSector();
				}
				else if (sectorMinor == -1) // Minor sector is no longer relevant
				{
					if (sectorMajor != -1)
						SwitchState_ObserveMajorSector();
					else
						SwitchState_Idle();
				}					
				else if (m_fTimer > m_fTimerThreshold)
				{
					if (m_fIntermissionDuration != 0)
						SwitchState_Intermission(STATE_OBSERVING_MAJOR);
					else if (sectorMajor != -1)
						SwitchState_ObserveMajorSector();
					else
						SwitchState_Idle();
				}
				
				break;
			}
			
			case STATE_INTERMISSION:
			{
				m_fTimer += timeSlice_s;
				
				if (sectorMajor != m_iSectorMajor && sectorMajor != -1)
				{
					// Major sector changed, so something important has happened
					// Start looking at major threat sector again
					resetLook = true;
					SwitchState_ObserveMajorSector();
				}
				else if (m_fTimer > m_fTimerThreshold)
				{
					// After pause is over, look at the next sector, if it's valid
					if (m_iStateAfterIntermission == STATE_OBSERVING_MAJOR)
					{
						if (sectorMajor != -1)
							SwitchState_ObserveMajorSector();
						else
							SwitchState_Idle();
					}
					else
					{
						if (sectorMinor != -1)
							SwitchState_ObserveMinorSector();
						else if (sectorMajor != -1)
							SwitchState_ObserveMajorSector();
						else
							SwitchState_Idle();
					}
				}
				
				break;
			}
		}
		
		m_iSectorMajor = sectorMajor;
		m_iSectorMinor = sectorMinor;
		
		
		//-------------------------------------------------
		// Propagate data to outputs
		
		// Reset
		if (m_iState != prevState) // Also reset if state changed
			resetLook = true;
		if (resetLook)
			SetVariableOut(PORT_RESET_LOOK, resetLook);
		
		// Active
		bool active = m_iState == STATE_OBSERVING_MAJOR || m_iState == STATE_OBSERVING_MINOR;
		SetVariableOut(PORT_ACTIVE, active);
		
		// Position and Radius
		if (active)
		{
			int sectorId;
			if (m_iState == STATE_OBSERVING_MAJOR)
				sectorId = m_iSectorMajor;
			else
				sectorId = m_iSectorMinor;
			
			vector threatPos = m_ThreatFilter.GetSectorPos(sectorId);
			SetVariableOut(PORT_CURRENT_POSITION, threatPos);
		}
		
		// Optional: Evaluate if we should raise weapon, and use optics
		if (m_bEvaluateEquipmentUsage)
		{
			bool raiseWeapon = false;
			bool useOptics = false;
			if (m_iSectorMajor != -1)
			{
				float dangerValue = m_ThreatFilter.GetSectorDanger(m_iSectorMajor);
				raiseWeapon = dangerValue > DANGER_THRESHOLD_RAISE_WEAPON;
				float distanceSq = vector.DistanceSq(m_ThreatFilter.GetSectorPos(m_iSectorMajor), m_MyEntity.GetOrigin());
				useOptics = distanceSq > DISTANCE_USE_OPTICS_SQ;
			}
			
			// Raise weapon
			SetVariableOut(PORT_RAISE_WEAPON, raiseWeapon);
			SetVariableOut(PORT_USE_OPTICS, useOptics);
		}
		
		return ENodeResult.SUCCESS;
	}
	
	protected void SwitchState_ObserveMajorSector()
	{
		m_iState = STATE_OBSERVING_MAJOR;
		m_fTimer = 0;
		m_fTimerThreshold = m_fLookDurationMajor;
		m_fTimerThreshold = Math.RandomFloat(m_fTimerThreshold, m_fTimerRandomizationCoefficient*m_fTimerThreshold);
	}
	
	protected void SwitchState_ObserveMinorSector()
	{
		m_iState = STATE_OBSERVING_MINOR;
		m_fTimer = 0;
		m_fTimerThreshold = m_fLookDurationMinor;
		m_fTimerThreshold = Math.RandomFloat(m_fTimerThreshold, m_fTimerRandomizationCoefficient*m_fTimerThreshold);
	}
	
	protected void SwitchState_Idle()
	{
		m_iState = STATE_IDLE;
		m_fTimer = 0;
	}
	
	protected void SwitchState_Intermission(int stateAfterIntermission)
	{
		m_iState = STATE_INTERMISSION;
		m_iStateAfterIntermission = stateAfterIntermission;
		m_fTimer = 0;
		m_fTimerThreshold = m_fIntermissionDuration;
		m_fTimerThreshold = Math.RandomFloat(m_fTimerThreshold, m_fTimerRandomizationCoefficient*m_fTimerThreshold);
	}
	
	//---------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { PORT_ACTIVE, PORT_RESET_LOOK, PORT_CURRENT_POSITION, PORT_RAISE_WEAPON, PORT_USE_OPTICS };
	override TStringArray GetVariablesOut() { return s_aVarsOut; };
	
	override static bool VisibleInPalette() { return true; }
}