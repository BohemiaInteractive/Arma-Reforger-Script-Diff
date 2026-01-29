class SCR_CompartmentDoorInfo : CompartmentDoorInfo
{
	[Attribute(SCR_ECompartmentDoorType.DOOR.ToString(), uiwidget: UIWidgets.ComboBox, enumType: SCR_ECompartmentDoorType)]
	protected SCR_ECompartmentDoorType m_eDoorType;

	//------------------------------------------------------------------------------------------------
	SCR_ECompartmentDoorType GetDoorType()
	{
		return m_eDoorType;
	}
}
