[ComponentEditorProps(category: "GameScripted/Component", description: "")]
class SCR_HelicopterCinematicFlyComponentClass : ScriptComponentClass
{
}

class SCR_HelicopterCinematicFlyComponent : ScriptComponent
{
	[Attribute(defvalue: "2", desc: "A force multiplier used on a helicopter.")]
	float m_fForceMultiplier;
	
	[Attribute(defvalue: "10", desc: "A force multiplier used on a helicopter with it's initial kick to keep the direction.")]
	float m_fInitialForceMultiplier;

	[Attribute(defvalue: "2000", desc: "Delay (ms) between two pulses.")]
	int m_iPulseDelay;

	DamageManagerComponent m_DamageManager;

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_DamageManager = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));
		
		VehicleHelicopterSimulation vehicleSimulation = VehicleHelicopterSimulation.Cast(GetOwner().FindComponent(VehicleHelicopterSimulation));
		vehicleSimulation.EngineStart();
		vehicleSimulation.SetThrottle(1);
		vehicleSimulation.RotorSetForceScaleState(0, 2);
		vehicleSimulation.RotorSetForceScaleState(1, 1);
		
		Kick(true);
		
		GetGame().GetCallqueue().CallLater(Kick, m_iPulseDelay, true, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void Kick(bool initialKick = false)
	{
		if (m_DamageManager.IsDestroyed())
		{
			GetGame().GetCallqueue().Remove(Kick);
			return;
		}
		
		float forceMultiplier;
		
		if (initialKick)
			forceMultiplier = m_fForceMultiplier * m_fInitialForceMultiplier; 
		else 
			forceMultiplier = m_fForceMultiplier;
		
		vector velOrig = GetOwner().GetPhysics().GetVelocity();
		vector rotVector = GetOwner().GetAngles();
		vector vel = {velOrig[0] + Math.Sin(rotVector[1] * Math.DEG2RAD) * forceMultiplier, velOrig[1], velOrig[2] + Math.Cos(rotVector[1] * Math.DEG2RAD) * forceMultiplier };
		GetOwner().GetPhysics().SetVelocity(vel);
	}
}
