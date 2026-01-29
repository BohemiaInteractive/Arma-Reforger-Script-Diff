class SCR_PatientCompartmentSlot : CargoCompartmentSlot
{
	[Attribute("1", UIWidgets.CheckBox, "Whether patients can be loaded into this compartment")]
	protected bool m_bAllowLoadingIntoCompartment;
}
