//------------------------------------------------------------------------------
class SCR_CampaignMapUIElement : SCR_MapUIElement
{
	// strings for searching in imagesets
	[Attribute("Armory")]
	protected string m_sArmory;
	[Attribute("FuelDepot")]
	protected string m_sFuelDepot;
	[Attribute("LightVehicleDepot")]
	protected string m_sLightVehicleDepot;
	[Attribute("HeavyVehicleDepot")]
	protected string m_sHeavyVehicleDepot;
	[Attribute("RadioAntenna")]
	protected string m_sRadioAntenna;
	[Attribute("Medical")]
	protected string m_sFieldHospital;
	[Attribute("Barracks")]
	protected string m_sBarracks;
	[Attribute("SupplyDepot")]
	protected string m_sSupplyDepot;
	[Attribute("VehicleDepot")]
	protected string m_sVehicleDepot;
	[Attribute("Heli_Pad")]
	protected string m_sHelipad;
	[Attribute("Unknown")]
	protected string m_sFactionNeutral;
	[Attribute("Relay")]
	protected string m_sRelay;
	[Attribute("Base")]
	protected string m_sBase;
	[Attribute("SourceBase")]
	protected string m_sSourceBase;
	[Attribute("Mobile")]
	protected string m_sMobileAssembly;
	[Attribute("Major")]
	protected string m_sMajorBase;
	[Attribute("Minor")]
	protected string m_sMinorBase;
	[Attribute("Small")]
	protected string m_sSmallBase;
	[Attribute("Respawn")]
	protected string m_sRespawn;
	[Attribute("Bg")]
	protected string m_sRespawnBackground;
	[Attribute("Select")]
	protected string m_sSelection;

	[Attribute("{F7E8D4834A3AFF2F}UI/Imagesets/Conflict/conflict-icons-bw.imageset")]
	protected ResourceName m_sImageSet;

	//------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
	}
	
	//------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		return false;
	}

	override void SetImage(string image)
	{
		if (m_wImage)
			m_bVisible = m_wImage.LoadImageFromSet(0, m_sImageSet, image);
	}
};