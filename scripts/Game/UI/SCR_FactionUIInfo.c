//! Forward declaration of UIInfo
//! Serves as a container of data to be displayed on player UI
class SCR_FactionUIInfo : UIInfo
{
	//! Name in upper case
	[Attribute("", UIWidgets.EditBox, "Upper case name.", "")]
	private string m_sNameUpper;
	
	string GetFactionNameUpperCase()
	{
		return m_sNameUpper;
	}
	
	
};