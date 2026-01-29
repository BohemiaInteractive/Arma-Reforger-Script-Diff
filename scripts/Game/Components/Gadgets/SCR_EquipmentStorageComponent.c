class SCR_EquipmentStorageComponentClass: EquipmentStorageComponentClass
{
}

//! Equipment storage are slots which are part of their parent entity but not taking up the standard cargo space, such as slot for flashlight in the form of a strap on the vest or belt 
class SCR_EquipmentStorageComponent : EquipmentStorageComponent
{	
	//------------------------------------------------------------------------------------------------
	//! Will be called to estimate if storage children has to be included in preview
	override protected bool ShouldPreviewAttachedItems()
	{
		return false;
	}
}

class SCR_EquipmentStorageSlot : EquipmentStorageSlot
{
	//------------------------------------------------------------------------------------------------
	override protected bool OnOccludedStateChanged(bool occluded) 
	{ 
		IEntity ent = GetAttachedEntity();
		if (ent)
		{
			SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(ent.FindComponent(SCR_GadgetComponent));
			if (!gadgetComp)
				return false;

			gadgetComp.OnSlotOccludedStateChanged(occluded);

			//return false to let the slot handle occlusion if the gadget can be visible
			//return true if it is slot which is not visualized, dont do any visibility changes (keep invis)
			return !gadgetComp.IsVisibleEquipped();
		}

		return false; 
	}
}
