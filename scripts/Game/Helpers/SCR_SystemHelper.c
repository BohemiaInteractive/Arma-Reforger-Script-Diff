class SCR_SystemHelper
{
	static SCR_EResolutionName GetResolutionName(int width, int height)
	{
		if (width == 640 && height == 480)
			return SCR_EResolutionName.E480p;
		else if (width == 1280 && height == 720)
			return SCR_EResolutionName.E720p;
		else if (width == 1920 && height == 1080)
			return SCR_EResolutionName.E1080p;
		else if (width == 2560 && height == 1440)
			return SCR_EResolutionName.E1440p;
		else if (width == 3840 && height == 2160)
			return SCR_EResolutionName.E2160p;
		else if (width == 7680 && height == 4320)
			return SCR_EResolutionName.E4320p;
		
		return SCR_EResolutionName.INVALID;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when this client is on PC, otherwise false
	static bool IsPC()
	{
		EPlatform platform = System.GetPlatform();
		return platform == EPlatform.WINDOWS || platform == EPlatform.LINUX || platform == EPlatform.WINDOWS_HANDHELD || platform == EPlatform.UNKNOWN;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when this client is on PlayStation, otherwise false
	static bool IsPlayStation()
	{
		EPlatform platform = System.GetPlatform();
		return platform == EPlatform.PS5 || platform == EPlatform.PS5_PRO || platform == EPlatform.PS4;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when this client is on Xbox, otherwise false
	static bool IsXbox()
	{
		EPlatform platform = System.GetPlatform();
		return platform == EPlatform.XBOX_ONE || platform == EPlatform.XBOX_ONE_S || platform == EPlatform.XBOX_ONE_X || platform == EPlatform.XBOX_SERIES_S || platform == EPlatform.XBOX_SERIES_X;
	}
}

enum SCR_EResolutionName 
{
	INVALID,
	E480p,	// 640 x 480
	E720p,	// 1280 x 720
	E1080p,	// 1920 x 1080
	E1440p,	// 2560 x 1440
	E2160p,	// 3840 x 2160
	E4320p	// 7680 x 4320
}