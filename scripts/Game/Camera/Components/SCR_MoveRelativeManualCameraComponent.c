//! @ingroup ManualCamera

//! Basic camera movement and rotation
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_MoveRelativeManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "25", desc: "Speed coefficient.")]
	private float m_fSpeed;	
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled) return;
		
		vector inputVector = Vector
		(
			GetInputManager().GetActionValue("ManualCameraMoveLateral"),
			GetInputManager().GetActionValue("ManualCameraMoveVertical"),
			GetInputManager().GetActionValue("ManualCameraMoveLongitudinal")
		);

		if (inputVector != vector.Zero)
		{
			inputVector[0] = inputVector[0] * param.multiplier[0] * m_fSpeed;
			inputVector[1] = inputVector[1] * param.multiplier[1] * m_fSpeed;
			inputVector[2] = inputVector[2] * param.multiplier[0] * m_fSpeed;
			
			param.transform[3] = param.transform[3]
				+ (param.transform[0] * inputVector[0])
				+ (param.transform[1] * inputVector[1])
				+ (param.transform[2] * inputVector[2]);
			
			param.isManualInput = true;
			param.isDirty = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
