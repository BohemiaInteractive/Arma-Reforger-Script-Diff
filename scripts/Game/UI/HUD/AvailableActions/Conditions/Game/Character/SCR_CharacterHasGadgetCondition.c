//------------------------------------------------------------------------------------------------
//! Returns true if character has provided gadget in their inventory
//! if m_bEquipped true is only returned if the gadget is currently held
[BaseContainerProps()]
class SCR_CharacterHasGadgetCondition : SCR_AvailableActionCondition
{
	[Attribute("1", UIWidgets.CheckBox, "Only if gadget is in hands", "")]
	protected bool m_bEquipped;

	[Attribute("0", UIWidgets.CheckBox, "Only if gadget is not in hands and in ", "")]
	protected bool m_bNotEquipped;

	[Attribute("1", UIWidgets.CheckBox, "Specified gadget type", "")]
	protected bool m_bCheckSpecificGadget;

	[Attribute(defvalue: SCR_Enum.GetDefault(EGadgetType.MAP), UIWidgets.ComboBox, "Gadget type", "", ParamEnumArray.FromEnum(EGadgetType))]
	protected EGadgetType m_eGadget;

	[Attribute("0", UIWidgets.CheckBox, "Pass if the gadget is turned on", "")]
	protected bool m_bIsToggledOn;

	[Attribute("0", UIWidgets.CheckBox, "Pass if the gadget is turned off", "")]
	protected bool m_bIsToggledOff;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has specified gadget
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool equipped;

		SCR_GadgetComponent gadgetComponent = data.GetHeldGadgetComponent();
		if (gadgetComponent)
		{
			if (m_bCheckSpecificGadget)
				equipped = gadgetComponent.GetType() == m_eGadget;
			else
				equipped = true;
		}

		// Force failure if expected as unequipped while gadget is equipped
		if (equipped && m_bNotEquipped)
			return false;

		bool result;
		if (m_bEquipped)
		{
			result = equipped;
		}
		else
		{
			// Check gadgets that are not in hands
			gadgetComponent = null;

			// Force failure if there is no gadget at that point
			IEntity gadget = data.GetGadget(m_eGadget);
			if (!gadget)
				return false;

			// Get new gadget component or clear
			if (m_bIsToggledOn || m_bIsToggledOff)
				gadgetComponent = SCR_GadgetComponent.Cast(gadget.FindComponent(SCR_GadgetComponent));

			result = true;
		}

		// Allow only toggled on or off
		if (result && (m_bIsToggledOn || m_bIsToggledOff))
		{
			// Pass if either requirement is met
			bool isToggledOn = gadgetComponent && gadgetComponent.IsToggledOn();
			if (isToggledOn && m_bIsToggledOn)
				result = true;
			else if (!isToggledOn && m_bIsToggledOff)
				result = true;
			else
				result = false;
		}

		return GetReturnResult(result);
	}
}
