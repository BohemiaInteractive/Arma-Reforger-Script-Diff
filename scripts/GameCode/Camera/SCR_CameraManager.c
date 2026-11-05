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
		SetupViewDistance();
		GetGame().OnUserSettingsChangedInvoker().Insert(SetupFOV);
		GetGame().OnUserSettingsChangedInvoker().Insert(SetupViewDistance);
	}

	protected void SetupViewDistance()
	{
		BaseContainer videoSettings = GetGame().GetGameUserSettings().GetModule("SCR_VideoSettings");
		if (!videoSettings)
			return;

		int viewDistance;
		if (videoSettings.Get("m_iViewDistance", viewDistance))
			GetGame().SetViewDistance(viewDistance);
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