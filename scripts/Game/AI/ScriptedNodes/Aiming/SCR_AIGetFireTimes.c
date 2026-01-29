enum EAIFirePattern
{
	Single,
	Burst,
	Suppress,
}

class SCR_AIGetFireTimes: AITaskScripted
{
	static const string PORT_FIRE_TIME = "FireTime";
	static const string PORT_STABILIZATION_TIME = "StabilizationTime";
	static const string PORT_REJECT_TIME = "RejectAimingTime";
	static const string PORT_SHOT_SPAN = "ShotSpan";
	
	protected static string TARGET_ENTITY_PORT = "TargetEntity";
	protected static string TARGET_POSITION_PORT = "TargetPosition";
	protected static string FIRE_RATE_COEF_PORT = "FireRateCoef";
		
	static const float MIN_FIRE_RATE_COEF = 0.05;
	static const float MAX_FIRE_RATE_COEF = 2;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Fire pattern used to calculate fire times", "", ParamEnumArray.FromEnum(EAIFirePattern))]
	protected EAIFirePattern m_iFirePattern;
		
	// Component refs
	protected SCR_AICombatComponent m_CombatComponent;
	protected SCR_AIConfigComponent m_ConfigComponent;
	
	// Weapon config
	protected ref SCR_AIWeaponTypeHandlingConfig m_WeaponConfig;
	protected float m_fShotSpan = 0.09;
	protected int m_fBurstSize = 1;
		
	// Weapon refs for detecting relevant changes
	protected BaseWeaponComponent m_LastWeapon;
	protected EWeaponType m_eLastWeaponType;	
	protected BaseMuzzleComponent m_LastMuzzle;
	protected BaseFireMode m_LastFireMode;
	
	// Is node valid for simulation
	protected bool m_bValid;

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{	
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (!utility)
			return;
		
		m_CombatComponent = utility.m_CombatComponent;
		m_ConfigComponent = utility.m_ConfigComponent;
		
		if (!m_CombatComponent || !m_ConfigComponent)
			return;
		
		// Mark valid to simulate
		m_bValid = true;
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_FIRE_TIME,
		PORT_STABILIZATION_TIME,
		PORT_REJECT_TIME,
		PORT_SHOT_SPAN
	};
    override array<string> GetVariablesOut() { return s_aVarsOut; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		TARGET_ENTITY_PORT,
		TARGET_POSITION_PORT,
		FIRE_RATE_COEF_PORT
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	// Returns currently used fire rate factor
	float GetFireRateFactor()
	{
		// Get our internal fire rate coef
		float fireRateCoef;
		if (!GetVariableIn(FIRE_RATE_COEF_PORT, fireRateCoef))
			fireRateCoef = 1;
		
		// If fire rate is overriden in agent combat component (external API), we use user value, not our intenal one
		float agentFRC = m_CombatComponent.GetFireRateCoef();
		if (agentFRC != 1)
			fireRateCoef = agentFRC;
		
		return 1 - Math.Clamp(fireRateCoef, MIN_FIRE_RATE_COEF, MAX_FIRE_RATE_COEF);	
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		// Get agent's character entity
		IEntity ownerEntity = owner.GetControlledEntity();
		
		// Get current weapon
		BaseWeaponComponent weapon = m_CombatComponent.GetCurrentWeapon();
		if (!weapon || !ownerEntity || !m_bValid)
			return ENodeResult.FAIL;

		// Get current muzzle and fire rate
		BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
		if (!muzzle)
			return ENodeResult.FAIL;
		
		BaseFireMode fireMode = muzzle.GetCurrentFireMode();
		if (!fireMode)
			return ENodeResult.FAIL;
		
		// Update config if something relevant changed
		if (weapon != m_LastWeapon || muzzle != m_LastMuzzle || fireMode != m_LastFireMode)
		{
			// Update weapon type
			m_eLastWeaponType = SCR_AIWeaponHandling.GetWeaponType(weapon, true);
			
			// Update weapon config
			m_WeaponConfig = m_ConfigComponent.GetWeaponTypeHandlingConfig(m_eLastWeaponType);
			
			// Weapon data
			m_fShotSpan = fireMode.GetShotSpan();
			m_fBurstSize = fireMode.GetBurstSize();
			
			// Update cache
			m_LastWeapon = weapon;
			m_LastMuzzle = muzzle;
			m_LastFireMode = fireMode;
		}	
				
		// Get fire pattern
		EAIFirePattern pattern = m_iFirePattern;
		
		// Get target pos
		vector targetPos;
		IEntity targetEntity;
		if (GetVariableIn(TARGET_ENTITY_PORT, targetEntity))
			targetPos = targetEntity.GetOrigin();
		else 
			GetVariableIn(TARGET_POSITION_PORT, targetPos);
		
		// We assume mid range to target
		float distToTarget = 250;
		// Get real distance if available
		if (targetPos != vector.Zero)
			distToTarget = vector.Distance(ownerEntity.GetOrigin(), targetPos);

		// Get config values
		float stabilizationTime = m_WeaponConfig.m_fBaseStabilizationTime;
		float rejectionTime = m_WeaponConfig.m_fBaseRejectionTime;				
		
		// CQC Factor
		float cqcFactor;
		if (distToTarget < SCR_AICombatComponent.CLOSE_RANGE_COMBAT_DISTANCE)
			cqcFactor = Math.Map(distToTarget, 5, SCR_AICombatComponent.CLOSE_RANGE_COMBAT_DISTANCE, 1, 0);
		
		// Use weapon burst size as default
		float burstSize = m_fBurstSize;
		
		// Burst or suppress fire
		if (pattern == EAIFirePattern.Burst || pattern == EAIFirePattern.Suppress)
		{
			// Override weapon burst size
			if (burstSize <= 1)
			{
				// It's a machine gun!
				if (m_eLastWeaponType == EWeaponType.WT_MACHINEGUN)
				{
					float range = 2.6;
					float base = 1.75;
					
					// Suppressive fire for mg
					if (pattern == EAIFirePattern.Suppress)
					{
						range = 3.5;
						base = 2.2;
					}
					
					stabilizationTime += Math.RandomGaussFloat(0.236, 0);
					burstSize = base + Math.Pow(Math.RandomFloat(0, range) - 0.4, 2);
				}
				else
				{
					float range = 1.77;
					
					// Suppressive fire for rifles
					if (pattern == EAIFirePattern.Suppress)
						range = 1.96;
					
					burstSize = 0.75 + Math.Pow(Math.RandomFloat(0, range) - 0.3, 3);
				}
			}
		}
		// SemiAuto/Single fire - default
		else
		{
			burstSize = Math.RandomFloat(0.1, m_fShotSpan * 1.9);
			stabilizationTime += Math.RandomGaussFloat(0.236, 0);
		}
		
		// Apply CQC factor to burst size
		burstSize -= Math.RandomGaussFloat(0.32, 0) * cqcFactor;
		
		if (stabilizationTime > 0)
			stabilizationTime -= Math.RandomFloat(stabilizationTime / 3, stabilizationTime) * cqcFactor * 0.85;
		else
			stabilizationTime = 0;
				
		// Ger fire time
		float fireTime = m_fShotSpan * burstSize;			
		
		// Get Agent/Group fire rate factor	
		float fireRateFactor = GetFireRateFactor();
	
		// Apply fire rate
		fireTime += (fireTime / 2.5) * fireRateFactor; 
		stabilizationTime += (stabilizationTime * 1.05) * fireRateFactor;
		
		// Fire rate factor in MG stabilization
		if (m_eLastWeaponType == EWeaponType.WT_MACHINEGUN)
			stabilizationTime += Math.AbsFloat(Math.RandomGaussFloat(0.12, 0)) * fireRateFactor;
			
		// Stabilization range factor
		vector stabTimeRandRange = Vector(0.1, 0, 0.7);
		
		// Mid or long range
		if (distToTarget > SCR_AICombatComponent.CLOSE_RANGE_COMBAT_DISTANCE)
		{
			// Add factors of mid range
			stabTimeRandRange += Vector(0.5, 0, 0.7);

			// Add factors of long range
			if (distToTarget > SCR_AICombatComponent.LONG_RANGE_COMBAT_DISTANCE)
				stabTimeRandRange += Vector(0.7, 0, 1.5);
		}
			
		// Distance is main factor in stabilization time
		stabilizationTime *= Math.RandomFloat(stabTimeRandRange[0], stabTimeRandRange[2]);
		
		// Make sure it's always some time
		stabilizationTime = Math.Max(stabilizationTime, 0.05);
		
		SetVariableOut(PORT_FIRE_TIME, fireTime);
		SetVariableOut(PORT_STABILIZATION_TIME, stabilizationTime);
		SetVariableOut(PORT_REJECT_TIME, rejectionTime);
		SetVariableOut(PORT_SHOT_SPAN, m_fShotSpan);
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
    static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription() {return "Get all time constants for fire - it is weapon dependent";}
};

