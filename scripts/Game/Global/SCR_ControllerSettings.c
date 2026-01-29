class SCR_AimSensitivitySettings : ModuleGameSettings
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0.1 2 0.01", desc: "Mouse aim sensitivity")]
	float m_fMouseSensitivity;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0.1 2 0.01", desc: "Stick aim sensitivity")]
	float m_fStickSensitivity;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0.1 1 0.01", desc: "Additional aim sensitivity multiplier for ADS")]
	float m_fAimADS;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "FOV input curve for mouse")]
	float m_fFOVInputCurveMouse;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "FOV input curve for stick")]
	float m_fFOVInputCurveStick;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "FOV input curve for gyro")]
	float m_fFOVInputCurveGyro;

	static void SetAimSensitivitySettings()
	{
		BaseContainer aimSensitivitySettings = GetGame().GetGameUserSettings().GetModule("SCR_AimSensitivitySettings");
		if (aimSensitivitySettings)
		{
			float aimSensitivityMouse;
			float aimSensitivityGamepad;
			float aimMultipADS;

			if (aimSensitivitySettings.Get("m_fMouseSensitivity", aimSensitivityMouse)
				&& aimSensitivitySettings.Get("m_fStickSensitivity", aimSensitivityGamepad)
				&& aimSensitivitySettings.Get("m_fAimADS", aimMultipADS))
			{
				CharacterControllerComponent.SetAimingSensitivity(aimSensitivityMouse, aimSensitivityGamepad, aimMultipADS);
				SCR_CharacterCameraHandlerComponent.SetADSSensitivity(aimMultipADS);
			}

			// Input curve: 0 = constant, 1 = linear
			float inputCurveMouse;
			float inputCurveStick;
			float inputCurveGyro;

			if (aimSensitivitySettings.Get("m_fFOVInputCurveMouse", inputCurveMouse)
				&& aimSensitivitySettings.Get("m_fFOVInputCurveStick", inputCurveStick)
				&& aimSensitivitySettings.Get("m_fFOVInputCurveGyro", inputCurveGyro))
			{
				CharacterControllerComponent.SetFOVInputCurve(inputCurveMouse, inputCurveStick, inputCurveGyro, 0);
			}
		}
	}
}

class SCR_HeadTrackingSettings : ModuleGameSettings
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, "Head tracking freelook")]
	bool m_bHeadTrackingFreelook;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, "Head tracking while aiming down sights")]
	bool m_bHeadTrackingADS;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 30 0.1", desc: "Head tracking sensitivity")]
	float m_fHeadTrackingSensitivity;

	[Attribute(defvalue: "0.05", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "Head tracking deadzone while aiming down sights")]
	float m_fHeadTrackingADSDeadzone;

	[Attribute(defvalue: SCR_Enum.GetDefault(ETrackIRLeanMode.ROLL), uiwidget: UIWidgets.ComboBox, desc: "Head tracking lean", enumType: ETrackIRLeanMode)]
	ETrackIRLeanMode m_eHeadTrackingLean;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.Slider, params: "0 30 0.1", desc: "Head tracking lean sensitivity")]
	float m_fHeadTrackingLeanSensitivity;

	[Attribute(defvalue: "0.2", uiwidget: UIWidgets.Slider, params: "0 1 0.01", desc: "Head tracking lean deadzone")]
	float m_fHeadTrackingLeanDeadzone;

	[Attribute(defvalue: "20", uiwidget: UIWidgets.Slider, params: "0 90 1", desc: "Head tracking lean yaw limit")]
	float m_fHeadTrackingLeanYawLimit;

	//------------------------------------------------------------------------------------------------
	static void SetHeadTrackingSettings()
	{
		BaseContainer headTrackingSettings = GetGame().GetGameUserSettings().GetModule("SCR_HeadTrackingSettings");
		if (headTrackingSettings)
		{
			bool headTrackingFreelook;
			bool headTrackingADS;
			ETrackIRLeanMode headTrackingLean;

			if (headTrackingSettings.Get("m_bHeadTrackingFreelook", headTrackingFreelook)
				&& headTrackingSettings.Get("m_bHeadTrackingADS", headTrackingADS)
				&& headTrackingSettings.Get("m_eHeadTrackingLean", headTrackingLean))
			{
				CharacterControllerComponent.SetTrackIREnable(headTrackingFreelook || headTrackingADS || headTrackingLean != ETrackIRLeanMode.DISABLED);
				CharacterControllerComponent.SetTrackIRFreelookEnable(headTrackingFreelook);
				CharacterControllerComponent.SetTrackIRWhileADSEnable(headTrackingADS);
				CharacterControllerComponent.SetTrackIRLeaningEnable(headTrackingLean != ETrackIRLeanMode.DISABLED);
				CharacterControllerComponent.SetTrackIRLeaningMode(headTrackingLean);
			}

			float headTrackingSensitivity;
			float headTrackingADSDeadzone;

			if (headTrackingSettings.Get("m_fHeadTrackingSensitivity", headTrackingSensitivity)
				&& headTrackingSettings.Get("m_fHeadTrackingADSDeadzone", headTrackingADSDeadzone))
			{
				CharacterControllerComponent.SetTrackIRFreelookSensitivity(headTrackingSensitivity);
				CharacterControllerComponent.SetTrackIRFreelookDeadzoneADS(headTrackingADSDeadzone);
			}

			float headTrackingLeanSensitivity;
			float headTrackingLeanDeadzone;
			float headTrackingLeanYawLimit;

			if (headTrackingSettings.Get("m_fHeadTrackingLeanSensitivity", headTrackingLeanSensitivity)
				&& headTrackingSettings.Get("m_fHeadTrackingLeanDeadzone", headTrackingLeanDeadzone)
				&& headTrackingSettings.Get("m_fHeadTrackingLeanYawLimit", headTrackingLeanYawLimit))
			{
				CharacterControllerComponent.SetTrackIRMoveLeaningSensitivity(headTrackingLeanSensitivity);
				CharacterControllerComponent.SetTrackIRRollLeaningSensitivity(headTrackingLeanSensitivity);
				CharacterControllerComponent.SetTrackIRLeaningDeadzone(headTrackingLeanDeadzone);
				CharacterControllerComponent.SetTrackIRLeaningActiveYawRange(headTrackingLeanYawLimit);
			}
		}
	}
}

class SCR_ControllerSettings : ModuleGameSettings
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, "Enable gyro always")]
	bool m_bGyroAlways;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, "Enable gyro while freelooking")]
	bool m_bGyroFreelook;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, "Enable gyro while aiming down sights")]
	bool m_bGyroADS;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 20 0.1", desc: "Gyro sensitivity")]
	float m_fGyroSensitivity;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, params: "0 2 0.01", desc: "Gyro vertical horizontal ratio")]
	float m_fGyroVerticalHorizontalRatio;

	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_EGyroAxisDirection.ENABLED), uiwidget: UIWidgets.ComboBox, desc: "Gyro yaw direction", enumType: SCR_EGyroAxisDirection)]
	SCR_EGyroAxisDirection m_eGyroDirectionYaw;

	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_EGyroAxisDirection.ENABLED), uiwidget: UIWidgets.ComboBox, desc: "Gyro pitch direction", enumType: SCR_EGyroAxisDirection)]
	SCR_EGyroAxisDirection m_eGyroDirectionPitch;

	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_EGyroAxisDirection.DISABLED), uiwidget: UIWidgets.ComboBox, desc: "Gyro roll direction", enumType: SCR_EGyroAxisDirection)]
	SCR_EGyroAxisDirection m_eGyroDirectionRoll;

	//------------------------------------------------------------------------------------------------
	static void SetControllerSettings()
	{
		BaseContainer controllerSettings = GetGame().GetGameUserSettings().GetModule("SCR_ControllerSettings");
		if (controllerSettings)
		{
			bool gyroAlways;
			bool gyroFreelook;
			bool gyroADS;

			if (controllerSettings.Get("m_bGyroAlways", gyroAlways)
				&& controllerSettings.Get("m_bGyroFreelook", gyroFreelook)
				&& controllerSettings.Get("m_bGyroADS", gyroADS))
			{
				CharacterControllerComponent.SetGyroControl(gyroAlways, gyroFreelook, gyroADS);
			}

			float gyroSensitivity;
			float gyroVerticalHorizontalRatio;

			SCR_EGyroAxisDirection gyroDirectionYaw;
			SCR_EGyroAxisDirection gyroDirectionPitch;
			SCR_EGyroAxisDirection gyroDirectionRoll;

			if (controllerSettings.Get("m_fGyroSensitivity", gyroSensitivity)
				&& controllerSettings.Get("m_fGyroVerticalHorizontalRatio", gyroVerticalHorizontalRatio)
				&& controllerSettings.Get("m_eGyroDirectionYaw", gyroDirectionYaw)
				&& controllerSettings.Get("m_eGyroDirectionPitch", gyroDirectionPitch)
				&& controllerSettings.Get("m_eGyroDirectionRoll", gyroDirectionRoll))
			{
				float sensitivityYaw;
				switch (gyroDirectionYaw)
				{
					case SCR_EGyroAxisDirection.ENABLED: sensitivityYaw = gyroSensitivity; break;
					case SCR_EGyroAxisDirection.INVERTED: sensitivityYaw = -gyroSensitivity; break;
				}

				float sensitivityPitch;
				switch (gyroDirectionPitch)
				{
					case SCR_EGyroAxisDirection.ENABLED: sensitivityPitch = gyroSensitivity; break;
					case SCR_EGyroAxisDirection.INVERTED: sensitivityPitch = -gyroSensitivity; break;
				}

				float sensitivityRoll;
				switch (gyroDirectionRoll)
				{
					case SCR_EGyroAxisDirection.ENABLED: sensitivityRoll = gyroSensitivity; break;
					case SCR_EGyroAxisDirection.INVERTED: sensitivityRoll = -gyroSensitivity; break;
				}

				if (gyroVerticalHorizontalRatio > 1)
				{
					// Slider over 50%: reduce horizontal
					float horizontalRatio = Math.Max(2 - gyroVerticalHorizontalRatio, 0);
					sensitivityYaw *= horizontalRatio;
					sensitivityRoll *= horizontalRatio;
				}
				else
				{
					// Slider below 50%: reduce vertical
					float verticalRatio = Math.Max(gyroVerticalHorizontalRatio, 0);
					sensitivityPitch *= verticalRatio;
				}

				CharacterControllerComponent.SetGyroSensitivity(sensitivityYaw, sensitivityPitch, sensitivityRoll);
			}
		}
	}
}
