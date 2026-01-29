//------------------------------------------------------------------------------------------------
//! Returns true if current muzzle's type matches specified type
[BaseContainerProps()]
class SCR_MuzzleTypeCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue:SCR_Enum.GetDefault(EMuzzleType.MT_BaseMuzzle), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EMuzzleType))]
	protected EMuzzleType m_eMuzzleType;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current muzzle's type matches specified type
	//! Returns opposite if m_bNegateCondition is enabled, but only if there is current muzzle at all
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		BaseMuzzleComponent muzzle = data.GetCurrentMuzzle();
		if (!muzzle)
			return false;

		bool result = muzzle.GetMuzzleType() == m_eMuzzleType;

		return GetReturnResult(result);
	}
};
