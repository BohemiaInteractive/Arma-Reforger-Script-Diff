//! Slot that is meant to be used for excplosive charge fuze
class FuzeSlotInfo : EntitySlotInfo
{
	[Attribute(SCR_EFuzeType.NONE.ToString(), UIWidgets.ComboBox, desc: "What fuze mesh will be stored in this slot", enums: ParamEnumArray.FromEnum(SCR_EFuzeType))]
	protected SCR_EFuzeType m_eFuzeType;

	//------------------------------------------------------------------------------------------------
	SCR_EFuzeType GetFuzeType()
	{
		return m_eFuzeType;
	}
}