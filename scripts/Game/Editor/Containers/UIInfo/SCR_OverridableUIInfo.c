//! UI info class with setter functions.
//! Use at own risk!
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_OverridableUIInfo : SCR_UIInfo
{
	//------------------------------------------------------------------------------------------------
	//! Set info name
	//! \param name New name
	override void SetName(LocalizedString name)
	{
		Name = name;
	}

	//------------------------------------------------------------------------------------------------
	//! Set info description
	//! \param description New description
	override void SetDescription(LocalizedString description)
	{
		Description = description;
	}

	//------------------------------------------------------------------------------------------------
	//! Set info icon
	//! \param icon New icon path
	void SetIcon(string icon)
	{
		Icon = icon;
	}

	//------------------------------------------------------------------------------------------------
	//! Set info icon set name
	//! \param iconSetName New icon set name
	void SetIconSetName(string iconSetName)
	{
		IconSetName = iconSetName;
	}
}
