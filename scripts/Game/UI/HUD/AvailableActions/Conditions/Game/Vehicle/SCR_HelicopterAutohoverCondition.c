//! Returns true if autohover is on
[BaseContainerProps()]
class SCR_HelicopterAutohoverCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		HelicopterControllerComponent controller = HelicopterControllerComponent.Cast(data.GetCurrentVehicleController());
		return GetReturnResult(controller && controller.GetAutohoverEnabled());		
	}
}
