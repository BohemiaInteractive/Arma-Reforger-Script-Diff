//! @ingroup ManualCamera

//! Adjust camera field of view.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_RotateRollManualCameraComponent : SCR_BaseManualCameraComponent
{
	protected static const float INERTIA_THRESHOLD = 0.001;
	
	[Attribute(defvalue: "100", desc: "Value coefficient.")]	
	private float m_fCoef;
	
	[Attribute(defvalue: "2", desc: "When roll is below this angle, return it back to 0.")]	
	private float m_fMinRoll;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraReset()
	{
		vector angles = GetCameraEntity().GetAngles();
		angles[2] = 0;
		GetCameraEntity().SetAngles(angles);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraSave(SCR_ManualCameraComponentSave data)
	{
		vector angles = GetCameraEntity().GetAngles();
		if (angles[2] != 0)
			data.m_aValues = {angles[2]};
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraLoad(SCR_ManualCameraComponentSave data)
	{
		if (!data.m_aValues || data.m_aValues.IsEmpty())
			return;
		
		vector angles = GetCameraEntity().GetAngles();
		angles[2] = data.m_aValues[0];
		GetCameraEntity().SetAngles(angles);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled) return;
		
		float rotateRoll = 0;
		if (GetInputManager().IsUsingMouseAndKeyboard() || GetInputManager().GetActionValue("ManualCameraModifier"))
			rotateRoll = GetInputManager().GetActionValue("ManualCameraRotateRoll");
		
		if (rotateRoll != 0)
		{
			param.rotDelta[2] = param.rotDelta[2] + rotateRoll * m_fCoef * Math.Max(Math.AbsFloat(param.rotOriginal[2] / 180), 0.1);
			param.isManualInput = true;
			param.isDirty = true;
		}
		else
		{
			float rollOriginal = Math.AbsFloat(param.rotOriginal[2]);
			if (rollOriginal > INERTIA_THRESHOLD && rollOriginal < m_fMinRoll)
			{
				param.rotDelta[2] = -param.rotOriginal[2] * 5; //--- Multiply to speed up smooth return faciliated by timeSlice multiplication
				param.isDirty = true;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
