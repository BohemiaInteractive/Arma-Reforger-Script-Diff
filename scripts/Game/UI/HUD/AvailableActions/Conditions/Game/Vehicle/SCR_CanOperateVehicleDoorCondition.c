[BaseContainerProps()]
class SCR_CanOperateVehicleDoorCondition : SCR_AvailableActionCondition
{
	[Attribute(desc: "Requires the door to be open")]
	protected bool m_bRequireOpenDoor;

	[Attribute(SCR_ECompartmentDoorType.DOOR.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Checks if door assigned to current compartment is a hatch", enumType: SCR_ECompartmentDoorType)]
	protected SCR_ECompartmentDoorType m_eDoorType;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		ChimeraCharacter character = data.GetCharacter();
		if (!character)
			return GetReturnResult(false);

		CompartmentAccessComponent compartmentAccessComp = character.GetCompartmentAccessComponent();
		if (!compartmentAccessComp)
			return GetReturnResult(false);

		if (compartmentAccessComp.IsGettingIn() || compartmentAccessComp.IsGettingOut() || compartmentAccessComp.IsSwitchingSeatsAnim())
			return GetReturnResult(false);

		BaseCompartmentSlot compartment = compartmentAccessComp.GetCompartment();
		if (!compartment)
			return GetReturnResult(false);

		array<int> arr = {};
		compartment.GetAvailableDoorIndices(arr);
		if (arr.IsEmpty())
			return GetReturnResult(false);

		int mainDoorId = arr[0];
		if (mainDoorId == compartment.GetTurnOutDoorIndex())
			return GetReturnResult(false);

		BaseCompartmentManagerComponent compartmentManager = compartment.GetManager();
		if (!compartmentManager)
			return GetReturnResult(false);

		if (compartmentManager.IsDoorFake(mainDoorId))
			return GetReturnResult(false);

		SCR_CompartmentDoorInfo doorInfo = SCR_CompartmentDoorInfo.Cast(compartmentManager.GetDoorInfo(mainDoorId));
		SCR_ECompartmentDoorType doorType = SCR_ECompartmentDoorType.DOOR;//we assume that if there is no info about the door type then it must be a door
		if (doorInfo)
			doorType = doorInfo.GetDoorType();

		if (doorType != m_eDoorType)
			return GetReturnResult(false);

		if (!m_bRequireOpenDoor && !compartmentAccessComp.CanAccessDoor(compartment.GetOwner(), compartmentManager, mainDoorId))
			return GetReturnResult(false);

		return GetReturnResult(compartmentManager.IsDoorOpen(mainDoorId) == m_bRequireOpenDoor);
	}
}