//! Utility class to help out with DLC specific scripting and similar odd jobs
class SCR_DLCHelperTools
{
	protected static const ResourceName DLC_ICONS_IMAGESET = "{3097250057240C6C}UI/Textures/DLC/DLC-Icons/DLCicons.imageset";

	protected static const ref array<string> DLC_ADDONS = {"RallyDLC"};

	//------------------------------------------------------------------------------------------------
	//! Returns the corresponding DLC icon for the DLC you put in
	//! \param[in] DLC Enum of the specific DLC
	//! \param[in] imageName image name of the DLC icon inside of the returned set
	//! \param[in] imageSetPrefab resource name for the corresponding set of DLC icons
	//! \return True if the icon got succesfully fetched
	static bool FetchDLCIcon(EDlc DLC, out string imageName, out ResourceName imageSetPrefab)
	{
		switch (DLC)
		{
			default:
				imageName = string.Empty;
				return false;
				break;
			
			case (EDlc.RALLY):
				imageName = "RallyDLC_symbol";
				break;
		}

		imageSetPrefab = DLC_ICONS_IMAGESET;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns an array of the names of all DLC addons 
	//! \return
	static array<string> FetchDLCAddons()
	{
		array<string> dlcAddons = {};
		dlcAddons.Copy(DLC_ADDONS);
		return dlcAddons;
	}
}