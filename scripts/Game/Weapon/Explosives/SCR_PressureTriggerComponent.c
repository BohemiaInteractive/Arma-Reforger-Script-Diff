[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "")]
class SCR_PressureTriggerComponentClass : SCR_BaseTriggerComponentClass
{
	[Attribute("10", desc: "Min. weight that can set off this trigger when applied to it. [kg]")]
	protected float m_fMinWeight;

	//------------------------------------------------------------------------------------------------
	float GetMinWeight()
	{
		return m_fMinWeight;
	}
}

class SCR_PressureTriggerComponent : SCR_BaseTriggerComponent
{
	protected const float MIN_DELAY = 125; //time between explosion tries in ms
	protected float m_fLastTryTime = 0;
	
	//------------------------------------------------------------------------------------------------
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (GetGame().GetWorld().GetWorldTime() - m_fLastTryTime < MIN_DELAY)
			return;
		
		Physics otherPhysics = other.GetPhysics();
		if (!otherPhysics)
			return;

	 	float otherMass = otherPhysics.GetMass();

		Vehicle vehicle = Vehicle.Cast(other);
		if (vehicle)
		{
			VehicleWheeledSimulation vehicleSimulation = VehicleWheeledSimulation.Cast(vehicle.FindComponent(VehicleWheeledSimulation));
			if (vehicleSimulation)
				otherMass /= vehicleSimulation.WheelCount(); // assume it's a vehicle and assume min. weight it lays on the trigger is weight / wheels count
		}
		
		m_fLastTryTime = GetGame().GetWorld().GetWorldTime();

		SCR_PressureTriggerComponentClass data = SCR_PressureTriggerComponentClass.Cast(GetComponentData(owner));
		if (!data)
			return;

		if (otherMass < data.GetMinWeight())
			return; // Too light, won't set the trigger off
		
		BaseTriggerComponent baseTriggerComponent = BaseTriggerComponent.Cast(GetOwner().FindComponent(BaseTriggerComponent));
		if (!baseTriggerComponent)
			return;
		
		GetGame().GetCallqueue().CallLater(RPC_DoTrigger); // Delay it to next frame, cannot delete entity in EOnContact
		Rpc(RPC_DoTrigger);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Manually trigger the entity (Server Only)
	void TriggerManuallyServer()
	{
		//~ Delay it to next frame, cannot trigger entity at the same time as Rpc
		GetGame().GetCallqueue().CallLater(RPC_DoTrigger);
		Rpc(RPC_DoTrigger);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Only call this on the server
	override void ActivateTrigger()
	{
		super.ActivateTrigger();
		
		IEntity owner = GetOwner();
		RplComponent rplComponent = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (!rplComponent || rplComponent.IsProxy())
			return;

		const float armingDelay = GetArmingTime() * 1000.0; //in ms
		if (armingDelay != 0)
			m_fLastTryTime = GetGame().GetWorld().GetWorldTime() + armingDelay;

		SetEventMask(owner, EntityEvent.CONTACT);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to reset the timeout, which is used to discard contacts when mine is arming
	//! Use it only on the server as it controlls when mines are detonated
	void ResetTimeout()
	{
		m_fLastTryTime = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Only call this on the server to disable the trigger mechanism
	override void DisarmTrigger()
	{
		HideFuse();
		GetInstigator().SetInstigator(null);

		IEntity owner = GetOwner();
		RplComponent rplComponent = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (!rplComponent || rplComponent.IsProxy())
			return;

		ClearEventMask(owner, EntityEvent.CONTACT);
		super.DisarmTrigger();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Method called on the clients, the item should be outside inventory already
	override void OnActivatedChanged()
	{
		super.OnActivatedChanged();

		SCR_PressureTriggerComponentClass data = SCR_PressureTriggerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		if (!m_bActivated)
			HideFuse();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void HideFuse()
	{
		IEntity owner = GetOwner();
		int meshIndex = GameAnimationUtils.FindMeshIndex(owner, m_sFuzeMeshName);
		if (meshIndex == -1)
			return;
		
		GameAnimationUtils.ShowMesh(owner, meshIndex, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		if (m_bLive)
			ActivateTrigger();
	}
}
