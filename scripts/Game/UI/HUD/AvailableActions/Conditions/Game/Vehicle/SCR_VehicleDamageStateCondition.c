[BaseContainerProps()]
class SCR_VehicleDamageStateCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EDamageState.INTERMEDIARY.ToString(), desc: "Compared value of vehicle damage state", uiwidget: UIWidgets.ComboBox, enumType: EDamageState)]
	private EDamageState m_eValue;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		IEntity vehicle = data.GetCurrentVehicle();
		if (!vehicle)
			return false;

		DamageManagerComponent damageManager = DamageManagerComponent.Cast(vehicle.FindComponent(DamageManagerComponent));
		return GetReturnResult(damageManager && damageManager.GetState() == m_eValue);
	}
}
