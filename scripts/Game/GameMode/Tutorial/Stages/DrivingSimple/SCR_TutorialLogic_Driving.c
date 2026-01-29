[BaseContainerProps()]
class SCR_TutorialLogic_Driving : SCR_BaseTutorialCourseLogic
{
	protected Vehicle m_CourseVehicle;
	
	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (tutorial)
			tutorial.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHitZoneDamageStateChanged(SCR_HitZone hitzone)
	{
		if (hitzone.GetDamageState() != EDamageState.DESTROYED)
			return;
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (tutorial)
			tutorial.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDamaged(EDamageState state)
	{	
		if (state != EDamageState.DESTROYED)
			return;
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (tutorial)
			tutorial.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEngineStoppedJeep()
	{
		if (!m_CourseVehicle)
			return;
		
		VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_CourseVehicle.FindComponent(VehicleControllerComponent));
		if (!vehicleController || !vehicleController.GetEngineDrowned())
			return;
		
		vehicleController.GetOnEngineStop().Remove(OnEngineStoppedJeep);
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (tutorial)
			tutorial.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		m_CourseVehicle = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("SmallJeep")); 
		
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_CourseVehicle.GetDamageManager());
		if (damageManager)
		{
			damageManager.GetOnDamageStateChanged().Insert(OnVehicleDamaged);
		
			SCR_HitZone engine = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
		
			if (engine)
				engine.GetOnDamageStateChanged().Insert(OnHitZoneDamageStateChanged);
		
			SCR_HitZone gearbox = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
		
			if (gearbox)
				gearbox.GetOnDamageStateChanged().Insert(OnHitZoneDamageStateChanged);
		}
			
		VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_CourseVehicle.FindComponent(VehicleControllerComponent));
		if (vehicleController)
			vehicleController.GetOnEngineStop().Insert(OnEngineStoppedJeep);
		
		IEntity instructor = GetGame().GetWorld().FindEntityByName("DRIVING_Copilot");
		if (!instructor)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(instructor.FindComponent(SCR_CompartmentAccessComponent));
		if (compartmentAccess)
			compartmentAccess.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
	};
	
	//------------------------------------------------------------------------------------------------
	void HandleOldAssets()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		IEntity oldEnt = GetGame().GetWorld().FindEntityByName("DRIVING_Copilot");
		if (oldEnt)
		{
			oldEnt.SetName(string.Empty);
			tutorial.InsertIntoGarbage(oldEnt);
			
			SCR_TutorialInstructorComponent instructorComp = SCR_TutorialInstructorComponent.Cast(oldEnt.FindComponent(SCR_TutorialInstructorComponent));
			if (instructorComp)
				instructorComp.EnableCourse(false);
		}
		
		oldEnt = GetGame().GetWorld().FindEntityByName("SmallJeep");
		if (oldEnt)
		{
			oldEnt.SetName(string.Empty);
			tutorial.InsertIntoGarbage(oldEnt);
			tutorial.ChangeVehicleLockState(oldEnt, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		IEntity instructor = GetGame().GetWorld().FindEntityByName("DRIVING_Copilot");
		if (instructor)
		{
			SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(instructor.FindComponent(SCR_CompartmentAccessComponent));
			if (compartmentAccess)
				compartmentAccess.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
		}
		
		if (!m_CourseVehicle)
			return;
		
		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_CourseVehicle.GetDamageManager());
		if (damageManager)
		{
			damageManager.GetOnDamageStateChanged().Remove(OnVehicleDamaged);
		
			SCR_HitZone engine = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Engine_01"));
			if (engine)
				engine.GetOnDamageStateChanged().Remove(OnHitZoneDamageStateChanged);
		
			SCR_HitZone gearbox = SCR_HitZone.Cast(damageManager.GetHitZoneByName("Gearbox_01"));
			if (gearbox)
				gearbox.GetOnDamageStateChanged().Remove(OnHitZoneDamageStateChanged);
		}
		
		VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_CourseVehicle.FindComponent(VehicleControllerComponent));
		if (vehicleController)
			vehicleController.GetOnEngineStop().Remove(OnEngineStoppedJeep);
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		if (tutorial.CanBreakCourse())
			HandleOldAssets();
	};
}