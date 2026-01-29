class SCR_DoctorCompartmentSlot : CargoCompartmentSlot
{
	[Attribute("1", UIWidgets.CheckBox, "Whether characters in this seat can interact with UserActions on other characters")]
	protected bool m_bAllowHealingFromCompartment;
	
	//-----------------------------------------------------------------------------------------------------------
	bool AllowHealingFromCompartment()
	{
		return m_bAllowHealingFromCompartment;
	}
}
