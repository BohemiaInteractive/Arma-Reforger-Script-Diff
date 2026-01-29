//! @ingroup ManualCamera

//! Speed boost for camera movement.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_SpeedBoostManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("3")]
	private float m_fSpeedBoost;
	
	private float m_fSpeedBoostActive = 1;
	
	//------------------------------------------------------------------------------------------------
	protected void ManualCameraSpeedBoostDown(float value, EActionTrigger trigger)
	{
		if (!IsEnabled())
			return;

		m_fSpeedBoostActive = m_fSpeedBoost * value;
	}

	//------------------------------------------------------------------------------------------------
	protected void ManualCameraSpeedBoostUp(float value, EActionTrigger trigger)
	{
		if (!IsEnabled())
			return;

		m_fSpeedBoostActive = 1;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled)
			return;
		
		if (m_fSpeedBoostActive != 1)
			param.multiplier *= m_fSpeedBoostActive;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		GetInputManager().AddActionListener("ManualCameraSpeedBoost", EActionTrigger.DOWN, ManualCameraSpeedBoostDown);
		GetInputManager().AddActionListener("ManualCameraSpeedBoost", EActionTrigger.UP, ManualCameraSpeedBoostUp);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		GetInputManager().RemoveActionListener("ManualCameraSpeedBoost", EActionTrigger.DOWN, ManualCameraSpeedBoostDown);
		GetInputManager().RemoveActionListener("ManualCameraSpeedBoost", EActionTrigger.UP, ManualCameraSpeedBoostUp);
	}
}
