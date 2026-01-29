[EntityEditorProps(category: "GameScripted/Components", description: "")]
class SCR_ExplosiveChargeInventoryItemComponentClass : SCR_PlaceableInventoryItemComponentClass
{
}

class SCR_ExplosiveChargeInventoryItemComponent : SCR_PlaceableInventoryItemComponent
{
	//------------------------------------------------------------------------------------------------
	protected override bool ShouldHideInVicinity()
	{
		return IsLocked();
	}
}
