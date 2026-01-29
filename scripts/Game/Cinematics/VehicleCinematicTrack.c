[CinematicTrackAttribute(name:"Vehicle Track", description:"Allows you to control vehicles")]
class VehicleCinematicTrack : CinematicTrackBase
{
	[Attribute("false")]
	bool m_bStartEngine;
	
	[Attribute("1.0", params:"0 100000")]
	float m_fEfficiency;
	
	[Attribute("0.0", params:"-1.0 1.0")]
	float m_fSteering;
	
	[Attribute("2", params: "0 5")]
	int m_fGear;
	
	[Attribute("1.0", params: "0.0 1.0")]
	float m_fClutch;
	
	[Attribute("0.0", params:"0.0 1.0")]
	float m_fBreak;
	
	[Attribute("false")]
	bool m_bHandBrake;
	
	[Attribute("false")]
	bool m_bLightsOn;
	
	[Attribute("0.0", params:"0.0 1.0")]
	float m_fThrottle;
	
	[Attribute("false")]
	bool m_bTurretControl;
	
	[Attribute("false")]
	bool m_bTurretShot;
	
	[Attribute("0.0", params: "-3 3")]
	float m_fTurretLeftRight;
	
	[Attribute("0.0", params: "-3 3")]
	float m_fTurretDownUp;
	
	private GenericEntity m_Vehicle;
	private CarControllerComponent m_Vehicle_c;
	private VehicleWheeledSimulation m_Vehicle_s;
	private BaseLightManagerComponent m_Vehicle_l;
	private World actualWorld;
	
	private SlotManagerComponent slotManager;
	private SlotManagerComponent secondarySlotManager;
	
	private int lastGear = 0;
	
	[CinematicEventAttribute()]
	void insertGunner()
	{
		if (slotManager)
		{
			array<EntitySlotInfo> vehicleSlotInfos = {};
			slotManager.GetSlotInfos(vehicleSlotInfos);
			
			foreach (EntitySlotInfo slotInfo : vehicleSlotInfos)
			{
			    IEntity slotEntity = slotInfo.GetAttachedEntity();
			
			    if (!slotEntity)
			        continue;
			
			    TurretControllerComponent turret = TurretControllerComponent.Cast(slotEntity.FindComponent(TurretControllerComponent));
				
				if (turret)
				{
					BaseCompartmentSlot compartment = turret.GetCompartmentSlot();
					if (compartment)
					{
						if (!compartment.IsOccupied())
						{
							AIGroup group;
							compartment.SpawnDefaultCharacterInCompartment(group);
						}
					}
				}
			}
		}
	}
	
	override void OnInit(World world)
	{
		
		actualWorld = world;
		
		// Find vehicle entity by using name of track
		findEntity(world);
		
		if (m_Vehicle) {
		
			// Find vehicle control and simulation components
			m_Vehicle_c = CarControllerComponent.Cast(m_Vehicle.FindComponent(CarControllerComponent));
			m_Vehicle_l = BaseLightManagerComponent.Cast(m_Vehicle.FindComponent(BaseLightManagerComponent));
			m_Vehicle_s = VehicleWheeledSimulation.Cast(m_Vehicle.FindComponent(VehicleWheeledSimulation));
			slotManager = SlotManagerComponent.Cast(m_Vehicle.FindComponent(SlotManagerComponent));
		}
	}
	
	void findEntity(World world)
	{
		// Find particle entity by using name of track
		TStringArray strs = new TStringArray;
		GetTrackName().Split("_", strs, true);
		
		m_Vehicle = GenericEntity.Cast(world.FindEntityByName(strs.Get(0)));
	}
	
	override void OnApply(float time)
	{
		
		if (m_Vehicle && m_Vehicle_c && m_Vehicle_s)
		{
			
			VehicleAnimationComponent chanim = VehicleAnimationComponent.Cast(m_Vehicle.FindComponent(VehicleAnimationComponent));
			int id = chanim.BindFloatVariable("VehicleSteering");
			chanim.SetFloatVariable(id,m_fSteering);
						
			//zeroes sentsitivities
			m_Vehicle_s.SetNoiseSteerSensitivity(0);
			m_Vehicle_s.SetRoughnessSensitivity(0);		
			m_Vehicle_c.SetPersistentHandBrake(0);
			m_Vehicle_s.GearboxSetEfficiencyState(m_fEfficiency);
			
			
			if(m_Vehicle_l)
			{
				m_Vehicle_l.SetLightsState(ELightType.Head, m_bLightsOn, 0);
				m_Vehicle_l.SetLightsState(ELightType.Head, m_bLightsOn, 1);	
			}
			
			//Start or stop engine
			if (m_bStartEngine && !m_Vehicle_c.IsEngineOn())
			{			
				m_Vehicle_c.ForceStartEngine();
				
			} else if (!m_bStartEngine && m_Vehicle_c.IsEngineOn())  {
				
				m_Vehicle_c.ForceStopEngine();
			}
			
				 
			//Break and hand-brake
			m_Vehicle_s.SetBreak(m_fBreak, m_bHandBrake);
				
			//Ggear
			if(m_fGear != lastGear)
			{
				m_Vehicle_s.SetGear(m_fGear);
				lastGear = m_fGear;
			}
				
			//Clutch
			m_Vehicle_s.SetClutch(m_fClutch);
				
			//Throttle
		 	m_Vehicle_s.SetThrottle(m_fThrottle);
				
			//Steering
			m_Vehicle_s.SetSteering(m_fSteering);
			
			
			if (slotManager && m_bTurretControl)
			{
				array<EntitySlotInfo> vehicleSlotInfos = {};
				slotManager.GetSlotInfos(vehicleSlotInfos);
				
				foreach (EntitySlotInfo slotInfo : vehicleSlotInfos)
				{
				    IEntity slotEntity = slotInfo.GetAttachedEntity();
				
				    if (!slotEntity)
				        continue;
				
				    TurretControllerComponent turret = TurretControllerComponent.Cast(slotEntity.FindComponent(TurretControllerComponent));
					
					if (turret)
					{
						
						turret.SetAimingAngles(m_fTurretLeftRight,m_fTurretDownUp);
					
					    	if (m_bTurretShot)
					    		turret.SetFireWeaponWanted(true);
							else
								turret.SetFireWeaponWanted(false);
					
					} else {
					
						//second layer of slots
						secondarySlotManager = SlotManagerComponent.Cast(slotEntity.FindComponent(SlotManagerComponent));
						
						if (secondarySlotManager)
						{
							array<EntitySlotInfo> secondaryVehicleSlotInfos = {};
							secondarySlotManager.GetSlotInfos(secondaryVehicleSlotInfos);
							
							foreach (EntitySlotInfo secondarySlotInfo : secondaryVehicleSlotInfos)
							{
								IEntity secondarySlotEntity = secondarySlotInfo.GetAttachedEntity();
								
								if (!secondarySlotEntity)
				        			continue;
								
								TurretControllerComponent secondaryTurret = TurretControllerComponent.Cast(secondarySlotEntity.FindComponent(TurretControllerComponent));
								
								if (secondaryTurret)
								{
									secondaryTurret.SetAimingAngles(m_fTurretLeftRight,m_fTurretDownUp);
									
									if (m_bTurretShot)
				    					secondaryTurret.SetFireWeaponWanted(true);
								}
							}
						}
					}
					
				}
			}
		}
	}
}