//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_VehicleDamageStateCondition : SCR_AvailableActionCondition
{
	[Attribute("1", UIWidgets.ComboBox, "Compared value of vehicle damage state", "", ParamEnumArray.FromEnum(EDamageState) )]
	private EDamageState m_eValue;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		IEntity vehicle = data.GetCurrentVehicle();
		if (!vehicle)
			return false;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(vehicle.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return false;

		bool result = (damageManager.GetState() == m_eValue);

		return GetReturnResult(result);
	}
};
