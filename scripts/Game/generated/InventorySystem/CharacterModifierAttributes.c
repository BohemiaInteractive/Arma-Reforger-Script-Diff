/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class CharacterModifierAttributes: BaseItemAttributeData
{
	/*!
	Returns the speed limit of the current character
	*/
	proto external float SpeedLimit();
	/*!
	Returns the speed limit of the current character while using ADS
	*/
	proto external float SpeedLimitADS();
	/*!
	Returns the speed limit of the current character while doing an item action
	*/
	proto external float SpeedLimitItemAction();
	/*!
	Returns the speed limit of the current character while have his weapon raised
	*/
	proto external float SpeedLimitHighReady();
	/*!
	Returns true if the ADS speed limit is currently applied
	*/
	proto external bool IsAdsSpeedModApplied();
	/*!
	Returns true if you can't shoot while using one hand
	*/
	proto external bool IsOneHandedShootingSupressed();
	/*!
	Returns true if you can use gadgets while using right hand
	*/
	proto external bool IsGadgetUseAllowed();
	/*!
	Returns true if you can equip gadget while being in vehicle
	*/
	proto external bool CanBeEquippedInVehicle();
	/*!
	Returns true if character can roll and reload weapon at the same time
	*/
	proto external bool IsAllowedReloadingWithRoll();
	/*!
	Returns true if character can jump when item is equipped
	*/
	proto external bool IsJumpingAllowed();
}

/*!
\}
*/
