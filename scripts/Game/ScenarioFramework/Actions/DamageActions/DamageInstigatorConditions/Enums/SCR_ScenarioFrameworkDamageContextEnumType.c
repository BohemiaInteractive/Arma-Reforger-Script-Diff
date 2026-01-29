[BaseContainerProps()]
class SCR_ScenarioFrameworkDamageContextEnumType
{
	[Attribute("Damage Type", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EDamageType))]
	protected EDamageType m_eDamageType;

	//------------------------------------------------------------------------------------------------
	//! \return
	EDamageType GetDamageType()
	{
		return m_eDamageType;
	}
}