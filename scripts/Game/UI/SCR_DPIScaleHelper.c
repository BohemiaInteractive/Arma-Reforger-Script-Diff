class SCR_DPIScaleHelper
{
	protected const string DPISCALE_SETTING_NAME = "m_fDPIscale";
	protected const string INTERFACE_SETTINGS_MODULE_NAME = "SCR_InterfaceSettings";
	
	protected const float DEFAULT_HANDHELD_SCALE = 1.05;

	//------------------------------------------------------------------------------------------------
	//! \param[in] workspace
	//! \param[in] scale
	static void SetDPIScale(notnull WorkspaceWidget workspace, float scale)
	{
		workspace.SetUserDPIScale(scale);
	}

	//------------------------------------------------------------------------------------------------
	static float GetSavedDPIScale()
	{
		float scale = 0;

		BaseContainer interfaceSettings = GetGame().GetGameUserSettings().GetModule(INTERFACE_SETTINGS_MODULE_NAME);
		if (interfaceSettings)
			interfaceSettings.Get(DPISCALE_SETTING_NAME, scale);
		
		//Set to default scale if there is no saved scale
		if (scale <= 0)
		{
			if (System.GetPlatform() == EPlatform.WINDOWS_HANDHELD)
				scale = DEFAULT_HANDHELD_SCALE;
			else
				scale = 1.0;
		}

		return scale;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] workspace
	static void LoadSavedDPIScale(notnull WorkspaceWidget workspace)
	{
		float savedScale = GetSavedDPIScale();
		SetDPIScale(workspace, savedScale);

		#ifdef ENABLE_DIAG
		DiagMenu.SetRangeValue(SCR_DebugMenuID.DEBUGUI_UI_SET_DPI_SCALE, savedScale);
		#endif
	}
}
