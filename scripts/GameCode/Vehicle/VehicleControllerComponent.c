class VehicleControllerComponentClass : BaseVehicleControllerComponentClass
{
}

/*!
	Class responsible for base game vehicle.
*/
class VehicleControllerComponent : BaseVehicleControllerComponent
{
	protected ref ScriptInvokerVoid m_OnEngineStart, m_OnEngineStop;
	protected int m_iOccupants;

	//! needed to check if engine start is even possible
	protected SCR_VehicleDamageManagerComponent m_DamageManager;
	protected SCR_PowerComponent m_PowerComponent;
	protected bool m_bIsStarting;

	//sound
	protected AudioHandle m_iEngineStarterHandle = AudioHandle.Invalid;
	protected AudioHandle m_iEngineStartHandle = AudioHandle.Invalid;
	protected AudioHandle m_iEngineStopHandle = AudioHandle.Invalid;

	//------------------------------------------------------------------------------------------------
	//! Invoker for the Engine Started event
	ScriptInvokerVoid GetOnEngineStart()
	{
		if (!m_OnEngineStart)
			m_OnEngineStart = new ScriptInvokerVoid();

		return m_OnEngineStart;
	}

	//------------------------------------------------------------------------------------------------
	//! Invoker for the Engine Stopped event
	ScriptInvokerVoid GetOnEngineStop()
	{
		if (!m_OnEngineStop)
			m_OnEngineStop = new ScriptInvokerVoid();

		return m_OnEngineStop;
	}

	//------------------------------------------------------------------------------------------------
	SCR_VehicleDamageManagerComponent GetDamageManager()
	{
		return m_DamageManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Return if engine starter is functional
	bool IsStarterFunctional()
	{
		if (m_DamageManager && !m_DamageManager.GetEngineFunctional())
			return false;

		// TODO: Option to start engine despite lack of power if vehicle is moving
		if (m_PowerComponent && !m_PowerComponent.HasPower())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDestroyed(IEntity ent)
	{
		auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(ent);
		if (garbageSystem)
			garbageSystem.Insert(ent);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (m_iOccupants++ == 0)
		{
			auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(vehicle);
			if (garbageSystem)
				garbageSystem.Withdraw(vehicle);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		if (--m_iOccupants == 0)
		{
			auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(vehicle);
			if (garbageSystem)
				garbageSystem.Insert(vehicle);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		m_PowerComponent = SCR_PowerComponent.Cast(owner.FindComponent(SCR_PowerComponent));
		m_DamageManager = SCR_VehicleDamageManagerComponent.Cast(owner.FindComponent(SCR_VehicleDamageManagerComponent));

		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (ev)
		{
			ev.RegisterScriptHandler("OnDestroyed", this, OnDestroyed, false);
			ev.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered, false);
			ev.RegisterScriptHandler("OnCompartmentLeft", this, OnCompartmentLeft, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
		if (ev)
		{
			ev.RemoveScriptHandler("OnDestroyed", this, OnDestroyed, false);
			ev.RemoveScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered);
			ev.RemoveScriptHandler("OnCompartmentLeft", this, OnCompartmentLeft);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when the engine start routine begins (animation event).
	override void OnEngineStartBegin()
	{
		if (GetEngineDrowned())
			return;

		if (!IsStarterFunctional())
			return;

		m_bIsStarting = true;

		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStart(true);

		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_iEngineStopHandle);
			m_iEngineStopHandle = AudioHandle.Invalid;

			m_iEngineStarterHandle = soundComponent.SoundEvent(SCR_SoundEvent.SOUND_ENGINE_STARTER_LP);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get called while engine starter is active.
	override void OnEngineStartProgress()
	{
		if (!m_bIsStarting)
			return;

		if (m_DamageManager && !m_DamageManager.GetEngineFunctional())
			OnEngineStartFail(EVehicleEngineStartFailedReason.DESTROYED);
		else if (!IsStarterFunctional())
			OnEngineStartFail(EVehicleEngineStartFailedReason.MAX_ATTEMPTS);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when the engine start routine was interrupted.
	override void OnEngineStartInterrupt()
	{
		m_bIsStarting = false;

		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStop();

		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_iEngineStarterHandle);
			m_iEngineStarterHandle = AudioHandle.Invalid;
			soundComponent.Terminate(m_iEngineStartHandle);
			m_iEngineStartHandle = AudioHandle.Invalid;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when the engine start routine has successfully completed.
	override void OnEngineStartSuccess()
	{
		m_bIsStarting = false;

		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_iEngineStarterHandle);
			m_iEngineStarterHandle = AudioHandle.Invalid;
			soundComponent.Terminate(m_iEngineStartHandle);
			m_iEngineStartHandle = soundComponent.SoundEvent(SCR_SoundEvent.SOUND_ENGINE_START);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets called when the engine start routine has failed.
	override void OnEngineStartFail(EVehicleEngineStartFailedReason reason)
	{
		if (!m_bIsStarting)
			return;

		m_bIsStarting = false;

		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStop();

		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_iEngineStarterHandle);
			m_iEngineStarterHandle = AudioHandle.Invalid;
			soundComponent.Terminate(m_iEngineStartHandle);
			m_iEngineStartHandle = AudioHandle.Invalid;

			if (reason != EVehicleEngineStartFailedReason.NO_FUEL)
				return;

			soundComponent.SoundEvent(SCR_SoundEvent.SOUND_ENGINE_START_FAILED);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Is called every time the controller wants to start the engine.

		\return true if the engine can start, false otherwise.
	*/
	override bool OnBeforeEngineStart()
	{
		if (!IsStarterFunctional())
			return false;

		// engine can start by default
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Is called every time the engine starts.
	override void OnEngineStart()
	{
		SCR_FuelConsumptionComponent fuelConsumption = SCR_FuelConsumptionComponent.Cast(GetOwner().FindComponent(SCR_FuelConsumptionComponent));
		if (fuelConsumption)
			fuelConsumption.SetEnabled(true);

		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStart(false);

		if (m_OnEngineStart)
			m_OnEngineStart.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! Is called every time the engine stops.
	override void OnEngineStop()
	{
		SCR_FuelConsumptionComponent fuelConsumption = SCR_FuelConsumptionComponent.Cast(GetOwner().FindComponent(SCR_FuelConsumptionComponent));
		if (fuelConsumption)
			fuelConsumption.SetEnabled(false);

		if (m_OnEngineStop)
			m_OnEngineStop.Invoke();

		SCR_MotorExhaustEffectGeneralComponent motorExhaust = SCR_MotorExhaustEffectGeneralComponent.Cast(GetOwner().FindComponent(SCR_MotorExhaustEffectGeneralComponent));
		if (motorExhaust)
			motorExhaust.OnEngineStop();

		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.Terminate(m_iEngineStarterHandle);
			m_iEngineStarterHandle = AudioHandle.Invalid;

			soundComponent.Terminate(m_iEngineStartHandle);
			m_iEngineStartHandle = AudioHandle.Invalid;

			soundComponent.SoundEvent(SCR_SoundEvent.SOUND_ENGINE_STOP);
		}
	}
	//------------------------------------------------------------------------------------------------
	override bool ValidateCanMove()
	{
		return m_DamageManager.GetMovementDamage() < 1;
	}
}
