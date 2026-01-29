[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations")]
class SCR_AICombatComponentClass : ScriptComponentClass
{
}

enum EAISkill
{
	NONE,
	NOOB 	= 10,
	ROOKIE	= 20,
	REGULAR	= 50,
	VETERAN	= 70,
	EXPERT 	= 80,
	CYLON  	= 100
}

class SCR_AICombatComponent : ScriptComponent
{
	// Constants
	static const int			 TARGET_ENDANGERED_TIMEOUT_S = 10;				//!< How long after last time target was endangering we stop treating it as such
	static const float			 ENDANGERING_TARGET_SCORE_MULTIPLIER = 1.05;	//!< Multiplier of target score if target is considered endangering
	
	// Score increments for assigned targets and endangering targets
	protected static const float ASSIGNED_TARGETS_SCORE_INCREMENT = 0.5;
	protected static const float ENDANGERING_TARGETS_SCORE_INCREMENT = 15.0;
	
	// Constants related to weapon&target selector
	static const float TARGET_MAX_LAST_SEEN_VISIBLE = 0.5;		//!< 'IsTargetVisible' variable in attack tree relies on this
	protected static const float TARGET_MAX_LAST_SEEN_DIRECT_ATTACK = 1.6;
	protected static const float TARGET_MAX_LAST_SEEN_DIRECT_ATTACK_CLOSE = 4.5;
			  static const float TARGET_MIN_LAST_SEEN_INDIRECT_ATTACK = 2.0;
	          static const float TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK = 7.0;
	          static const float TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK_MG = 14.0;
	          static const float TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK_CLOSE = 20.0;

	          static const float TARGET_MAX_LAST_SEEN = 11.0;
	protected static const float TARGET_MIN_INDIRECT_TRACE_FRACTION_MIN = 0.5;	//!< Min value of TraceFraction for indirect attacks. \see BaseTarget.GetTraceFraction.
	protected static const float TARGET_MAX_DISTANCE_INFANTRY = 9999.0; 		//!< This value is old and is now meaningless with the way group logic works.
	protected static const float TARGET_MAX_DISTANCE_VEHICLE = 700.0;		//!<
	protected static const float TARGET_MAX_DISTANCE_DISARMED = 0.0;		//!< Max distance at which disarmed targets are considered for attack. Now it's 0, so they never shoot disarmed targets
	protected static const float TARGET_MAX_TIME_SINCE_ENDANGERED = 5.0; 	//!< Max time since we were endangered to consider the enemy target endangering
	protected static const float TARGET_SCORE_RETREAT = 75.0;				//!< Threshold for target score for retreating from that target
	static const float TARGET_SCORE_HIGH_PRIORITY_ATTACK = 98.5;			//!< Threshold for high priority attack
	
	protected static ref array<EWeaponType> s_aWeaponBlacklistFragGrenades = {EWeaponType.WT_FRAGGRENADE};
	
	// Perception factors
	protected const float PERCEPTION_FACTOR_SAFE = 1.0;			//!< We are safe and are good at recognising enemies
	protected const float PERCEPTION_FACTOR_VIGILANT = 2.5;		//!< When vigilant and alert we are very good at recognising enemies
	protected const float PERCEPTION_FACTOR_ALERTED = 2.5;
	protected const float PERCEPTION_FACTOR_THREATENED = 0.4;	// We are suppressed and are bad at recognizing enemies
	
	protected const float PERCEPTION_FACTOR_EQUIPMENT_BINOCULARS = 3.0;	//!< Looking through binoculars
	protected const float PERCEPTION_FACTOR_EQUIPMENT_NONE = 1.0;		//!< Not using any special equipment, same recognition ability as usual
	
	//! Within this distance AI considers combat as 'close range', used in firing times
	static const float CLOSE_RANGE_COMBAT_DISTANCE = 35.0;
	static const float CLOSE_RANGE_COMBAT_DISTANCE_SQ = CLOSE_RANGE_COMBAT_DISTANCE * CLOSE_RANGE_COMBAT_DISTANCE;
	
	//! Beyond this distance AI considers combat as 'long range', used for danger events and firing times
	static const float LONG_RANGE_COMBAT_DISTANCE = 280.0;
	
	protected const float FRAG_GRENADE_MAX_THREAT = 0.6; // Max threat measure soldier is allowed to have to be able to use frag grenades
	
	protected SCR_ChimeraAIAgent m_Agent;
	protected SCR_CharacterControllerComponent		m_CharacterController;
	protected SCR_InventoryStorageManagerComponent	m_InventoryManager;
	protected BaseWeaponManagerComponent			m_WpnManager;
	protected SCR_CompartmentAccessComponent		m_CompartmentAccess;
	protected SCR_ExtendedDamageManagerComponent	m_DamageManager;
	protected PerceptionComponent 					m_Perception;
	protected SCR_AIInfoComponent					m_AIInfo;
	protected SCR_AIUtilityComponent				m_Utility;
	
	// Cached data about current vehicle and compartment
	protected IEntity m_CurrentVehicle;								//!< Current vehicle. It's not always same as compartment owner!
	protected bool m_bCurrentVehicleEvac;							//!< True if we're exiting vehicle because of damage/fire
	protected SCR_BaseCompartmentManagerComponent m_CurrentVehicleCompartmentManager;	//!< Vehicle's compartment manager
	protected BaseCompartmentSlot m_CurrentCompartmentSlot;			//!< Current compartment slot
	protected TurretControllerComponent m_CurrentTurretController;	//!< Current turret controller (if we are in a turret)

	[Attribute("50", UIWidgets.ComboBox, "AI skill in combat", "", ParamEnumArray.FromEnum(EAISkill) )]
	protected EAISkill m_eAISkillDefault;

	protected EAISkill m_eAISkill;
	
	// Belongs to friendly in aim check
	protected bool m_bFriendlyAimLastResult;
	#ifdef WORKBENCH
	protected ref Shape m_FriendlyAimShape;
	#endif
	
	// Weapon and target selection
	ref AIWeaponTargetSelector m_WeaponTargetSelector = new AIWeaponTargetSelector();
	private ref BaseTarget	m_SelectedTarget;
	
	protected bool m_SelectedTargetVisible;
	protected vector m_SelectedTargetDestinationPos;
	
	protected ref BaseTarget m_SelectedRetreatTarget;				//!< Target we should retreat from
	protected ref array<IEntity> m_aAssignedTargets = {};			//!< Array with assigned targets. Their score is increased.
	protected SCR_AITargetClusterState m_TargetClusterState;		//!< Assigned target cluster from our group
	protected EAIUnitType m_eExpectedEnemyType = EAIUnitType.UnitType_Infantry;	//!< Enemy type we expect to fight
	protected ResourceName m_SelectedWeaponResource;				//!< selected weapon handle tree read from config

	// Weapon selection against a specific target and its properties
	protected BaseWeaponComponent m_SelectedWeaponComp;				//!< Weapon, muzzle and magazine which we want to equip.
	protected BaseMagazineComponent m_SelectedMagazineComp;			//!< Character might currently have a different one though.
	protected int m_iSelectedMuzzle;
	protected float m_fSelectedWeaponMinDist;
	protected float m_fSelectedWeaponMaxDist;
	protected bool m_bSelectedWeaponDirectDamage;
	protected EAIUnitType m_eUnitTypesCanAttack;
	protected BaseCompartmentSlot m_WeaponEvaluationCompartment;	//!< Compartment at previous weapon evaluation
		
	// Current soldier fire rate
	protected float m_fFireRateCoef = 1;
	
	// Is current soldier fire rate a result of direct/personal order. This is intented to be used 
	// when you want to set specific fire rate to specific soldier and not be changed by group, unless overriden
	protected bool m_bIsFireRatePersistent = false;	
	
	// Weapon selection updates
	protected float m_fNextWeaponTargetEvaluation_ms = 0;
	protected const float WEAPON_TARGET_UPDATE_PERIOD_MS = 500;
	
	protected SCR_AIConfigComponent m_ConfigComponent;
	
	// Turret dismounting
	protected float m_fDismountTurretTimer;
	protected static const float DISMOUNT_TURRET_TARGET_LAST_SEEN_MAX_S = 100;
	protected static const float TURRET_TARGET_EXCESS_ANGLE_THRESHOLD_DEG = 3.0;	//!< We will dismount turret when target's angle exceeds turret limits by this value
	protected const float DISMOUNT_TURRET_TIMER_MS = 1200;							//!< How much time must pass until we decide to dismount a turret if enemy is out of turret limits.
	protected static ref array<EVehicleType> s_aForbidDismountTurretsOfVehicleTypes = { EVehicleType.APC }; //!< Array of vehicle types we should not dismount when turret cannot shoot the target
	
	// Perception
	protected float m_fEquipmentPerceptionFactor = 1.0;	//!< How good we can spot targets based on our current equipment. Updated by RecalculateEquipmentPerceptionFactor().
	protected float m_fPerceptionFactor = 1.0;			//!< Perception factor which is set by SetPerceptionFactor(). It acts as multiplier.
	
	protected EGadgetType m_eCurrentGadgetType = -1;	//!< Gadget state. Used for detection when we use binoculars.
	protected bool m_bGadgetFocus = false;				//!< True when we are in gadget focus mode (like looking through binoculars)
		
	//------------------------------------------------------------------------------------------------
	void SetGroupSuppressClusterState(SCR_AITargetClusterState state)
	{
		SCR_AISuppressGroupClusterBehavior behavior = SCR_AISuppressGroupClusterBehavior.Cast(m_Utility.FindActionOfType(SCR_AISuppressGroupClusterBehavior));
		
		if (behavior)
		{
			// Same cluster as currently suppressed, no need to do anything
			if (behavior.GetClusterState() == state)
				return;
			
			// Cluster changed, fail current behavior
			behavior.Fail();
		}
		
		// New cluster is not null, create new behavior
		if (state)
		{
			behavior = new SCR_AISuppressGroupClusterBehavior(m_Utility, null, null, 0, 1, SCR_AIActionBase.PRIORITY_LEVEL_NORMAL, state);
			m_Utility.AddAction(behavior);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Sets group's fire rate on agent
	void SetGroupFireRateCoef(float coef = 1, bool overridePersistent = false)
	{
		// Exit if not override and current rate is persistent
		if (m_bIsFireRatePersistent && !overridePersistent)
			return;
		
		// Group order is never persistent
		SetFireRateCoef(coef, false);
	}
	
	//------------------------------------------------------------------------------------------------
	// Sets agent fire rate
	void SetFireRateCoef(float coef = 1, bool persistent = false)
	{
		m_fFireRateCoef = coef;
		m_bIsFireRatePersistent = persistent;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get current agent fire rate coef
	float GetFireRateCoef()
	{
		return m_fFireRateCoef;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EAISkill GetAISkill()
	{
		return m_eAISkill;
	}
	
	//------------------------------------------------------------------------------------------------
	EAISkill GetAISkillDefault()
	{
		return m_eAISkillDefault;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Use this to change AIskill dynamically
	//! \param[in] skill
	void SetAISkill(EAISkill skill)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetAISkill: %1", typename.EnumToString(EAISkill, skill)));
		#endif
		
		m_eAISkill = skill;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets perception ability. Affects speed at which perception detects targets.
	//! Bigger value means proportionally faster detection.
	//! Provided value must be non-negative
	//! !!! Now it only affects visual detection.
	//! \param[in] value
	void SetPerceptionFactor(float value)
	{
		if (value < 0)
			value = 0;
		m_fPerceptionFactor = value;
		UpdatePerceptionFactor(m_Perception, m_Utility.m_ThreatSystem);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPerceptionFactor()
	{
		return m_fPerceptionFactor;
	}
	
	//------------------------------------------------------------------------------------------------
	//! use this to reset AI skill to default
	void ResetAISkill()
	{
		#ifdef AI_DEBUG
		AddDebugMessage("ResetAISkill");
		#endif
		
		m_eAISkill = m_eAISkillDefault;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseWeaponComponent GetCurrentWeapon()
	{
		BaseWeaponManagerComponent wpnManger = GetCurrentWeaponManager();
		if (wpnManger)
			return wpnManger.GetCurrentWeapon();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseWeaponManagerComponent GetCurrentWeaponManager()
	{
		if (m_CurrentTurretController)
			return m_CurrentTurretController.GetWeaponManager();
		
		return m_WpnManager;
	}
		
	//------------------------------------------------------------------------------------------------
	//! \return
	EWeaponType GetCurrentWeaponType(bool overrideWithMuzzle = false)
	{
		BaseWeaponComponent currentWeapon = GetCurrentWeapon();
		if (currentWeapon)
			return SCR_AIWeaponHandling.GetWeaponType(currentWeapon, overrideWithMuzzle);
		
		return EWeaponType.WT_NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	BaseTarget GetCurrentTarget()
	{
		return m_SelectedTarget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	BaseTarget GetRetreatTarget()
	{
		return m_SelectedRetreatTarget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	BaseTarget GetLastSeenEnemy()
	{
		return m_Perception.GetLastSeenTarget(ETargetCategory.ENEMY, float.MAX);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	// Finds out if some of my known enemies is aimng at me. 
	BaseTarget GetEndangeringEnemy()
	{
		array<BaseTarget> targets = {};
		m_Perception.GetTargetsList(targets, ETargetCategory.ENEMY);
		
		foreach (BaseTarget tgt : targets)
		{
			if (tgt.IsEndangering())
				return tgt;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] ent
	//! \return
	bool IsEnemyKnown(IEntity ent)
	{
		bool knownAsEnemy = m_Perception.GetTargetPerceptionObject(ent, ETargetCategory.ENEMY);
		bool knownAsDetected = m_Perception.GetTargetPerceptionObject(ent, ETargetCategory.DETECTED);
		return knownAsEnemy || knownAsDetected;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] outWeaponEvent when true, selected weapon, muzzle or magazine has changed
	//! \param[out] outSelectedTargetChanged when true, selected target has changed
	//! \param[out] outPrevTarget
	//! \param[out] outCurrentTarget
	//! \param[out] outRetreatTargetChanged when true, the most dangerous target we should retreat from has changed since last evaluation
	//! \param[out] outCompartmentChanged when true, compartment has changed since last evaluation
	void EvaluateWeaponAndTarget(out bool outWeaponEvent, out bool outSelectedTargetChanged,
		out BaseTarget outPrevTarget, out BaseTarget outCurrentTarget,
		out bool outRetreatTargetChanged, out bool outCompartmentChanged)
	{
		float worldTime = GetGame().GetWorld().GetWorldTime();
		if (worldTime < m_fNextWeaponTargetEvaluation_ms)
		{
			outWeaponEvent = false;
			outSelectedTargetChanged = false;
			return;
		}
		
		m_fNextWeaponTargetEvaluation_ms = worldTime + WEAPON_TARGET_UPDATE_PERIOD_MS;
		
		SCR_ChimeraAIAgent myAgent = GetAiAgent();
		float agentThreat = m_Utility.m_ThreatSystem.GetThreatMeasure();

		AIGroup myGroup = myAgent.GetParentGroup();
		SCR_AIGroupInfoComponent groupInfoComp;
		if (myGroup)
			groupInfoComp = SCR_AIGroupInfoComponent.Cast(myGroup.FindComponent(SCR_AIGroupInfoComponent));
		
		BaseTarget newTarget = null;
		bool weaponEvent = false;
		bool selectedTargetChanged = false;
		bool retreatTargetChanged = false;
		bool compartmentChanged = false;
		
		// Resolve if we want to think of throwing grenade
		// Grenade throw is synchronized via group
		array<EWeaponType> weaponBlacklist;
		if (groupInfoComp)
		{
			if (agentThreat > FRAG_GRENADE_MAX_THREAT || !groupInfoComp.IsGrenadeThrowAllowed(myAgent))
				weaponBlacklist = s_aWeaponBlacklistFragGrenades;
		}
		
		bool useCompartmentWeapons = m_AIInfo.HasUnitState(EUnitState.IN_TURRET); // True when we are in a turret
		
		// Which assigned targets array to use?
		array<IEntity> assignedTargets;
		if (m_TargetClusterState && m_TargetClusterState.m_Cluster && m_TargetClusterState.m_Cluster.m_aEntities)
			assignedTargets = m_TargetClusterState.m_Cluster.m_aEntities;
		else
			assignedTargets = m_aAssignedTargets;
		
		bool selectedWpnTarget = m_WeaponTargetSelector.SelectWeaponAndTarget(assignedTargets,
			ASSIGNED_TARGETS_SCORE_INCREMENT, ENDANGERING_TARGETS_SCORE_INCREMENT,
			useCompartmentWeapons, weaponTypesBlacklist: weaponBlacklist);
		
		m_eUnitTypesCanAttack = m_WeaponTargetSelector.GetUnitTypesCanAttack();
		if (selectedWpnTarget)
		{
			BaseWeaponComponent newWeaponComp;
			BaseMagazineComponent newMagazineComp;
			int newMuzzleId;
			
			newTarget = m_WeaponTargetSelector.GetSelectedTarget();
			m_WeaponTargetSelector.GetSelectedWeapon(newWeaponComp, newMuzzleId, newMagazineComp);
			m_WeaponTargetSelector.GetSelectedWeaponProperties(m_fSelectedWeaponMinDist, m_fSelectedWeaponMaxDist, m_bSelectedWeaponDirectDamage);
			
			
			weaponEvent = newWeaponComp != m_SelectedWeaponComp ||
							newMuzzleId != m_iSelectedMuzzle ||
							newMagazineComp != m_SelectedMagazineComp;
			
			bool weaponOrMuzzleChanged = newWeaponComp != m_SelectedWeaponComp ||
									newMuzzleId != m_iSelectedMuzzle;
			
			if (weaponOrMuzzleChanged)
			{
				ref array<BaseMuzzleComponent> muzzles = {};
				newWeaponComp.GetMuzzlesList(muzzles);
				if (newMuzzleId >= muzzles.Count() || newMuzzleId < 0)
					m_SelectedWeaponResource = m_ConfigComponent.GetTreeNameForWeaponType(newWeaponComp.GetWeaponType(),0);	
				else 
					m_SelectedWeaponResource = m_ConfigComponent.GetTreeNameForWeaponType(newWeaponComp.GetWeaponType(),muzzles[newMuzzleId].GetMuzzleType());
				
				if (newWeaponComp)
				{
					EWeaponType weaponType = newWeaponComp.GetWeaponType();
					if (groupInfoComp && weaponType == EWeaponType.WT_FRAGGRENADE)
					{
						// We want to throw a grenade
						// Notify group immediately
						groupInfoComp.OnAgentSelectedGrenade(myAgent);
					}
				}
			}
			
			m_SelectedWeaponComp = newWeaponComp;
			m_iSelectedMuzzle = newMuzzleId;
			m_SelectedMagazineComp = newMagazineComp;
		}
		
		BaseTarget prevTarget = m_SelectedTarget;
		if (newTarget != m_SelectedTarget)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("Target has changed. New: %1, Previous: %2", newTarget, m_SelectedTarget));
			#endif
			m_SelectedTarget = newTarget;
			selectedTargetChanged = true;
		}
		
		// Check if we must retreat from some target
		BaseTarget targetCantAttack;
		float targetCantAttackScore;
		m_WeaponTargetSelector.GetMostRelevantTargetCantAttack(targetCantAttack, targetCantAttackScore);
		if (targetCantAttackScore < TARGET_SCORE_RETREAT)
			targetCantAttack = null;
		if (targetCantAttack != m_SelectedRetreatTarget)
		{
			m_SelectedRetreatTarget = targetCantAttack;
			retreatTargetChanged = true;
		}
		
		// Check if compartment has changed
		BaseCompartmentSlot currentCompartment = m_CompartmentAccess.GetCompartment();
		if (currentCompartment != m_WeaponEvaluationCompartment)
		{
			compartmentChanged = true;
			m_WeaponEvaluationCompartment = currentCompartment;
		}
		
		// Reset last velocity if target changed
		if (selectedTargetChanged)
		{
			m_SelectedTargetVisible = false;
			m_SelectedTargetDestinationPos = vector.Zero;
		}
			
		if (newTarget)
		{
			bool visible = IsTargetVisible(newTarget);
			IEntity targetEntity = newTarget.GetTargetEntity();
			
			if (visible != m_SelectedTargetVisible)
			{
				m_SelectedTargetVisible = visible;
				
				// Save position (destination) of target pos at the time we figured we've lost LOS
				// Note: this solution is dependent on update rate of EvaluateWeaponAndTarget
				if (!visible && targetEntity)
					m_SelectedTargetDestinationPos = targetEntity.GetOrigin();
			}
		}
				
		outWeaponEvent = weaponEvent;
		outSelectedTargetChanged = selectedTargetChanged;
		outRetreatTargetChanged = retreatTargetChanged;
		outCompartmentChanged = compartmentChanged;
		outCurrentTarget = newTarget;
		outPrevTarget = prevTarget;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns selected target destination pos - pos where unit where just after we've lost LOS
	vector GetTargetDestinationPos()
	{
		return m_SelectedTargetDestinationPos;
	}	
	
	//------------------------------------------------------------------------------------------------
	SCR_AIInfoComponent GetAIInfoComponent()
	{
		return m_AIInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks amount of magazines for given weapon, and if low on ammo, reports that to group
	//! \param[in] weaponComp
	//! \param[in] muzzleId
	//! \return
	bool EvaluateLowAmmo(BaseWeaponComponent weaponComp, int muzzleId)
	{
		if (!weaponComp)
			return false;
		array<BaseMuzzleComponent> muzzles = {};
		weaponComp.GetMuzzlesList(muzzles);
		if (muzzleId >= muzzles.Count() || muzzleId < 0)
			return false;
		
		BaseMuzzleComponent muzzleComp = muzzles[muzzleId];
		if (!muzzleComp)
			return false;
				
		// Ignore disposable weapons
		if (muzzleComp.IsDisposable())
			return false;
		
		int magCount = m_InventoryManager.GetMagazineCountByWeapon(weaponComp);
		int lowMagThreshold = GetWeaponLowMagThreshold(weaponComp);
		
		return magCount < lowMagThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetWeaponLowMagThreshold(BaseWeaponComponent weapon)
	{
		EWeaponType wpType = SCR_AIWeaponHandling.GetWeaponType(weapon, true);
		SCR_AIWeaponTypeHandlingConfig config = m_Utility.m_ConfigComponent.GetWeaponTypeHandlingConfig(wpType);
		if (!config)
			return SCR_AIWeaponTypeHandlingConfig.DEFAULT_LOW_MAG_THRESHOLD;
		
		return config.m_iLowMagCountThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSliceMs
	void Update(float timeSliceMs)
	{
		// Evaluate if we must dismount turret - only if we are already in turret
		if (m_CurrentTurretController)
			EvaluateDismountTurret(timeSliceMs);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Condition to dismount turret
	//! Returns true when we should dismount it immediately
	//! \param[in,out] targetPos
	//! \param[in] targetPosProvided
	//! \return
	bool DismountTurretCondition(inout vector targetPos, bool targetPosProvided)
	{
		// False if not in turret
		if (!m_CurrentTurretController)
			return false;
		
		// False if we have a valid target to attack
		if (m_SelectedTarget)
			return false;
		
		TurretComponent turretComp = m_CurrentTurretController.GetTurretComponent();
		if (!turretComp)
			return false;
		
		// False if we have a driver in the vehicle
		array<BaseCompartmentSlot> compartments = {};
		m_CurrentVehicleCompartmentManager.GetCompartments(compartments);
		foreach (BaseCompartmentSlot slot : compartments)
		{
			if (PilotCompartmentSlot.Cast(slot) && slot.GetOccupant())
				return false;
		}
		
		// False if we are in a vehicle and we should not leave turret of this vehicle type
		// Note that static turrets are not of Vehicle class.
		Vehicle vehicle = Vehicle.Cast(m_CurrentVehicle);
		if (vehicle && s_aForbidDismountTurretsOfVehicleTypes.Find(vehicle.m_eVehicleType) != -1)
			return false;
		
		// If target pos is not provided, find a target which we are going to check against
		if (!targetPosProvided)
		{
			BaseTarget target = m_Perception.GetClosestTarget(ETargetCategory.DETECTED, DISMOUNT_TURRET_TARGET_LAST_SEEN_MAX_S, DISMOUNT_TURRET_TARGET_LAST_SEEN_MAX_S);
			if (target)
				targetPos = target.GetLastDetectedPosition();
			else
			{
				target = m_Perception.GetClosestTarget(ETargetCategory.ENEMY, DISMOUNT_TURRET_TARGET_LAST_SEEN_MAX_S, DISMOUNT_TURRET_TARGET_LAST_SEEN_MAX_S);
				if (target)
					targetPos = target.GetLastSeenPosition();
			}
			
			if (!target)
			{
				// If there is no target, try to get data out of threat system
				int sectorMajor, sectorMinor;
				m_Utility.m_SectorThreatFilter.GetActiveSectors(sectorMajor, sectorMinor);
				
				// False if there's nothing in threat system
				if (sectorMajor == -1)
					return false;
				
				targetPos = m_Utility.m_SectorThreatFilter.GetSectorPos(sectorMajor);
			}
			else
			{
				IEntity targetEntity = target.GetTargetEntity();
				if (!targetEntity)
					return false;
				else
				{
					vector bmin, bmax;
					targetEntity.GetBounds(bmin, bmax);
					targetPos = targetPos + 0.5 * (bmin + bmax);
				}
			}
		}
			
		// Check angle excess of the target's position
		vector angleExcess = turretComp.GetAimingAngleExcess(targetPos);
		
		//PrintFormat("Excess angle: %1", angleExcess);
		
		return angleExcess.Length() > TURRET_TARGET_EXCESS_ANGLE_THRESHOLD_DEG;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EvaluateDismountTurret(float timeSliceMs)
	{
		vector targetPos;
		bool mustDismount = DismountTurretCondition(targetPos, false);
		
		if (!mustDismount)
		{
			m_fDismountTurretTimer = 0;
		}
		else
		{
			// Do nothing if already requested to dismount
			if (m_fDismountTurretTimer == -1.0)
				return;
			
			m_fDismountTurretTimer += timeSliceMs;
			
			if (m_fDismountTurretTimer > DISMOUNT_TURRET_TIMER_MS)
			{
				m_fDismountTurretTimer = -1.0;
				
				TryAddDismountTurretActions(targetPos);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adds action to dismount turret, investigate, and go back
	//! \param[in] targetPos
	//! \param[in] addGetOut
	//! \param[in] addGetIn
	void TryAddDismountTurretActions(vector targetPos, bool addGetOut = true, bool addGetIn = true)
	{
		BaseCompartmentSlot compartmentSlot = m_CurrentCompartmentSlot;
		if (!compartmentSlot)
			return;
		
		if (addGetOut)
		{
			AIActionBase prevGetOutAction = m_Utility.FindActionOfType(SCR_AIGetOutVehicle);
			if (prevGetOutAction)
				return;
			
			SCR_AIGetOutVehicle getOutAction = new SCR_AIGetOutVehicle(m_Utility, null, compartmentSlot.GetOwner(), priority: SCR_AIActionBase.PRIORITY_BEHAVIOR_DISMOUNT_TURRET);
			m_Utility.AddAction(getOutAction);
		}
		
		if (addGetIn)
		{	
			AIActionBase prevGetInAction = m_Utility.FindActionOfType(SCR_AIGetInVehicle);
			if (!prevGetInAction)
			{
				SCR_AIGetInVehicle getInAction = new SCR_AIGetInVehicle(m_Utility, null, compartmentSlot.GetVehicle(), compartmentSlot, ECompartmentType.TURRET, SCR_AIActionBase.PRIORITY_BEHAVIOR_DISMOUNT_TURRET_GET_IN);
				m_Utility.AddAction(getInAction);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Event_OnInventoryChanged(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		// This event spams so much, especially at start, we want to process it only once
		//ScriptCallQueue callQueue = GetGame().GetCallqueue();
		//callQueue.Remove(Event_OnTimerAfterInventoryChanged);
		//callQueue.CallLater(Event_OnTimerAfterInventoryChanged, 2500, false);
	}
	protected void Event_OnTimerAfterInventoryChanged()
	{
		//EvaluateAndReportOutOfAmmo();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] vehicle
	//! \param[in] manager
	//! \param[in] mgrID
	//! \param[in] slotID
	protected void Event_OnCompartmentEntered( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{
		m_CurrentCompartmentSlot = manager.FindCompartment(slotID, mgrID);
		IEntity compartmentEntity = m_CurrentCompartmentSlot.GetOwner();
		m_CurrentTurretController = TurretControllerComponent.Cast(compartmentEntity.FindComponent(TurretControllerComponent));
		m_CurrentVehicle = m_CurrentCompartmentSlot.GetVehicle();
		
		// Ensure it's compartment mgr on root of entity, not on a bench
		if (m_CurrentVehicle)
			m_CurrentVehicleCompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(m_CurrentVehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		else
			m_CurrentVehicleCompartmentManager = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] vehicle
	//! \param[in] manager
	//! \param[in] mgrID
	//! \param[in] slotID
	protected void Event_OnCompartmentLeft( IEntity vehicle, BaseCompartmentManagerComponent manager, int mgrID, int slotID )
	{
		m_CurrentVehicle = null;
		m_bCurrentVehicleEvac = false;
		m_CurrentCompartmentSlot = null;
		m_CurrentTurretController = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Event_OnGadgetStateChanged(IEntity gadget, bool isInHand, bool isOnGround)
	{
		if (isInHand)
		{
			SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(gadget.FindComponent(SCR_GadgetComponent));
			if (gadgetComp)
				m_eCurrentGadgetType = gadgetComp.GetType();
			else
				m_eCurrentGadgetType = -1;
		}
		else
			m_eCurrentGadgetType = -1;
		
		RecalculateEquipmentPerceptionFactor();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Event_OnGadgetFocusStateChanged(IEntity gadget, bool isFocused)
	{
		m_bGadgetFocus = isFocused;
		RecalculateEquipmentPerceptionFactor();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Event_OnDamage(BaseDamageContext damageContext)
	{
		if (damageContext.damageType != EDamageType.FIRE || !m_Utility || !m_CurrentVehicle || m_bCurrentVehicleEvac)
			return;
		
		// Fire damage inside a vehicle - evac vehicle
		m_bCurrentVehicleEvac = true;
		
		SCR_AIGetOutVehicle behaviorGetOut = new SCR_AIGetOutVehicle(m_Utility, null, m_CurrentVehicle, priority: SCR_AIActionBase.PRIORITY_BEHAVIOR_GET_OUT_VEHICLE_HIGH_PRIORITY);
		m_Utility.AddAction(behaviorGetOut);
		
		SCR_AIMoveFromDangerBehavior behaviorMoveFromDanger = new SCR_AIMoveFromDangerBehavior(m_Utility, null, m_CurrentVehicle.GetOrigin(), m_CurrentVehicle);
		m_Utility.AddAction(behaviorMoveFromDanger);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Event_OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		if (!m_Utility || !m_Utility.m_AIInfo || dmgEffect.GetDamageType() != EDamageType.BLEEDING || !DotDamageEffect.Cast(dmgEffect))
			return;
		
		SCR_AIActionBase currentAction = SCR_AIActionBase.Cast(m_Utility.GetCurrentAction());
		if (!currentAction)
			return;
		
		float priorityLevelClamped = currentAction.GetRestrictedPriorityLevel();
		
		if (!m_Utility.m_AIInfo.HasUnitState(EUnitState.UNCONSCIOUS) && m_Utility.m_AIInfo.HasRole(EUnitRole.MEDIC))
		{
			if (!m_Utility.HasActionOfType(SCR_AIHealBehavior))
			{
				// If we can heal ourselves, add Heal Behavior.
				SCR_AIHealBehavior behavior = new SCR_AIHealBehavior(m_Utility, null, m_Utility.m_OwnerEntity, true, priorityLevel: priorityLevelClamped);
				m_Utility.AddAction(behavior);
			}
		}
		else if (m_Agent)
		{
			// If we immediately know that we can't heal ourselves, report to group
			AIGroup myGroup = m_Agent.GetParentGroup();
			if (myGroup)
			{
				// Hack: we use group's comms component, and send from it to it directly, because if this AI is unconscious, his mailbox is already disabled.
				AICommunicationComponent comms = myGroup.GetCommunicationComponent();
				if (comms)
				{
					SCR_AIMessage_Wounded msg = SCR_AIMessage_Wounded.Create(m_Utility.m_OwnerEntity);
					
					#ifdef AI_DEBUG
					msg.m_sSentFromBt = "SCR_AICombatComponent.Event_OnDamageEffectAdded";
					#endif
					
					msg.SetReceiver(myGroup);
					comms.RequestBroadcast(msg, myGroup);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns current combat mode of soldier. Now soldier doesn't have his own combat mode, it's dictated by group.
	EAIGroupCombatMode GetCombatMode()
	{
		SCR_AIGroup myGroup = SCR_AIGroup.Cast(GetAiAgent().GetParentGroup());
		if (!myGroup)
			return EAIGroupCombatMode.FIRE_AT_WILL;
		
		return myGroup.GetGroupUtilityComponent().GetCombatModeActual();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsFriendlyInAim()
	{
		IEntity friendlyEntInAim = m_Perception.GetFriendlyInLineOfFire();
#ifdef WORKBENCH
		if (friendlyEntInAim && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_FRIENDLY_IN_AIM))
			m_FriendlyAimShape = Shape.CreateSphere(COLOR_RED, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, friendlyEntInAim.GetOrigin() + Vector(0, 2, 0), 0.1);	
		else 
			m_FriendlyAimShape = null;
#endif		
		m_bFriendlyAimLastResult = friendlyEntInAim != null;
				
		return m_bFriendlyAimLastResult;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Checks if given target can be considered visible
	//! \param[in] target
	//! \return
	bool IsTargetVisible(notnull BaseTarget target)
	{
		// When m_lastSeenMax value is lower than perception component update interval,
		// which might be large at higher LOD levels, the decorator might periodically return false,
		// even when target is still visible.
		// Therefore we must prevent the threshold from being smaller than update interval of perception component.		
		float lastSeenMax = Math.Max(TARGET_MAX_LAST_SEEN_VISIBLE, m_Perception.GetUpdateInterval()) + 0.02;
		bool visible = target.GetTimeSinceSeen() < lastSeenMax;
		return visible;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Either array of targets or target cluster must be provided
	//! \param[in] assignedTargets
	//! \param[in] clusterState
	void SetAssignedTargets(array<IEntity> assignedTargets, SCR_AITargetClusterState clusterState)
	{
		m_aAssignedTargets.Clear();
		
		if (assignedTargets)
			m_aAssignedTargets.Copy(assignedTargets);
		else if (clusterState)
			m_TargetClusterState = clusterState;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetAssignedTargets()
	{
		m_aAssignedTargets.Clear();
		m_TargetClusterState = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] expectedEnemyType
	void SetExpectedEnemyType(EAIUnitType expectedEnemyType)
	{
		m_eExpectedEnemyType = expectedEnemyType;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetExpectedEnemyType()
	{
		m_eExpectedEnemyType = EAIUnitType.UnitType_Infantry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getters which return the weapon which combat component thinks is best against current target
	//! If there is no current target, then there is no selected weapon as well!
	//! In that case use GetExpectedWeapon
	//! \param[out] outWeaponComp
	//! \param[out] outMuzzleId
	//! \param[out] outMagazineComp
	void GetSelectedWeaponAndMagazine(out BaseWeaponComponent outWeaponComp, out int outMuzzleId, out BaseMagazineComponent outMagazineComp)
	{
		if (m_SelectedWeaponComp)
		{
			outWeaponComp = m_SelectedWeaponComp;
			outMagazineComp = m_SelectedMagazineComp;
			outMuzzleId = m_iSelectedMuzzle;
		}
		else
		{
			outWeaponComp = null;
			outMuzzleId = -1;
			outMagazineComp = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] outWeaponComp
	//! \param[out] outMuzzleId
	void GetSelectedWeapon(out BaseWeaponComponent outWeaponComp, out int outMuzzleId)
	{
		if (m_SelectedWeaponComp)
		{
			outWeaponComp = m_SelectedWeaponComp;
			outMuzzleId = m_iSelectedMuzzle;
		}
		else
		{
			outWeaponComp = null;
			outMuzzleId = -1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EWeaponType GetSelectedWeaponType()
	{
		if (m_SelectedWeaponComp)
			return m_SelectedWeaponComp.GetWeaponType();
		else
			return EWeaponType.WT_NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] outMinDistance
	//! \param[out] outMaxDistance
	//! \param[out] outDirectDamage
	void GetSelectedWeaponProperties(out float outMinDistance, out float outMaxDistance, out bool outDirectDamage)
	{
		outMinDistance = m_fSelectedWeaponMinDist;
		outMaxDistance = m_fSelectedWeaponMaxDist;
		outDirectDamage = m_bSelectedWeaponDirectDamage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetSelectedWeaponDirectDamage()
	{
		return m_bSelectedWeaponDirectDamage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSelectedWeaponMinDist()
	{
		return m_fSelectedWeaponMinDist;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSelectedWeaponMaxDist()
	{
		return m_fSelectedWeaponMaxDist;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetSelectedWeaponResource()
	{
		return m_SelectedWeaponResource;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EAIUnitType GetUnitTypesCanAttack()
	{
		return m_eUnitTypesCanAttack;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns weapon which combat component thinks is expected to be used this situation
	//! This should be used for selecting fallback weapon in a situation without an actively attacked target
	//! \param[out] outWeaponComp
	//! \param[out] outMuzzleId
	//! \param[out] outMagazineComp
	void EvaluateExpectedWeapon(out BaseWeaponComponent outWeaponComp, out int outMuzzleId, out BaseMagazineComponent outMagazineComp)
	{
		bool useCompartmentWeapons = m_AIInfo.HasUnitState(EUnitState.IN_TURRET);
		bool success = m_WeaponTargetSelector.SelectWeaponAgainstUnitType(m_eExpectedEnemyType, useCompartmentWeapons);
		
		if (success)
		{
			BaseWeaponComponent weaponComp;
			int muzzleId;
			BaseMagazineComponent magazineComp;
			
			m_WeaponTargetSelector.GetSelectedWeapon(weaponComp, muzzleId, magazineComp);
			
			outWeaponComp = weaponComp;
			outMuzzleId = muzzleId;
			outMagazineComp = magazineComp;
		}
		else
		{
			outWeaponComp = null;
			outMuzzleId = null;
			outMagazineComp = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns optimal weapon which can be used for suppressing target of given type at given range.
	//! This will first of all return weapons which are configured in prefabs to cause indirect damage.
	void EvaluateSuppressionWeapon(EAIUnitType targetUnitType, float distanceToTarget, out BaseWeaponComponent outWeaponComp, out int outMuzzleId, out BaseMagazineComponent outMagazineComp)
	{
		bool useCompartmentWeapons = m_AIInfo.HasUnitState(EUnitState.IN_TURRET);
		bool success = m_WeaponTargetSelector.SelectWeaponAgainstUnitTypeAndDistance(targetUnitType, distanceToTarget, false, useCompartmentWeapons,
			weaponTypesWhitelist: null,
			weaponTypesBlacklist: s_aWeaponBlacklistFragGrenades,
			minMagCountSpec: m_ConfigComponent.m_aMinSuppressiveMagCountSpec);
		
		if (success)
		{
			BaseWeaponComponent weaponComp;
			int muzzleId;
			BaseMagazineComponent magazineComp;
			
			m_WeaponTargetSelector.GetSelectedWeapon(weaponComp, muzzleId, magazineComp);
			
			outWeaponComp = weaponComp;
			outMuzzleId = muzzleId;
			outMagazineComp = magazineComp;
		}
		else
		{
			outWeaponComp = null;
			outMuzzleId = null;
			outMagazineComp = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponType
	//! \return
	bool HasWeaponOfType(EWeaponType weaponType)
	{
		return m_WeaponTargetSelector.HasWeaponOfType(weaponType, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] weaponType
	//! \return
	BaseWeaponComponent FindWeaponOfType(EWeaponType weaponType)
	{
		return m_WeaponTargetSelector.FindWeaponOfType(weaponType, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] magazineWell
	//! \param[in] countMagazineInWeapon
	//! \return
	int GetMagazineCount(typename magazineWell, bool countMagazineInWeapon = false)
	{
		int count = m_WeaponTargetSelector.GetMagazineCount(magazineWell, false);
		return count - 1 + countMagazineInWeapon;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_eAISkill = m_eAISkillDefault;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (character)
		{	
			m_CharacterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (m_CharacterController)
			{
				m_CharacterController.m_OnGadgetStateChangedInvoker.Insert(Event_OnGadgetStateChanged);
				m_CharacterController.m_OnGadgetFocusStateChangedInvoker.Insert(Event_OnGadgetFocusStateChanged);
				
				m_WpnManager = m_CharacterController.GetWeaponManagerComponent();
			
				m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_CharacterController.GetInventoryStorageManager());
				if (m_InventoryManager)
				{
					m_InventoryManager.m_OnItemAddedInvoker.Insert(Event_OnInventoryChanged);
					m_InventoryManager.m_OnItemRemovedInvoker.Insert(Event_OnInventoryChanged);
				}
			}
			
			m_CompartmentAccess = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
			if (m_CompartmentAccess)
			{
				m_CompartmentAccess.GetOnCompartmentEntered().Insert(Event_OnCompartmentEntered);
				m_CompartmentAccess.GetOnCompartmentLeft().Insert(Event_OnCompartmentLeft);
			}
		}	
			
		m_Perception = PerceptionComponent.Cast(owner.FindComponent(PerceptionComponent));
		
		BaseWorld world = GetGame().GetWorld();
		if (world)
		{
			float worldTime = world.GetWorldTime();
			m_fNextWeaponTargetEvaluation_ms = worldTime + Math.RandomFloat(0, WEAPON_TARGET_UPDATE_PERIOD_MS);
		}

		if (owner)
		{
			InitWeaponTargetSelector(owner);
		}

		m_DamageManager = SCR_ExtendedDamageManagerComponent.Cast(owner.FindComponent(SCR_ExtendedDamageManagerComponent));
		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamageEffectAdded().Insert(Event_OnDamageEffectAdded);
			m_DamageManager.GetOnDamage().Insert(Event_OnDamage);
		}
		
		AIControlComponent ctrl = AIControlComponent.Cast(owner.FindComponent(AIControlComponent));
		if (ctrl)
		{
			SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(ctrl.GetAIAgent());
			if (agent)
			{
				m_AIInfo = agent.m_InfoComponent;
				m_ConfigComponent = SCR_AIConfigComponent.Cast(agent.FindComponent(SCR_AIConfigComponent));
				m_Utility = SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_CompartmentAccess)
		{
			m_CompartmentAccess.GetOnCompartmentEntered().Remove(Event_OnCompartmentEntered);
			m_CompartmentAccess.GetOnCompartmentLeft().Remove(Event_OnCompartmentLeft);
		}
		
		if (m_CharacterController)
		{
			m_CharacterController.m_OnGadgetStateChangedInvoker.Remove(Event_OnGadgetStateChanged);
			m_CharacterController.m_OnGadgetFocusStateChangedInvoker.Remove(Event_OnGadgetFocusStateChanged);
		}
		
		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamageEffectAdded().Remove(Event_OnDamageEffectAdded);
			m_DamageManager.GetOnDamage().Remove(Event_OnDamage);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitWeaponTargetSelector(IEntity owner)
	{
		m_WeaponTargetSelector.Init(owner);
		
		// maxLastSeenIndirect must be consistent with conditions in ShouldAttackEndForTarget, therefore we use TARGET_INVISIBLE_TIME here
		SetTargetSelectionProperties(false);

		
		m_WeaponTargetSelector.SetTargetScoreConstants(EAIUnitType.UnitType_Infantry,			100.0,	-0.1); // At short range we prefer to shoot enemies in vehicle
		m_WeaponTargetSelector.SetTargetScoreConstants(EAIUnitType.UnitType_VehicleUnarmored,	99.0,	-0.08);
		m_WeaponTargetSelector.SetTargetScoreConstants(EAIUnitType.UnitType_VehicleMedium,		150.0,	-0.15);
		m_WeaponTargetSelector.SetTargetScoreConstants(EAIUnitType.UnitType_VehicleHeavy,		200.0,	-0.15);
		m_WeaponTargetSelector.SetTargetScoreConstants(EAIUnitType.UnitType_Aircraft,			90.0,	-0.015);
		m_WeaponTargetSelector.SetTargetScoreConstants(EAIUnitType.UnitType_Fortification,		40.0,	-0.1); // Fortifications are not used ATM
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTargetSelectionProperties(bool closeCombat)
	{
		if (closeCombat)
		{
			// Here TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK_CLOSE is used instead of TARGET_MAX_LAST_SEEN, since it's largest of the two
			m_WeaponTargetSelector.SetSelectionProperties(TARGET_MAX_LAST_SEEN_DIRECT_ATTACK_CLOSE, TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK_CLOSE, TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK_CLOSE,
			TARGET_MIN_INDIRECT_TRACE_FRACTION_MIN, TARGET_MAX_DISTANCE_INFANTRY, TARGET_MAX_DISTANCE_VEHICLE, TARGET_MAX_TIME_SINCE_ENDANGERED, TARGET_MAX_DISTANCE_DISARMED);		
			return;
		}
		
		m_WeaponTargetSelector.SetSelectionProperties(TARGET_MAX_LAST_SEEN_DIRECT_ATTACK, TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK, TARGET_MAX_LAST_SEEN,
		TARGET_MIN_INDIRECT_TRACE_FRACTION_MIN, TARGET_MAX_DISTANCE_INFANTRY, TARGET_MAX_DISTANCE_VEHICLE, TARGET_MAX_TIME_SINCE_ENDANGERED, TARGET_MAX_DISTANCE_DISARMED);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] perceptionComp
	//! \param[in] threatSystem
	void UpdatePerceptionFactor(PerceptionComponent perceptionComp, SCR_AIThreatSystem threatSystem)
	{
		EAIThreatState threatState = threatSystem.GetState();
		float perceptionFactor;
		switch (threatState)
		{
			case EAIThreatState.SAFE:
				perceptionFactor = PERCEPTION_FACTOR_SAFE; break; 
			case EAIThreatState.VIGILANT:
				perceptionFactor = PERCEPTION_FACTOR_VIGILANT; break;
			case EAIThreatState.ALERTED:
				perceptionFactor = PERCEPTION_FACTOR_ALERTED; break; 
			case EAIThreatState.THREATENED:
				perceptionFactor = PERCEPTION_FACTOR_THREATENED; break; 
		}
		
		perceptionFactor *= m_fEquipmentPerceptionFactor;
		perceptionFactor *= m_fPerceptionFactor;
		
		perceptionComp.SetPerceptionFactor(perceptionFactor);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RecalculateEquipmentPerceptionFactor()
	{
		if (m_eCurrentGadgetType == EGadgetType.BINOCULARS && m_bGadgetFocus)
			m_fEquipmentPerceptionFactor = PERCEPTION_FACTOR_EQUIPMENT_BINOCULARS;
		else
			m_fEquipmentPerceptionFactor = PERCEPTION_FACTOR_EQUIPMENT_NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_AICombatComponent()
	{
		GetGame().GetCallqueue().Remove(Event_OnTimerAfterInventoryChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used by AIDebugInfoComponent
	void DebugPrintToWidget(TextWidget w)
	{
		string str = string.Format("\n%1", m_SelectedTarget.ToString());
		if (m_SelectedTarget)
			str = str + string.Format("\n%1", m_SelectedTarget.GetTimeSinceSeen());
		else
			str = str + "\n";
		w.SetText(str);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ChimeraAIAgent GetAiAgent()
	{
		if (m_Agent)
			return m_Agent;
		
		AIControlComponent controlComp = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		m_Agent = SCR_ChimeraAIAgent.Cast(controlComp.GetAIAgent());
		return m_Agent;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ent
	//! \return
	BaseTarget FindTargetByEntity(IEntity ent)
	{
		BaseTarget tgt = m_Perception.FindTargetPerceptionObject(ent);
		if (tgt)
			return tgt;
		
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("FindTargetByEntity: did not find target: %1", ent), LogLevel.WARNING);
		Print(string.Format("FindTargetByEntity: did not find target: %1", ent), LogLevel.WARNING);
		#endif
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] targetInfo
	void UpdateLastSeenPosition(IEntity entity, notnull SCR_AITargetInfo targetInfo)
	{
		BaseTarget tgt = m_Perception.FindTargetPerceptionObject(entity);
		if (!tgt)
			return;
		
		if (targetInfo.m_fTimestamp > tgt.GetTimeLastSeen())
			tgt.UpdateLastSeenPosition(targetInfo.m_vWorldPos, targetInfo.m_fTimestamp);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] tgt
	//! \param[in] targetInfo
	void UpdateLastSeenPosition(notnull BaseTarget tgt, notnull SCR_AITargetInfo targetInfo)
	{		
		if (targetInfo.m_fTimestamp > tgt.GetTimeLastSeen())
			tgt.UpdateLastSeenPosition(targetInfo.m_vWorldPos, targetInfo.m_fTimestamp);
	}
	
	//------------------------------------------------------------------------------------------------
	// Obsolete methods
	[Obsolete("Outdated functionality which is not used anywhere")]
	bool IsActionAllowed(int action) { return true; }
	
	[Obsolete("Outdated functionality which is not used anywhere")]
	int GetAllowedActions() { return 0; }
	
	[Obsolete("Outdated method, does not affect anything in AI")]
	private void SetActionAllowed(int action, bool isAllowed) {}
	
	[Obsolete("Use Combat Modes instead")]
	void SetHoldFire(bool isHoldFire) {}
	
	[Obsolete("Outdated functionality which is not used anywhere")]
	int GetCombatType() {	return 0; }
	
	[Obsolete("Use Combat Modes instead")]
	void SetCombatType(int combatType) {}
	
	[Obsolete()]
	void SetDefaultCombatType(int combatType) {}
	
	[Obsolete()]
	void ResetCombatType() {}
	
	#ifdef AI_DEBUG
	//------------------------------------------------------------------------------------------------
	protected void AddDebugMessage(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		AIControlComponent controlComp = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		AIAgent agent = controlComp.GetAIAgent();
		if (!agent)
			return;
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(agent.FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(str, msgType: EAIDebugMsgType.COMBAT, logLevel);
	}
	#endif
}
