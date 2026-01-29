class SCR_EngineAction : SCR_VehicleActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_VehicleController);
		if (controller)
			controller.TryStartEngine();
	}

	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	override void OnActionStart(IEntity pUserEntity)
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_VehicleController);
		if (controller)
			controller.TryStartEngine();
	}

	//! Action canceled
	//! \param pUserEntity The entity that started performing this action
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_bIsToggle && !m_bTargetState)
			return;

		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_VehicleController);
		if (controller && !controller.IsEngineOn())
			controller.CancelStart();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return m_VehicleController && super.CanBeShownScript(user) && CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetState()
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_VehicleController);
		return controller && controller.IsEngineOn();
	}

	//------------------------------------------------------------------------------------------------
	override void SetState(bool enable)
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(m_VehicleController);
		if (!controller)
			return;

		if (enable)
			controller.StartEngine();
		else
			controller.StopEngine();
	}
}
