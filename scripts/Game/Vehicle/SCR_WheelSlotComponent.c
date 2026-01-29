#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
class SCR_WheelSlotComponentClass: BaseSlotComponentClass
{
};

class SCR_WheelSlotComponent : BaseSlotComponent
{
	[Attribute("0", UIWidgets.Slider, "Corresponding index to the wheel definition in VehicleWheeledSimulation (-1 means ignore)", "-1 1000 1")]
	int m_iWheelIndex;
#ifdef ENABLE_BASE_DESTRUCTION
	//------------------------------------------------------------------------------------------------
	override void OnAttachedEntity(IEntity ent)
	{
		SCR_DestructionTireComponent tire = SCR_DestructionTireComponent.Cast(ent.FindComponent(SCR_DestructionTireComponent));
		if (tire)
			tire.SetWheelIndex(m_iWheelIndex);
	}
#endif
};
