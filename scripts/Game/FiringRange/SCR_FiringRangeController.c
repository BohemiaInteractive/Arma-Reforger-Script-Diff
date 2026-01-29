[EntityEditorProps(category: "GameScripted/FiringRange", description: "Controller of firing line, handles behaviour of targets.", color: "0 0 255 255", dynamicBox: true)]


class SCR_FiringRangeControllerClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_FiringRangeController : GenericEntity
{
	const static int NO_TARGET_OWNER = -1;
	const static int RESET_COUNTER_TIME = 1500;
	const static int POWER_LIGHT_DELAY = 1000;
	const static int INDICATOR_MAX_VALUE = 9999;
	
	// Digits cone rotatioin value
	const static int DIGIT_ZERO = 0;
	const static int DIGIT_ONE = 315;
	const static int DIGIT_TWO = 280;
	const static int DIGIT_THREE = 245;
	const static int DIGIT_FOUR = 210;
	const static int DIGIT_FIVE = 175;
	const static int DIGIT_SIX = 140;
	const static int DIGIT_SEVEN = 105;
	const static int DIGIT_EIGHT = 70;
	const static int DIGIT_NINE = 35;
	
	const static string LIGHT_POWER = "light_001_power";
	const static string LIGHT_SEQUENCE = "light_002_sequence";
	const static string LIGHT_MALFUNCTION = "light_003_malfunction";
	
	const static int LIGHT_EMISSIVITY_ON = 1;
	const static int LIGHT_EMISSIVITY_OFF = 0;
	
	const static bool LIGHT_ON = true;
	const static bool LIGHT_OFF = false;
	
	private ParametricMaterialInstanceComponent m_LightPower;
	private ParametricMaterialInstanceComponent m_LightSequence;
	private ParametricMaterialInstanceComponent m_LightMalfunction;
		
	// Variable used for JIP synchronization. Because Power light don't change it's state and Malfunction light blink only for one sec, only this worht of synchronization.
	// Todo: Replace this with EmissiveMultiplier getter (ParametricMaterialInstanceComponent).
	private bool m_bRoundLightState = false;
	private int m_iDistanceIndicatorValue;
	private int m_iNumberOfTargetsIndicatorValue;
	
	// Time for how long warning light should be on when player leaves the area 
	const static int ERROR_LIGHT_DURATION = 1000;

	//! For how long target stays in erected position
	[Attribute("4", UIWidgets.Slider, "For how long target stays in erected position in seconds.", "1 100 1")] 
	protected int m_iErectTargetTime;
		
	//! Material used for target hit indicator (decal)
	[Attribute("{F2E83D562703F861}Assets/Decals/Impact/DecalHitIndicator.emat", UIWidgets.ResourceNamePicker, "Material used for target hit indicator", "emat")]
	private ResourceName m_PreviewDecal;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Power light entity", "et")]
	private ResourceName m_LightPowerRes;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Sequence light entity", "et")]
	private ResourceName m_LightSequenceRes;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Malfunction light entity", "et")]
	private ResourceName m_LightMalfunctionRes;
	
	[Attribute("1", UIWidgets.Slider, "Targets in one round", "1 20 10")]
	private int m_iTargetsInRound;
	
	[RplProp()]	
	private int m_iCountShots = 0;
	
	[RplProp()]	
	private int m_iTargetSetDistance;
		
	// owner of the firing line PlayerController (player)
	[RplProp()]	
	private int m_iFiringLineOwner = NO_TARGET_OWNER; 
	
	private SCR_FiringRangeTarget m_LastSelectedTarget;
	private SCR_FiringRangeTarget m_CurrentSelectedTarget;
	
	private int m_iElementInArray = 0;
	private int m_iValuePos;
	private int m_iHighestPossibleDistance;
		
	private static ref array<SCR_FiringRangeController> s_aInstances = {};
	private ref array<SCR_FiringRangeTarget> m_aAllTargetsArray = {};
	private ref array<int> m_aDistances = {};
	private ref array<Decal> m_aIndicators = {};
	
	// Arrays with saved coordinates and vectors of decals
	private ref array<vector> m_aIndicatorsCoords = {};
	private ref array<vector> m_aIndicatorsVector = {};
	
	// time for how long target stays erected
	private float m_fTargetErectedDuration;
	
	// target which serves as an indicators 
	private IEntity m_Indicator; 
	
	// components
	private RplComponent m_RplComponent;
	private SignalsManagerComponent m_SignalManager;
	
	private SCR_FiringRangeManager m_FiringRangeManager;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_SignalManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
				
		array<EntitySlotInfo> slots = {}; EntitySlotInfo.GetSlotInfos(owner, slots);
		IEntity slotEntity;
		string slotBone;
		
		foreach (EntitySlotInfo slot: slots)
		{
			if (!slot)
				continue;
			
			IEntity ent = slot.GetAttachedEntity();
			if (!ent)
				continue;
			
			slotBone = slot.GetBoneName();
			if (!slotBone.IsEmpty())
			{
				switch (slotBone)
				{
					case LIGHT_POWER: {m_LightPower = ParametricMaterialInstanceComponent.Cast(ent.FindComponent(ParametricMaterialInstanceComponent)); break;};
					case LIGHT_SEQUENCE: {m_LightSequence = ParametricMaterialInstanceComponent.Cast(ent.FindComponent(ParametricMaterialInstanceComponent)); break;};
					case LIGHT_MALFUNCTION: {m_LightMalfunction = ParametricMaterialInstanceComponent.Cast(ent.FindComponent(ParametricMaterialInstanceComponent)); break;};
				}
			}
		}

		// get an array of all distances of targets
		CollectAllDistances(owner);
		SetControllerLight(ControllerLightType.LIGHT_POWER, LIGHT_ON)

	}
	
	//------------------------------------------------------------------------------------------------
	//! Animation proces of the target
	void AnimateTargets(IEntity pOwnerEntity,int playerID)
	{
		#ifdef ENABLE_DIAG
		// safe check this method should be executed only on server 
		if (!CheckMasterOnlyMethod("AnimateTargets()"))
			return;
		#endif
		
		if (!pOwnerEntity)
			return;
		
		SCR_SoundManagerModule.CreateAndPlayAudioSource(pOwnerEntity, SCR_SoundEvent.SOUND_RANGECP_STARTBUTTON);
		
		// Gets all child targets in the fire line	
	   	IEntity child = pOwnerEntity.GetChildren();
		array<SCR_FiringRangeTarget> targetArray = new array<SCR_FiringRangeTarget>();
				
		// Set the owner of firing Line
		m_iFiringLineOwner = playerID;
		Replication.BumpMe();
		
		while (child)
		{
			SCR_FiringRangeTarget target = SCR_FiringRangeTarget.Cast(child);
			if (target)
			{
				if (!target.IsIndicator())
				{
					// array of all targets in line. Not just those which were selected.
					m_aAllTargetsArray.Insert(target);

					if ((target.GetSetDistance() == m_iTargetSetDistance) || m_iTargetSetDistance == -1)
					{
						targetArray.Insert(target);
					};
					
					// Reset All targets into folded position
					target.SetState(ETargetState.TARGET_DOWN);
					target.SetAutoResetTarget(false); // The BumpMe is called in SetState!
							
					// Set target owner and target mode
					target.SetTargetOwner(playerID);
					target.SetTargetMode(ETargetMode.COMPETITION);
				}
			}
	
			child = child.GetSibling();
		};
		
		// Sound of starting round
		SCR_SoundManagerModule.CreateAndPlayAudioSource(pOwnerEntity, SCR_SoundEvent.SOUND_RANGECP_ROUNDSTART);
		
		GetGame().GetCallqueue().CallLater(ResetCountPopUpTargets, RESET_COUNTER_TIME, false); 
		GetGame().GetCallqueue().CallLater(ErectRandomTarget, m_iErectTargetTime * 1000, true, targetArray, m_aAllTargetsArray); 
		
		if (m_FiringRangeManager)
			m_FiringRangeManager.ControllerLight(this, ControllerLightType.LIGHT_SEQUENCE, LIGHT_ON);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Erect randomly selected target
	void ErectRandomTarget(array<SCR_FiringRangeTarget> targetArray)
	{
		// if the round finished, reset targets back to default behaviour
		if (m_iCountShots >= m_iTargetsInRound)
			{				
				BackToDefaultTarget();
				return;
			}
		
		m_LastSelectedTarget = m_CurrentSelectedTarget;
		while (m_LastSelectedTarget == m_CurrentSelectedTarget && targetArray.Count() > 1)
		{
			m_CurrentSelectedTarget = targetArray.GetRandomElement();
		};
				
		if (m_CurrentSelectedTarget)
		{
			m_fTargetErectedDuration = m_CurrentSelectedTarget.GetErectedDuration();
			m_CurrentSelectedTarget.SetState(ETargetState.TARGET_UP);
			GetGame().GetCallqueue().CallLater(FoldTarget, m_fTargetErectedDuration, false, m_CurrentSelectedTarget); 
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fold erected target back to default
	void FoldTarget(SCR_FiringRangeTarget selectedTarget)
	{
		if (m_fTargetErectedDuration == 0)
			return;
		
		if (selectedTarget)
		{
			float TargetState = selectedTarget.GetState();
			if (TargetState == 0)					
				selectedTarget.SetState(ETargetState.TARGET_DOWN);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reset targets back to default behaviour, stop targets pops up loop
	void BackToDefaultTarget()
	{
		#ifdef ENABLE_DIAG
		// safe check this method should be executed only on server 
		if (!CheckMasterOnlyMethod("BackToDefaultTarget()"))
			return;
		#endif
		
		// Reset the owner of firing Line
		m_iFiringLineOwner = NO_TARGET_OWNER;
		Replication.BumpMe();

		GetGame().GetCallqueue().Remove(ErectRandomTarget);
		
		foreach (SCR_FiringRangeTarget target : m_aAllTargetsArray)
		{
			if (!target)
				continue;

			target.SetState(ETargetState.TARGET_UP);
			target.SetAutoResetTarget(true); // The BumpMe is called in SetState!
			target.SetTargetMode(ETargetMode.TRAINING);
			target.SetTargetOwner(NO_TARGET_OWNER);
		}

		m_aAllTargetsArray.Clear();
		if (m_FiringRangeManager)
			m_FiringRangeManager.ControllerLight(this, ControllerLightType.LIGHT_SEQUENCE, LIGHT_OFF);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIndicator(IEntity indicator)
	{
		m_Indicator = indicator;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reset targets back to default behaviour, called from controller area trigger
	void BackToDefaultTargetsFromLineArea()
	{
		// set the time for how long the target should be in erected position to zero.
		m_fTargetErectedDuration = 0;
				
		// Find local player controller
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
				
		// Find firing range network entity to send RPC to server
		SCR_FiringRangeNetworkEntity firingRangeNetworkEntity = SCR_FiringRangeNetworkEntity.GetInstance();
		if (!firingRangeNetworkEntity)
			return;
		
		firingRangeNetworkEntity.BackToDefaultTarget(this);
		if (m_FiringRangeManager)
		{
			m_FiringRangeManager.ControllerLight(this, ControllerLightType.LIGHT_MALFUNCTION, LIGHT_ON);
			m_FiringRangeManager.ControllerLight(this, ControllerLightType.LIGHT_SEQUENCE, LIGHT_OFF);
			GetGame().GetCallqueue().CallLater(m_FiringRangeManager.ControllerLight, ERROR_LIGHT_DURATION, false, this, ControllerLightType.LIGHT_MALFUNCTION, LIGHT_OFF);
		}
		
		// Sound of presed button
		SCR_SoundManagerModule.CreateAndPlayAudioSource(this, SCR_SoundEvent.SOUND_RANGECP_ROUNDABORT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Make an array of all possible distances of targets under assigned firing range controller
	void CollectAllDistances(IEntity owner)
	{		
		IEntity child = owner.GetChildren();
		while (child)
		{
			SCR_FiringRangeTarget target = SCR_FiringRangeTarget.Cast(child);
			if (target && !target.IsIndicator())
			{
				int distance = target.GetSetDistance();
				if (m_aDistances.Find(distance) == -1)
					m_aDistances.Insert(distance);
			}
			
			child = child.GetSibling();
		};
		
		if (m_aDistances.IsEmpty())
			return;
		
		m_aDistances.Sort();
		m_iTargetSetDistance = m_aDistances[0];
		if (m_SignalManager)
		{
			SetControllerCounter(EControlerSection.DISTANCE, GetTargetDistance());
			SetControllerCounter(EControlerSection.NUMBER_OF_TARGETS ,GetTargetsInRound());
		}
			
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter of all possible distances
	array<int> GetAllDistances()
	{
		return m_aDistances;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Rerurns ID of player who owns the firing line
	int GetFiringLineOwnerId()
	{
		return m_iFiringLineOwner;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLowestDistanceSet()
	{
		return GetTargetDistance() == m_aDistances[0];
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsHighestDistanceSet()
	{
		return GetTargetDistance() == m_aDistances[m_aDistances.Count()-1];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter of next upper distance
	int CalculateTargetDistance(bool increase)
	{
		#ifdef ENABLE_DIAG
		// safe check this method should be executed only on server 
		if (!CheckMasterOnlyMethod("CalculateTargetDistance()"))
			return -1;
		#endif
		
		// get the current set distance
		int currentDistance = m_iTargetSetDistance;
		
		// if the last state was last, lets go back to the beginning of the row
		if (m_iTargetSetDistance == -1)
		{
			m_iTargetSetDistance = 0;
			Replication.BumpMe();
		}
		
		// check if it's highest possible distance in current row. If so, select all targets (-1) or go to the next element in row
		m_iHighestPossibleDistance = m_aDistances.Count()-1;
		
		m_iValuePos = m_aDistances.Find(m_iTargetSetDistance);
		if (increase) 
		{
			m_iValuePos++;
		}
		else
		{
			m_iValuePos--;
		}
		m_iTargetSetDistance = m_aDistances[m_iValuePos];
		Replication.BumpMe();

		
		// Entity has a signal manager, try to set up the value on counter
		if (m_SignalManager && m_FiringRangeManager)
			m_FiringRangeManager.SetControllerCounter(this, EControlerSection.DISTANCE, m_iTargetSetDistance);
		
		// Sound of presed button
		SCR_SoundManagerModule.CreateAndPlayAudioSource(this, SCR_SoundEvent.SOUND_RANGECP_CHANGEDISTANCE);

		return m_iTargetSetDistance;	
	}
	
	//------------------------------------------------------------------------------------------------
	// Increase or decrease a number of targets in one round.
	void UpdateNumberOfTargets(bool increase)
	{
		if (increase)	
		{
			m_iTargetsInRound++;
		}
		else 
		{
			m_iTargetsInRound--;
		}
			
		// Entity has a signal manager, try to set up the value on counter
		if (m_SignalManager)
			m_FiringRangeManager.SetControllerCounter(this, EControlerSection.NUMBER_OF_TARGETS, m_iTargetsInRound);
		
		// Sound of presed button
		SCR_SoundManagerModule.CreateAndPlayAudioSource(this, SCR_SoundEvent.SOUND_RANGECP_CHANGETARGET);			
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the selected distance on controller display
	void SetControllerCounter(EControlerSection indicator, int value)
	{		
		// Prepare signals
		int digitOneIndex;
		int digitTenIndex;
		int digitHundredIndex;
		int digitThousandIndex;
		
		switch (indicator)
		{
			case EControlerSection.DISTANCE : 
			{
				
				digitOneIndex = m_SignalManager.FindSignal("DistanceOne");
				digitTenIndex = m_SignalManager.FindSignal("DistanceTen");
				digitHundredIndex = m_SignalManager.FindSignal("DistanceHundred");
				digitThousandIndex = m_SignalManager.FindSignal("DistanceThousand");
				m_iDistanceIndicatorValue = value; 
				break;
			}
			case EControlerSection.NUMBER_OF_TARGETS : 
			{
				digitOneIndex = m_SignalManager.FindSignal("RoundOne");
				digitTenIndex = m_SignalManager.FindSignal("RoundTen");
				digitHundredIndex = m_SignalManager.FindSignal("RoundHundred");
				digitThousandIndex = m_SignalManager.FindSignal("RoundThousand");
				m_iNumberOfTargetsIndicatorValue = value;
				break;
			}
		}
		
		// Calculate value
		array<int> digits = {0,0,0,0};
		int i = 0;
		while (value != 0)
        {
            int m = value % 10;
			digits.Set(i,m);
			i++;
            value = value / 10;
        }

		// Set signals
		for (int y = digits.Count() - 1; y >= 0; y--)
		{
			float signalVal = GetSignalValue(digits[y]);
			switch (y)
			{
				case EDigit.DIGIT_ONE: {m_SignalManager.SetSignalValue(digitOneIndex,signalVal); break;};
				case EDigit.DIGIT_TEN: {m_SignalManager.SetSignalValue(digitTenIndex,signalVal); break;};
				case EDigit.DIGIT_HUNDRED: {m_SignalManager.SetSignalValue(digitHundredIndex,signalVal); break;};
				case EDigit.DIGIT_THOUSAND: {m_SignalManager.SetSignalValue(digitThousandIndex,signalVal); break;};
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns signal value for a given number
	float GetSignalValue(int num)
	{
		switch (num)
		{
			case 0: {return DIGIT_ZERO; break;};
			case 1: {return DIGIT_ONE; break;};
			case 2: {return DIGIT_TWO; break;};
			case 3: {return DIGIT_THREE; break;};
			case 4: {return DIGIT_FOUR; break;};
			case 5: {return DIGIT_FIVE; break;};
			case 6: {return DIGIT_SIX; break;};
			case 7: {return DIGIT_SEVEN; break;};
			case 8: {return DIGIT_EIGHT; break;};
			case 9: {return DIGIT_NINE; break;};
		}
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Count how many targets was shot
	void CountPopUpTargets()
	{
		m_iCountShots++;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Reset the counter
	void ResetCountPopUpTargets()
	{
		m_iCountShots = 0;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! register indicator (decal) into the array and draw the hit
	void AddIndicator(vector localCoordOfHit, vector localVectorOfHit)
	{
		// Check if the indicator exist 
		if (!m_Indicator)
			return;
		
		World world = GetWorld();
		Decal decal = world.CreateDecal(
				m_Indicator, // Entity 
				m_Indicator.CoordToParent(localCoordOfHit), // origin vector (position) 
				m_Indicator.VectorToParent(localVectorOfHit), // project vector 
				0.0, // nearclip
				0.2, // farclip
				0, // angle 
				0.1, // size 
				1, // stretch 
				m_PreviewDecal, //emat path
				-1,// lifetime, if <= 0 the decal is created as static
				0xFFFFFFFF); //color of decal
		
		if (!decal)
			return;

		
		m_aIndicators.Insert(decal);
		Replication.BumpMe();
		
		m_aIndicatorsCoords.Insert(localCoordOfHit);
		m_aIndicatorsVector.Insert(localVectorOfHit);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return indicator entity
	IEntity GetIndicator()
	{
		return m_Indicator;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove all declas from indicator
	void RemoveIndicators()
	{
		World world = GetWorld();
		while (m_aIndicators.Count() > 0)
		{
			Decal decal = m_aIndicators.Get(0);
			if (decal)
			{
				world.RemoveDecal(decal);
				m_aIndicators.Remove(0);
			}
		}
		
		m_aIndicatorsVector.Clear();
		m_aIndicatorsCoords.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTargetDistance()
	{
		return m_iTargetSetDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTargetsInRound()
	{
		return m_iTargetsInRound;
	}
	//------------------------------------------------------------------------------------------------
	//! Count and set the max score in the round on a specific line
	float GetMaxScoreInRound()
	{
		return m_iTargetsInRound * SCR_FiringRangeTarget.SCORE_CENTER;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CheckMasterOnlyMethod(string methodName)
	{
		if (IsProxy())
		{
			Print("Master-only method (SCR_FiringRangeController." + methodName + ") called on proxy. Some functionality might be broekn!", LogLevel.WARNING);
			return false;
		}
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetControllerLight(ControllerLightType light, bool mode)
	{
		switch (light)
		{
			case ControllerLightType.LIGHT_POWER: {SetEmissivity(m_LightPower, mode); break;};
			case ControllerLightType.LIGHT_SEQUENCE: {SetEmissivity(m_LightSequence, mode); m_bRoundLightState = mode; break;};
			case ControllerLightType.LIGHT_MALFUNCTION: {SetEmissivity(m_LightMalfunction, mode); break;};
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEmissivity(ParametricMaterialInstanceComponent light, bool on)
	{
		if (!light)
			return;
		
		if (on)
		{
			light.SetEmissiveMultiplier(LIGHT_EMISSIVITY_ON);
		}
		else
		{
			light.SetEmissiveMultiplier(LIGHT_EMISSIVITY_OFF);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Return if is proxy or not
	protected bool IsProxy()
	{	
		if (!m_RplComponent)
			m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		// Counters on the controller
		writer.WriteIntRange(m_iDistanceIndicatorValue, 0, INDICATOR_MAX_VALUE);
		writer.WriteIntRange(m_iNumberOfTargetsIndicatorValue, 0, INDICATOR_MAX_VALUE);
		
		// Sequence light indicator status
		writer.WriteBool(m_bRoundLightState);
		
		// Decals on hit indicator
		int count = m_aIndicatorsCoords.Count();
		writer.WriteInt(count);
		
		for (int i = 0; i < count; i++)
		{
			writer.WriteVector(m_aIndicatorsCoords[i]);
			writer.WriteVector(m_aIndicatorsVector[i]);
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		// Counters on the controller
		reader.ReadIntRange(m_iDistanceIndicatorValue, 0, INDICATOR_MAX_VALUE);
		reader.ReadIntRange(m_iNumberOfTargetsIndicatorValue, 0, INDICATOR_MAX_VALUE);
		
		// Sequence light indicator status
		reader.ReadBool(m_bRoundLightState);
		
		// Hit indicator
		int count;
		reader.ReadInt(count);
		
		for (int i = 0; i < count; i++)
		{
			vector position;
			vector rotation;
			reader.ReadVector(position);
			reader.ReadVector(rotation);

			AddIndicator(position, rotation);
		}	
		
		SetControllerLight(ControllerLightType.LIGHT_SEQUENCE, m_bRoundLightState);
		SetControllerCounter(EControlerSection.DISTANCE ,m_iDistanceIndicatorValue);
		SetControllerCounter(EControlerSection.NUMBER_OF_TARGETS ,m_iNumberOfTargetsIndicatorValue);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Constructor
	void SCR_FiringRangeController(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		
		if (s_aInstances)
			s_aInstances.Insert(this);

		m_FiringRangeManager = SCR_FiringRangeManager.Cast(parent);
		
		if (m_FiringRangeManager)
			m_FiringRangeManager.RegisterFiringRangeController(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_FiringRangeController()
	{
		if (s_aInstances)
			s_aInstances.RemoveItem(this);
	}
};

enum EControlerSection
{
	DISTANCE,
	NUMBER_OF_TARGETS
};

enum EDigit 
{
	DIGIT_ONE,
	DIGIT_TEN,
	DIGIT_HUNDRED,
	DIGIT_THOUSAND
};

enum ControllerLightType 
{
	LIGHT_POWER,
	LIGHT_SEQUENCE,
	LIGHT_MALFUNCTION
};
