class SCR_AIUpdateTargetAttackData : AITaskScripted
{	
	// Inputs
	protected static const string BASE_TARGET_PORT = "BaseTarget";
	protected static const string WEAPON_IS_READY = "WeaponReady";
	
	// Outputs
	protected static const string PORT_LAST_SEEN_POSITION = "LastSeenPosition";
	protected static const string PORT_THREAT_POSITION = "ThreatPosition";
	protected static const string PORT_VISIBLE = "Visible";
	protected static const string PORT_FIRE_TREE_ID = "FireTreeId";
	protected static const string PORT_FIRE_RATE = "FireRate";
	static const string PORT_AIMPOINT_TYPE_0 = "AimpointType0";
	static const string PORT_AIMPOINT_TYPE_1 = "AimpointType1";
	
	
	// These IDs must match to actual trees in attack tree
	protected const int FIRE_TREE_INVALID 		= -1;	// No aiming or firing is allowed at all
	protected const int FIRE_TREE_LOOK			= 0;	// Looking at target without firing
	protected const int FIRE_TREE_BURST			= 1;
	protected const int FIRE_TREE_SINGLE		= 2;
	protected const int FIRE_TREE_SUPPRESSIVE	= 3;
	protected const int FIRE_TREE_MELEE			= 4;
	protected const int FIRE_TREE_LOOK_THREATS	= 5;	// Looking at data from threat system
	
	protected const float MELEE_MAX_DISTANCE = 2.0;
	protected const float BURST_FIRE_MAX_DISTANCE = 50.0;
	
	protected SCR_ChimeraAIAgent m_Agent;
	protected SCR_AICombatComponent m_CombatComponent;
	protected CharacterControllerComponent m_CharacterController;
	protected PerceptionComponent m_PerceptionComponent;
	protected SCR_AIUtilityComponent m_UtilityComponent;
	protected SCR_AISectorThreatFilter m_ThreatFilter;
	protected BaseTarget m_Target;
	
	// Flag for executing some logic only once at start
	protected bool m_bFirstSimulate = true;
	
	protected bool m_bWeaponHasBurstOrAuto; // Cached on first run
	
	// State of looking at threat system
	protected bool m_bLookAtThreats;
	protected WorldTimestamp m_LookAtThreatsEndTime;
	protected vector m_vLookAtThreatsPos;
	
	//-----------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_UtilityComponent = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		m_Agent = SCR_ChimeraAIAgent.Cast(owner);
		
		IEntity myEntity = owner.GetControlledEntity();
		if (myEntity)
		{
			m_CombatComponent = SCR_AICombatComponent.Cast(myEntity.FindComponent(SCR_AICombatComponent));
			
			m_PerceptionComponent = PerceptionComponent.Cast(myEntity.FindComponent(PerceptionComponent));
			m_CharacterController = CharacterControllerComponent.Cast(myEntity.FindComponent(CharacterControllerComponent));
		}
		
		if (m_UtilityComponent)
		{
			m_ThreatFilter = m_UtilityComponent.m_SectorThreatFilter;
			
			m_ThreatFilter.GetOnEscalationInvoker().Remove(OnThreatSectorEscalation);
			m_ThreatFilter.GetOnEscalationInvoker().Insert(OnThreatSectorEscalation);
			
			m_ThreatFilter.GetOnDamageTaken().Remove(OnThreatSectorDamageTaken);
			m_ThreatFilter.GetOnDamageTaken().Insert(OnThreatSectorDamageTaken);
		}
			
		m_bFirstSimulate = true;
	}
	
	//-----------------------------------------------------------------------------------------------------
	void ~SCR_AIUpdateTargetAttackData()
	{
		if (m_ThreatFilter)
		{
			m_ThreatFilter.GetOnEscalationInvoker().Remove(OnThreatSectorEscalation);
			m_ThreatFilter.GetOnDamageTaken().Remove(OnThreatSectorDamageTaken);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		m_bFirstSimulate = true;
	}
	
	//-----------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseTarget target;
		GetVariableIn(BASE_TARGET_PORT, target);
		m_Target = target;
		
		bool weaponReady;
		GetVariableIn(WEAPON_IS_READY, weaponReady);
		
		if (!target || !m_CombatComponent || !m_PerceptionComponent || !m_CharacterController || !m_UtilityComponent)
			return ENodeResult.FAIL;
		
		if (m_bFirstSimulate)
		{
			BaseWeaponComponent selectedWeaponComp;
			int selectedMuzzleId;
			m_CombatComponent.GetSelectedWeapon(selectedWeaponComp, selectedMuzzleId);
			m_bWeaponHasBurstOrAuto = WeaponHasBurstOrAutoMode(selectedWeaponComp, selectedMuzzleId);
		}
		
		// Reset looking at threats if time is out
		if (m_bLookAtThreats)
		{
			WorldTimestamp timeNow = GetGame().GetWorld().GetTimestamp();
			if (timeNow.Greater(m_LookAtThreatsEndTime))
				m_bLookAtThreats = false;
		}
		
		// Last seen pos
		SetVariableOut(PORT_LAST_SEEN_POSITION, target.GetLastSeenPosition());
		
		// Threat pos (only relevant if we're in threat observation state)
		SetVariableOut(PORT_THREAT_POSITION, m_vLookAtThreatsPos);
		
		// Visible?
		bool visible = m_CombatComponent.IsTargetVisible(target);
		SetVariableOut(PORT_VISIBLE, visible);
		
		// Fire rate modifier (modified in ResolveFireTree)
		float fireRate = 1;
		
		// Which fire tree to use?
		// This must be reevaluated periodically
		int fireTreeId = ResolveFireTree(target, visible, weaponReady, fireRate);
		SetVariableOut(PORT_FIRE_TREE_ID, fireTreeId);
		SetVariableOut(PORT_FIRE_RATE, fireRate);
		
		// Which aimpoints to use?
		// We run this only once, since it's not going to change much
		if (m_bFirstSimulate)
		{
			EAimPointType aimpointType0;
			EAimPointType aimpointType1;
			ResolveAimpointTypes(target, aimpointType0, aimpointType1);
			SetVariableOut(PORT_AIMPOINT_TYPE_0, aimpointType0);
			SetVariableOut(PORT_AIMPOINT_TYPE_1, aimpointType1);
		}
		
		// Reset m_bFirstSimulate flag
		m_bFirstSimulate = false;
		
		return ENodeResult.SUCCESS;
	}
	
	//-----------------------------------------------------------------------------------------------------
	// Evaluates which fire tree should be used
	int ResolveFireTree(BaseTarget target, bool visible, bool weaponReady, out float fireRate)
	{
		// Is aiming forbidden by combat move?
		SCR_AIBehaviorBase executedBehavior = SCR_AIBehaviorBase.Cast(m_UtilityComponent.GetExecutedAction());
		if (executedBehavior && executedBehavior.m_bUseCombatMove && !m_UtilityComponent.m_CombatMoveState.m_bAimAtTarget)
			return FIRE_TREE_INVALID;
		
		// Is looking at threats activated?
		if (m_bLookAtThreats)
			return FIRE_TREE_LOOK_THREATS;
		
		// Is weapon not ready?
		if (!weaponReady)
			return FIRE_TREE_LOOK;
		
		BaseWeaponComponent selectedWeaponComp;
		int selectedMuzzleId;
		m_CombatComponent.GetSelectedWeapon(selectedWeaponComp, selectedMuzzleId);
		
		bool directDamage
		float weaponMinDist, weaponMaxDist;
		m_CombatComponent.GetSelectedWeaponProperties(weaponMinDist, weaponMaxDist, directDamage);
		
		if (!selectedWeaponComp)
			return FIRE_TREE_LOOK;
		
		// Hold fire?
		if (m_CombatComponent.GetCombatMode() == EAIGroupCombatMode.HOLD_FIRE)
			return FIRE_TREE_LOOK;
		
		EWeaponType weaponType = selectedWeaponComp.GetWeaponType();
		
		float targetDistance = target.GetDistance();
		
		// Use melee?
		if (targetDistance < MELEE_MAX_DISTANCE &&
			!m_CharacterController.CanFire() &&
			m_CharacterController.GetStance() != ECharacterStance.PRONE)
		{
			return FIRE_TREE_MELEE;
		}
		
		// Friendly in aim?
		if (m_PerceptionComponent.GetFriendlyInLineOfFire())
		{
			return FIRE_TREE_LOOK;
		}
	
		
		// Not melee
		
		if (targetDistance < weaponMinDist || targetDistance > weaponMaxDist)
		{
			// Outside weapon usage range
			// Look at target
			
			return FIRE_TREE_LOOK;
		}

		// Within weapon usage range
		
		if (visible)
		{
			// Visible
			// If machinegun, always use burst at any range
			// For regular weapons, use burst at short range if available, otherwise single
			if (weaponType == EWeaponType.WT_MACHINEGUN)
				return FIRE_TREE_BURST;
			else if (targetDistance < BURST_FIRE_MAX_DISTANCE && m_bWeaponHasBurstOrAuto)
				return FIRE_TREE_BURST;
			else
				return FIRE_TREE_SINGLE;
			
		}
		else
		{
			// Invisible
			// Use suppressive fire or don't fire at all
			
			// If weapon is configured to deal indirect damage, then use it for invisible target
			// Otherwise regular weapons can also be used to fire at hidden target,
			// except for rocket launchers, their ammo is too valuable
			// Also ensure we don't do suppressive fire into a wall in front of us
			
			float threat = m_UtilityComponent.m_ThreatSystem.GetThreatMeasure();
			float lastSeenThreshold;
			if (weaponType == EWeaponType.WT_MACHINEGUN)
				lastSeenThreshold = SCR_AICombatComponent.TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK_MG;
			else
			{
				if (targetDistance < SCR_AICombatComponent.CLOSE_RANGE_COMBAT_DISTANCE)
					lastSeenThreshold = SCR_AICombatComponent.TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK_CLOSE;
				else
					lastSeenThreshold = SCR_AICombatComponent.TARGET_MAX_LAST_SEEN_INDIRECT_ATTACK;
			}
			
			lastSeenThreshold = Math.Max(SCR_AICombatComponent.TARGET_MIN_LAST_SEEN_INDIRECT_ATTACK, lastSeenThreshold * threat);
			
			if ((!directDamage || weaponType != EWeaponType.WT_ROCKETLAUNCHER) &&
				target.GetTimeSinceSeen() < lastSeenThreshold &&
				target.GetTraceFraction() > 0.5)
			{
				float maxFireRate = Math.Max(1, Math.Map(targetDistance, 0, SCR_AICombatComponent.LONG_RANGE_COMBAT_DISTANCE, 2, 1));
				fireRate = maxFireRate * threat;
								
				return FIRE_TREE_SUPPRESSIVE;
			}
			else
				return FIRE_TREE_LOOK;
		}
		
		return FIRE_TREE_LOOK;
	}

	//-----------------------------------------------------------------------------------------------------
	static bool WeaponHasBurstOrAutoMode(notnull BaseWeaponComponent weaponComp, int muzzleId)
	{		
		array<BaseMuzzleComponent> muzzles = {};
		weaponComp.GetMuzzlesList(muzzles);
		
		if (!muzzles.IsIndexValid(muzzleId))
			return false;
		
		BaseMuzzleComponent muzzle = muzzles[muzzleId];
		if (!muzzle)
			return false;
		
		array<BaseFireMode> fireModes = {};
		muzzle.GetFireModesList(fireModes);
		foreach (BaseFireMode mode : fireModes)
		{
			EWeaponFiremodeType modeType = mode.GetFiremodeType();
			if (modeType == EWeaponFiremodeType.Burst || modeType == EWeaponFiremodeType.Auto)
				return true;
		}
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------
	void ResolveAimpointTypes(notnull BaseTarget target, out EAimPointType aimpointType0, out EAimPointType aimpointType1)
	{
		IEntity targetEntity = target.GetTargetEntity();
		if (!targetEntity)
		{
			aimpointType0 = -1;
			aimpointType1 = -1;
			return;
		}
					
		ChimeraCharacter character = ChimeraCharacter.Cast(targetEntity);
		if (character)
		{
			// Characters
			
			if (character.IsInVehicle())
			{
				// Aim at head
				aimpointType0 = EAimPointType.WEAK;
				aimpointType1 = EAimPointType.NORMAL;
				return;
			}
			
			aimpointType0 = EAimPointType.NORMAL;
			aimpointType1 = EAimPointType.WEAK;
			return;
		}
		else
		{
			// Vehicles and the rest
			
			EWeaponType weaponType = m_CombatComponent.GetSelectedWeaponType();
			
			if (weaponType == EWeaponType.WT_ROCKETLAUNCHER)
			{
				// Rocket launcher, aim at weak point
				aimpointType0 = EAimPointType.WEAK;
				aimpointType1 = EAimPointType.NORMAL;
				return;
			}
			
			aimpointType0 = EAimPointType.NORMAL;
			aimpointType1 = EAimPointType.WEAK;
			return;
		}
	}
	
	//-----------------------------------------------------------------------------------------------------
	// Handling of Look At Threats state
	// If we are attacking a target and then something more horrible happens sideways, we look at it briefly.
	
	//-----------------------------------------------------------------------------------------------------
	protected void OnThreatSectorEscalation(SCR_AISectorThreatFilter ts, int sectorId, float dangerValue)
	{
		vector sectorPos = ts.GetSectorPos(sectorId);
		m_vLookAtThreatsPos = sectorPos;
		
		// Ignore if it's already in our view cone
		if (PosSameDirectionAsTarget(sectorPos))
			return;
		
		SCR_EAIThreatSectorFlags flags = ts.GetSectorFlags(sectorId);
		float lookDuration = CalculateLookAtThreatDuration(m_Target, flags);
		LookAtThreats(lookDuration);
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected void OnThreatSectorDamageTaken(SCR_AISectorThreatFilter ts, int sectorId)
	{
		vector sectorPos = ts.GetSectorPos(sectorId);
		m_vLookAtThreatsPos = sectorPos;
		
		// Ignore if it's already in our view cone
		if (PosSameDirectionAsTarget(sectorPos))
			return;
		
		SCR_EAIThreatSectorFlags flags = ts.GetSectorFlags(sectorId);
		float lookDuration = CalculateLookAtThreatDuration(m_Target, flags);
		LookAtThreats(lookDuration);
	}
	
	//-----------------------------------------------------------------------------------------------------
	//! Activates state when we are looking at threats
	protected void LookAtThreats(float duration_s)
	{
		m_bLookAtThreats = true;
		WorldTimestamp timeNow = GetGame().GetWorld().GetTimestamp();
		m_LookAtThreatsEndTime = timeNow.PlusSeconds(2.0);
		
		// Also allow aiming, even if it was forbidden temporary.
		// Character might not be able to look around now, but this is very important to look elsewhere.
		m_UtilityComponent.m_CombatMoveState.EnableAiming(true);
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected float CalculateLookAtThreatDuration(BaseTarget currentTarget, SCR_EAIThreatSectorFlags threatFlags)
	{
		// Time depends on how far our target is.
		// The further the target, the more time we can spare to look at something else.
		float tgtDist = currentTarget.GetDistance();
		float baseDuration = -5.3 + 4*Math.Log10(tgtDist + 21.2);
		
		if (threatFlags & SCR_EAIThreatSectorFlags.CAUSED_DAMAGE)
			baseDuration *= 2.0;
		else if (threatFlags & SCR_EAIThreatSectorFlags.DIRECTED_AT_ME)
			baseDuration *= 1.5;
		
		float duration = Math.RandomFloat(baseDuration, 1.5*baseDuration);
		
		return duration;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static const float COS_VIEW_CONE = 0.707; // Cos 45, should be same as view cone as perception
	protected bool PosSameDirectionAsTarget(vector threatPos)
	{
		IEntity controlledEntity = m_Agent.GetControlledEntity();
		
		if (!controlledEntity || !m_Target)
			return false;
		
		vector myPos = controlledEntity.GetOrigin();
		vector targetPos = m_Target.GetLastSeenPosition();
		
		vector dirToTarget = (targetPos - myPos).Normalized();
		vector dirToThreat = (threatPos - myPos).Normalized();
		float cosAngle = vector.Dot(dirToTarget, dirToThreat);
		
		return cosAngle > COS_VIEW_CONE;		
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected ref TStringArray s_aVarsIn = {
		BASE_TARGET_PORT,
		WEAPON_IS_READY
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected ref TStringArray s_aVarsOut = {
		PORT_VISIBLE,
		PORT_LAST_SEEN_POSITION,
		PORT_THREAT_POSITION,
		PORT_FIRE_TREE_ID,
		PORT_AIMPOINT_TYPE_0,
		PORT_AIMPOINT_TYPE_1,
		PORT_FIRE_RATE
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription() { return "Special node which is used in attack behavior"; };

}