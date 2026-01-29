//! @ingroup ManualCamera

//! Basic camera movement and rotation
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_MouseMoveManualCameraComponent : SCR_BaseManualCameraComponent
{
	/*
	[Attribute("25", UIWidgets.Auto, "")]	
	float m_fFlySpeed;
	
	override void Add(out vector posLocalDelta, out vector posWorldDelta, out vector rotDelta, float multiplier, vector posCurrent, vector rotCurrent, vector posSet, vector rotSet)
	{
		if (m_InputManager.GetActionValue("ManualCameraRotateModifier") && !m_InputManager.GetActionValue("ManualCameraMoveLongitudinal"))
		{
			float moveLongitudinal = m_InputManager.GetActionValue("ManualCameraMoveLongitudinalSecondary");
			
			if (moveLongitudinal == 0) return;
			
			posLocalDelta += Vector(0, 0, moveLongitudinal) * m_fFlySpeed * multiplier;
			
			m_Camera.SetManualMovement(true);
			m_Camera.SetNewHeightFromTerrain(true);
		}			
	}
	*/
}
