//-----------------------------------------------------------------------------
/*!
	Enables vehicle camera automatic default-direction alignment.
	Provides multiple parameters of desired values. 
	
	Can be added or removed to/from SCR_VehicleCameraDataComponent at will,
	to enable or disable this camera behaviour.
*/
[BaseContainerProps()]
class SCR_VehicleCameraAlignment : Managed
{
	/*!
		The delay (in seconds) without input before camera auto-alignment is engaged.
	*/
	[Attribute("1.1", UIWidgets.Slider, "Delay in seconds before auto-alignment is triggered", params: "0 60 0.001")]
	private float m_fDelay;
	
	/*
		The speed of the camera when interpolating to default.
	*/
	[Attribute("1.2", UIWidgets.Slider, "The speed of the camera when interpolating to default. Lower values mean smoother, slower transition.", params: "0 10 0.001")]
	private float m_fInterpolateSpeed;
	
	/*
		Align angles (in degrees) as yaw, pitch, roll in local space.
	*/
	[Attribute("0 -9 0", UIWidgets.Slider, "Align angles as yaw, pitch, roll in local space.")]
	private vector m_vAlignYawPitchRoll;
	
	/*
		Align angles (in degrees) as yaw, pitch, roll in local space when reversing.
	*/
	[Attribute("180 -18 0", UIWidgets.Slider, "Align angles as yaw, pitch, roll in local space when reversing.")]
	private vector m_vAlignYawPitchRollReverse;
	
	/*
		If checked (true), auto-alignment will be enabled even when in "focus mode".
		If unchecked, auto-alignment will be disabled when in "focus mode".
	*/
	[Attribute("0", UIWidgets.CheckBox, "If checked (true), auto-alignment will be enabled even when in 'focus mode'. If unchecked, auto-alignment will be disabled when in 'focus mode'.")]
	private bool m_bEnableInFocusMode;
	
	/*
		The speed in kph going backwards needed for the camera to transition into opposite direction.
	*/
	[Attribute("3", UIWidgets.Slider, "The speed in kph going backwards needed for the camera to transition into opposite direction.", params: "0 10 0.001")]
	private float m_fReverseSpeedTrigger;
	
	/*
		The speed in kph going forward needed for the camera to start aligning.
	*/
	[Attribute("5", UIWidgets.Slider, "The speed in kph going forward needed for the camera to automatically start aligning to direction.", params: "0 10 0.001")]
	private float m_fForwardSpeedTrigger;
	
	/*!
		Internal timer that counts elapsed time.
	*/
	private float m_fTimer;	
	
	
	/*
		Should auto align be enabled on gamepad?
	*/
	[Attribute("1", UIWidgets.CheckBox, "Enabled on gamepad?")]
	private bool m_bEnableGamepad;
	
	/*
		Should auto align be enabled on keyboard and mouse?
	*/
	[Attribute("0", UIWidgets.CheckBox, "Enabled on keyboard and mouse?")]
	private bool m_bEnableKeyboard;

	//-----------------------------------------------------------------------------
	/*!
		Update the auto alignment object.
		Returns new rotation in local space as yaw, pitch, roll in degrees.
		\param aimChange The input change (if any).
		\param currentAngles Current camera local yaw, pitch, roll in degrees.
		\param timeSlice The time step since last frame
		\returns Returns new local rotation as yaw, pitch, roll in degrees
	*/	
	bool Update(vector aimChange, vector currentAngles, vector localVelocity, bool isFocus, float timeSlice, out vector newAngles)
	{		
		EInputDeviceType lastDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		if ((lastDevice != EInputDeviceType.GAMEPAD) && !m_bEnableKeyboard)
			return false;
		else if ((lastDevice == EInputDeviceType.GAMEPAD) && !m_bEnableGamepad)
			return false;
		
		
		// Input changed, return original.
		if (aimChange != vector.Zero) 
		{
			m_fTimer = 0.0;
			return false;
		}
		
		// Focus mode not enabled
		if (!m_bEnableInFocusMode && isFocus)
		{
			m_fTimer = 0.0;
			return false;
		}
		
		// Do not align until fast enough
		float speed = localVelocity[2] * METERS_PER_SEC_TO_KILOMETERS_PER_HOUR;
		if (speed > -m_fReverseSpeedTrigger && speed < m_fForwardSpeedTrigger)
		{
			m_fTimer = 0.0;
			return false;
		}
		
		// Timer has reached its peak, perform auto aligning..
		if (m_fTimer > m_fDelay)
		{
			// Check fwd/backwards speed to find out if we're reversing
			bool isReverse = speed <= -m_fReverseSpeedTrigger;
			vector alignTarget = m_vAlignYawPitchRoll;
			if (isReverse)
				alignTarget = m_vAlignYawPitchRollReverse;
			alignTarget[1] = 0.0; // Hack: Ignore pitch for now. This is independently controlled by CharacterCamera3rdPersonVehicle.c
			
			// We can return result if we're within a threshold
			vector delta = (alignTarget - currentAngles);
			const float DELTA_EPSILON = 0.01;
			
			float deltaMagnitude = delta.LengthSq();
			if (deltaMagnitude <= DELTA_EPSILON)
			{
				newAngles = alignTarget;
				return true;
			}
			
			// From quaternion
			vector fromMat[3];
			Math3D.AnglesToMatrix(currentAngles, fromMat);
			float fromQuat[4];
			Math3D.MatrixToQuat(fromMat, fromQuat);
			
			// To quaternion
			vector toMat[3];
			Math3D.AnglesToMatrix(alignTarget, toMat);
			float toQuat[4];
			Math3D.MatrixToQuat(toMat, toQuat);
			
			// Interpolate
			float resQuat[4];
			Math3D.QuatLerp(resQuat, fromQuat, toQuat, timeSlice * m_fInterpolateSpeed);
			
			newAngles = Math3D.QuatToAngles(resQuat);
			return true;
		}
		else
		{
			// Keep incrementing the timer
			m_fTimer += timeSlice;
		}
				
		// Return original angles
		return false;
	}
};