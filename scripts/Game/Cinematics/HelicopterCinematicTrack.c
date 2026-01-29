[CinematicTrackAttribute(name:"Helicopter Track", description:"Allows you to control helicopters")]
class HelicopterCinematicTrack : CinematicTrackBase
{
	[Attribute("false")]
	bool m_bStartEngine;
	
	[Attribute("1.0", params:"0 100000")]
	float m_fThrottle;
	
	[Attribute("2.0", params:"0 100000")]
	float m_fMainRotorForce;
	
	[Attribute("2.0", params:"0 100000")]
	float m_fTailRotorForce;
	
	[Attribute("40.0", params:"0 50")]
	float m_fForwardIntensity;
	
	[Attribute("false")]
	bool m_bshootTurret;
	
	[Attribute("")]
	string m_sTurretInSlot;
	
	private GenericEntity m_Helicopter;
	private VehicleHelicopterSimulation m_Helicopter_s;
	private BaseLightManagerComponent m_Helicopter_l;
	private World actualWorld;
	private SlotManagerComponent slotManager;
	private EntitySlotInfo entityInSlot;
	private IEntity m_SlotEntity;
	private TurretControllerComponent turret;
	
	
	[CinematicEventAttribute()]
	void pushForward()
	{
		if (m_Helicopter && m_Helicopter_s)
		{
			vector velOrig = m_Helicopter.GetPhysics().GetVelocity();
			vector rotVector = m_Helicopter.GetAngles();
			vector vel = {velOrig[0] + Math.Sin(rotVector[1] * Math.DEG2RAD) * m_fForwardIntensity, velOrig[1], velOrig[2] + Math.Cos(rotVector[1] * Math.DEG2RAD) * m_fForwardIntensity };
			m_Helicopter.GetPhysics().SetVelocity(vel);
		}
	}
	
	override void OnInit(World world)
	{
		
		actualWorld = world;
		
		// Find vehicle entity by using name of track
		findEntity(world);
		
		if (m_Helicopter) {
		
			// Find vehicle control and simulation components
			m_Helicopter_s = VehicleHelicopterSimulation.Cast(m_Helicopter.FindComponent(VehicleHelicopterSimulation));
		}
	}
	
	void findEntity(World world)
	{
		// Find particle entity by using name of track
		TStringArray strs = new TStringArray;
		GetTrackName().Split("_", strs, true);
		
		m_Helicopter = GenericEntity.Cast(world.FindEntityByName(strs.Get(0)));
	}
	
	override void OnApply(float time)
	{
		
		if (m_Helicopter && m_Helicopter_s)
		{
			m_Helicopter_s.EngineStart();
			m_Helicopter_s.SetThrottle(m_fThrottle);
			m_Helicopter_s.RotorSetForceScaleState(0, m_fMainRotorForce);
			m_Helicopter_s.RotorSetForceScaleState(1, m_fTailRotorForce);
		}		
		
		slotManager = SlotManagerComponent.Cast(m_Helicopter.FindComponent(SlotManagerComponent));
				
		if (!slotManager)
			return;
		
		if (m_sTurretInSlot == "")
			return;
		
		entityInSlot = slotManager.GetSlotByName(m_sTurretInSlot); //M260CarrierVirtual
			
		if(!entityInSlot)
			return;
		
		m_SlotEntity = entityInSlot.GetAttachedEntity();
				
		if(!m_SlotEntity)
			return;
		
		turret = TurretControllerComponent.Cast(m_SlotEntity.FindComponent(TurretControllerComponent));
					
		if (!turret)
			return;
		
		if (m_bshootTurret)
			turret.SetFireWeaponWanted(true);
		else
			turret.SetFireWeaponWanted(false);
	}
}