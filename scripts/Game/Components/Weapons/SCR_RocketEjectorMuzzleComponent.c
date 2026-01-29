[ComponentEditorProps(category: "GameScripted/Muzzle", description: "")]
class SCR_RocketEjectorMuzzleComponentClass: RocketEjectorMuzzleComponentClass
{
}

class SCR_RocketEjectorMuzzleComponent: RocketEjectorMuzzleComponent
{
	[Attribute("", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Default ammunition type that belongs with this muzzle", params: "et", category: "")]
	protected ResourceName m_sDefaultAmmoPrefab;
	
	//------------------------------------------------------------------------------------------------
	//! \return Get the default rocket for this muzzle
	ResourceName GetDefaultRocketPrefab()
	{
		return m_sDefaultAmmoPrefab;
	}
}