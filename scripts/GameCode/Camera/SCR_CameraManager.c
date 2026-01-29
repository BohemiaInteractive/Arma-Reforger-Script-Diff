class SCR_CameraManagerClass: CameraManagerClass
{
};

class SCR_CameraManager : CameraManager
{

	/*!
		Called on initialization.
	*/
	override protected void OnInit() 
	{
		SetupFOV();
		GetGame().OnUserSettingsChangedInvoker().Insert(SetupFOV);
	}
	
	protected void SetupFOV()
	{
		BaseContainer fovSettings = GetGame().GetGameUserSettings().GetModule("SCR_FieldOfViewSettings");
		if (!fovSettings)
			return;
		
		float fov;
		if (fovSettings.Get("m_fFirstPersonFOV", fov))
			SetFirstPersonFOV(fov);
		
		if (fovSettings.Get("m_fThirdPersonFOV", fov))
			SetThirdPersonFOV(fov);
		
		if (fovSettings.Get("m_fVehicleFOV", fov))
			SetVehicleFOV(fov);
	}
};