class SCR_AIIdleBehavior_Driver : SCR_AIIdleBehavior
{
	protected const int LIGHTS_CHECK_INTERVAL_MS = 2*1000;
	protected const int UNSAFE_LIGHTS_LOCK_DURATION_MS = 5 * 60 * 1000; // For how long since feeling unsafe we don't want to enable lights
	
	protected const float MIN_HIBEAM_LIGHTS_VEHICLE_SPEED_MPS_SQ = 4; // Min speed in sq m/s to enable hi beam lights
	
	protected const int VEHICLE_DANGER_HIBEAM_LIGHTS_TIMEOUT_MS = 5*1000; // For how long since last vehicle danger event we don't use hi beam lights
	
	protected SCR_AIInfoComponent m_Info;
	protected IEntity m_Vehicle;
	protected BaseLightManagerComponent m_VehicleLightManager;
	
	protected float m_fNextLightsCheck;
	
	protected float m_fHiBeamIgnoreTime;
	
	void SCR_AIIdleBehavior_Driver(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, notnull IEntity vehicleEntity)
	{
		if (!utility)
			return;
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/VehicleInCombat_DriverListener.bt";
		SetPriority(PRIORITY_BEHAVIOR_IDLE_DRIVER);
		m_bUseCombatMove = true;
		
		m_Vehicle = vehicleEntity;
		m_VehicleLightManager = BaseLightManagerComponent.Cast(m_Vehicle.FindComponent(BaseLightManagerComponent));
		m_Info = utility.m_AIInfo;
	}

	//------------------------------------------------------------------------------------------------	
	override void OnFail()
	{
		super.OnFail();
		
		// Disable lights when exiting
		if (m_Utility.GetOwner().IsAIActivated() && IsLightInteractionAllowed())
			SetLightsState(false);
	}
	
	//------------------------------------------------------------------------------------------------
	// Called by SCR_AIDangerReaction_Vehicle each time we have car approaching
	void OnVehicleApproaching()
	{
		m_fHiBeamIgnoreTime = GetGame().GetWorld().GetWorldTime() + VEHICLE_DANGER_HIBEAM_LIGHTS_TIMEOUT_MS;
		
		// Turn off hi beams immediately
		if (GetHiBeamLightsState() && IsLightInteractionAllowed())
			SetHiBeamLightState(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EvaluateLightsUsage()
	{
		// Disable hazard lights on first evaluation
		if (m_fNextLightsCheck == 0 && IsLightInteractionAllowed())
			m_VehicleLightManager.SetLightsState(ELightType.Hazard, false);
		
		float worldTime = GetGame().GetWorld().GetWorldTime();
		if (worldTime < m_fNextLightsCheck)
			return;
		
		// Do nothing if we are forbidden to touch lights
		if (!IsLightInteractionAllowed())
			return;
		
		m_fNextLightsCheck = worldTime + LIGHTS_CHECK_INTERVAL_MS * Math.RandomFloat(0.8, 1.2);
		bool useLights = ShouldUseLights(worldTime);
		bool useHiBeams;
		
		if (useLights)
			useHiBeams = ShouldUseHiBeamLights(worldTime);
		
		if (GetLightsState() != useLights || GetHiBeamLightsState() != useHiBeams)
			SetLightsState(useLights, useHiBeams);
	}
	
	//------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (m_VehicleLightManager)
			EvaluateLightsUsage();
		
		return GetPriority();
	}
	
	
	//------------------------------------------------------------------------------------------------------------------------------
	// Auxiliary methods related to lights
	
	//------------------------------------------------------------------------------------------------
	protected bool ShouldUseHiBeamLights(float worldTime)
	{
		if (m_fHiBeamIgnoreTime > worldTime)
			return false;
		
		Physics ph = m_Vehicle.GetPhysics();
		if (!ph)
			return false;
		
		return ph.GetVelocity().LengthSq() > MIN_HIBEAM_LIGHTS_VEHICLE_SPEED_MPS_SQ;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ShouldUseLights(float worldTime)
	{
		if (!SCR_AIWorldHandling.IsLowLightEnvironment())
			return false;

		if (m_Info.GetThreatState() != EAIThreatState.SAFE)
		{
			// We're feeling unsafe, lock lights at disabled state for set duration
			m_fNextLightsCheck = worldTime + UNSAFE_LIGHTS_LOCK_DURATION_MS;
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsLightInteractionAllowed()
	{
		if (!m_Utility.m_SettingsComponent)
			return true;
		
		SCR_AICharacterLightInteractionSettingBase setting = SCR_AICharacterLightInteractionSettingBase.Cast(m_Utility.m_SettingsComponent.GetCurrentSetting(SCR_AICharacterLightInteractionSettingBase));
		if (!setting)
			return true;
		
		return setting.IsLightInterractionAllowed();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetHiBeamLightsState()
	{
		if (!m_VehicleLightManager)
			return false;
		
		return m_VehicleLightManager.GetLightsState(ELightType.HiBeam);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetLightsState()
	{		
		if (!m_VehicleLightManager)
			return false;
		
		return m_VehicleLightManager.GetLightsState(ELightType.Head);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetHiBeamLightState(bool enable)
	{
		if (!m_VehicleLightManager)
			return;
	
		m_VehicleLightManager.SetLightsState(ELightType.HiBeam, enable);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetLightsState(bool enableLights, bool enableHiBeamLights = false)
	{
		if (!m_VehicleLightManager)
			return;
		
		m_VehicleLightManager.SetLightsState(ELightType.Head, enableLights);
		SetHiBeamLightState(enableLights && enableHiBeamLights);
	}
}