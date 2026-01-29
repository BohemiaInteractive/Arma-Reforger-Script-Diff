[BaseContainerProps()]
class SCR_TutorialLogic_Heli : SCR_BaseTutorialCourseLogic
{
	protected const float TIME_FOR_ANGLE_WARNING = 3000;
	protected const float TIME_FOR_ALTITUDE_WARNING = 2500;
	
	protected SCR_TutorialGamemodeComponent m_TutorialGamemode;
	protected Vehicle m_CourseHelicopter;
	protected SCR_HelicopterControllerComponent m_HelicopterController;
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected float m_fAngleTimer, m_fAltitudeTimer;
	protected VehicleHelicopterSimulation m_HelicopterSimulation;
	bool m_bForceAutohover, m_bCheckAngle, m_bCheckAltitude;
	
	//------------------------------------------------------------------------------------------------
	protected void RotorCheck()
	{
		if (!m_CourseHelicopter)
			return;
		
		if (!m_HelicopterSimulation)
			m_HelicopterSimulation = VehicleHelicopterSimulation.Cast(m_CourseHelicopter.FindComponent(VehicleHelicopterSimulation));
		
		if (!m_HelicopterSimulation)
			return;
		
		if (m_HelicopterSimulation.RotorGetState(0) == RotorState.DESTROYED || m_HelicopterSimulation.RotorGetState(1) == RotorState.DESTROYED)
			m_TutorialGamemode.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HelicopterDrownedCheck()
	{
		if (!m_CourseHelicopter)
			return;
		
		VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_CourseHelicopter.FindComponent(VehicleControllerComponent));
		if (!vehicleController)
			return;
		
		if (!vehicleController.GetEngineDrowned())
			return;
		
		m_TutorialGamemode.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PrepareDamageInvokers()
	{
		if (!m_CourseHelicopter)
			return;
		
		//general helicopter damage check
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_CourseHelicopter.GetDamageManager());
		if (damageManager)
			damageManager.GetOnDamageStateChanged().Insert(OnHelicopterDamaged);
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_CourseHelicopter.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		IEntity rotor;
		EntitySlotInfo slot = slotManager.GetSlotByName("RotorMain");
		if (slot)
		{
			rotor = slot.GetAttachedEntity();
			SCR_RotorDamageManagerComponent rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			rotorDmgComp.GetOnDamageStateChanged().Insert(OnHelicopterDamaged);
		}
		
		slot = slotManager.GetSlotByName("RotorTail");
		if (slot)
		{
			rotor = slot.GetAttachedEntity();
			SCR_RotorDamageManagerComponent rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			rotorDmgComp.GetOnDamageStateChanged().Insert(OnHelicopterDamaged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveHelicopterInvokers()
	{
		if (!m_CourseHelicopter)
			return;
		
		//General damage invoker
		SCR_VehicleDamageManagerComponent heliDamageManager = SCR_VehicleDamageManagerComponent.Cast(m_CourseHelicopter.GetDamageManager());
		if (heliDamageManager)
			heliDamageManager.GetOnDamageStateChanged().Remove(OnHelicopterDamaged);
		
		//Rotor damage invoker
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(m_CourseHelicopter.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;
		
		EntitySlotInfo slot = slotManager.GetSlotByName("RotorMain");
		if (slot)
		{
			IEntity rotor = slot.GetAttachedEntity();
			if (!rotor)
				return;
			
			SCR_RotorDamageManagerComponent rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			if (!rotorDmgComp)
				return;
			
			rotorDmgComp.GetOnDamageStateChanged().Remove(OnHelicopterDamaged);
		}
		
		slot = slotManager.GetSlotByName("RotorTail");
		if (slot)
		{
			IEntity rotor = slot.GetAttachedEntity();
			if (!rotor)
				return;
			
			SCR_RotorDamageManagerComponent rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			if (!rotorDmgComp)
				return;
			
			rotorDmgComp.GetOnDamageStateChanged().Remove(OnHelicopterDamaged);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHelicopterDamaged(EDamageState state)
	{	
		if (state == EDamageState.DESTROYED)
			return;
		
		m_TutorialGamemode.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ContinuousCheck()
	{
		if (!m_CourseHelicopter || !m_HelicopterController || !m_SignalsManagerComponent)
			m_TutorialGamemode.SetCourseConfig(SCR_ETutorialCourses.FREE_ROAM);
		
		if (m_bForceAutohover && !m_HelicopterController.GetAutohoverEnabled())
		{
			if (m_TutorialGamemode.GetCurrentStage().Type() != SCR_Tutorial_Heli_AUTOHOVER_ON)
			{
				m_TutorialGamemode.InsertStage("TURN_AUTOHOVER", m_TutorialGamemode.GetActiveStageIndex());
				m_TutorialGamemode.SetStage(m_TutorialGamemode.GetActiveStageIndex());
			}
		}
		
		if (m_bCheckAngle)
		{
			float angle = m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("PitchAngle"));
			float roll = m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("RollAngle"));
			
			if ((angle <= -60 || angle >= 50) || (roll <= -45 || roll >= 45))
			{
				if (m_fAngleTimer == 0)
					m_fAngleTimer = GetGame().GetWorld().GetWorldTime();
				
				if (((m_fAngleTimer + TIME_FOR_ANGLE_WARNING) < GetGame().GetWorld().GetWorldTime()) && (m_TutorialGamemode.GetCurrentStage().Type() != SCR_Tutorial_Heli_DANGER_ANGLE))
				{
					m_TutorialGamemode.InsertStage("Danger_ANGLE", m_TutorialGamemode.GetActiveStageIndex());
					m_TutorialGamemode.SetStage(m_TutorialGamemode.GetActiveStageIndex());
				}
			}
			else
			{
				m_fAngleTimer = 0;
			}
		}
		
		if (m_bCheckAltitude)
		{
			if (m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("AltitudeAGL")) < 10)
			{
				if (m_fAltitudeTimer == 0)
					m_fAltitudeTimer = GetGame().GetWorld().GetWorldTime();
				
				if (((m_fAltitudeTimer + TIME_FOR_ALTITUDE_WARNING) < GetGame().GetWorld().GetWorldTime()) && (m_TutorialGamemode.GetCurrentStage().Type() != SCR_Tutorial_Heli_DANGER_ALTITUDE))
				{
					m_TutorialGamemode.InsertStage("DANGER_ALTITUDE", m_TutorialGamemode.GetActiveStageIndex());
					m_TutorialGamemode.SetStage(m_TutorialGamemode.GetActiveStageIndex());
				}
			}
			else
			{
				m_fAltitudeTimer = 0;
			}
		}
		
		HelicopterDrownedCheck();
		RotorCheck();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		m_TutorialGamemode = SCR_TutorialGamemodeComponent.GetInstance();
		if (!m_TutorialGamemode)
			return;
		
		m_CourseHelicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!m_CourseHelicopter)
			return;
		
		ChimeraCharacter copilot = ChimeraCharacter.Cast(GetGame().GetWorld().FindEntityByName("HELICOPTER_Copilot"));
		if (copilot)
			m_TutorialGamemode.MoveCharacterIntoVehicle(copilot, m_CourseHelicopter, ECompartmentType.PILOT, 1);
		
		PrepareDamageInvokers();
		
		m_HelicopterController = SCR_HelicopterControllerComponent.Cast(m_CourseHelicopter.FindComponent(SCR_HelicopterControllerComponent));
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(m_CourseHelicopter.FindComponent(SignalsManagerComponent));
		
		GetGame().GetCallqueue().CallLater(ContinuousCheck, 500, true);	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		if (m_TutorialGamemode && m_CourseHelicopter)
			m_TutorialGamemode.ChangeVehicleLockState(m_CourseHelicopter, true);
		
		m_bCheckAngle = false;
		m_bCheckAltitude = false;
		m_bForceAutohover = false;
		
		m_fAngleTimer = 0;
		m_fAltitudeTimer = 0;
		
		GetGame().GetCallqueue().Remove(ContinuousCheck);
		RemoveHelicopterInvokers();
	}
}