class SCR_MotionControlCalibrationPromptDialog : SCR_ConfigurableDialogUi
{
	//------------------------------------------------------------------------------------------------
	void SCR_MotionControlCalibrationPromptDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "calibrate_motion_control_prompt", this);
	}

    //------------------------------------------------------------------------------------------------
    override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
    {
		super.OnMenuOpen(preset);
		
		SCR_InputButtonComponent confirm = FindButton("continue_calibration");
		if (confirm)
			confirm.m_OnActivated.Insert(OnConfirmCalibrate);
    }
	
	//------------------------------------------------------------------------------------------------
	void OnConfirmCalibrate()
	{
		new SCR_MotionControlCalibrationProcessDialog();
		Close();
	}
}

class SCR_MotionControlCalibrationProcessDialog : SCR_ConfigurableDialogUi
{
	SCR_InputButtonComponent m_ConfirmButton;
	ref InputBinding m_Binding;
	bool calibrating = false;
	
	//------------------------------------------------------------------------------------------------
	void SCR_MotionControlCalibrationProcessDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "calibrate_motion_control_process", this);
		
		m_ConfirmButton = FindButton("confirm");
		if (m_ConfirmButton)
			m_ConfirmButton.SetVisible(false);
		
		m_Binding = GetGame().GetInputManager().CreateUserBinding();
		if (!m_Binding)
		{
			Print("SCR_MotionControlCalibrationProcessDialog: InputBindings were not created!", LogLevel.WARNING);
			OnResult(false);
			return;
		}
		
		m_Binding.StartCalibration(EInputDeviceType.GYRO);
		calibrating = true;
	}
	
	override void OnCancel()
	{
		calibrating = false;
		m_Binding.StopCalibration();
		Close();
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		if (calibrating)
		{
			EInputBindingCalibrationStatus status = m_Binding.GetCalibrationStatus();
			if (status != EInputBindingCalibrationStatus.CALIBRATING && status != EInputBindingCalibrationStatus.IDLE)
			{
				calibrating = false;
				OnResult(status == EInputBindingCalibrationStatus.CALIBRATION_SUCCESS);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnResult(bool success)
	{
		if (m_ConfirmButton)
			m_ConfirmButton.SetVisible(true);
		
		SCR_InputButtonComponent cancel = FindButton("cancel");
		if (cancel)
			cancel.SetVisible(false);
		
		RichTextWidget title = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Title"));
		if (title)
		{
			if (success)
				title.SetColor(UIColors.CONFIRM);
			else
				title.SetColor(UIColors.WARNING);
		}
		
		ImageWidget icon = ImageWidget.Cast(m_wRoot.FindAnyWidget("ImgTitleIcon"));
		if (icon)
		{
			if (success)
			{
				icon.SetColor(UIColors.CONFIRM);
				SetTitleIcon("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", "check");
			}
			else
			{
				icon.SetColor(UIColors.WARNING);
				SetTitleIcon("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", "cancel");
			}
		}
		
		RichTextWidget message = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Message"));
		if (message)
		{
			message.SetVisible(true);
			if (success)
				message.SetText("#AR-Settings_MotionControlCalibration_ResultSuccess");
			else
				message.SetText("#AR-Settings_MotionControlCalibration_ResultFail");
		}	
	}
	
	
}