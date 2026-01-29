class SCR_AIToggleFlashlights : AITaskScripted
{
	[Attribute("true", UIWidgets.CheckBox, "Set flashlights active or inactive?")]
	protected bool m_bEnable;
	
	protected SCR_GadgetManagerComponent m_GadgetManager;
	protected SCR_AICharacterSettingsComponent m_Settings;
	
	//------------------------------------------------------------------------------------------------------
	override static string GetOnHoverDescription()
	{
		return "Sets all flashlights attached to vest enabled or disabled. Returns failure if no suitable flashlight found.";
	}
	
	
	//------------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(owner.GetControlledEntity());
		m_Settings = SCR_AICharacterSettingsComponent.Cast(owner.FindComponent(SCR_AICharacterSettingsComponent));
	}
	
	//------------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		return string.Format("Flashlights Enabled: %1", m_bEnable);
	}
	
	//------------------------------------------------------------------------------------------------------
	ENodeResult ToggleFlashlights(notnull SCR_GadgetManagerComponent gadgetManager, SCR_AICharacterSettingsComponent settingsComp,  bool newState)
	{
		array<SCR_GadgetComponent> gadgets = gadgetManager.GetGadgetsByType(EGadgetType.FLASHLIGHT);
		
		if (!gadgets)
			return ENodeResult.FAIL;
		
		if (gadgets.IsEmpty())
			return ENodeResult.FAIL;
		
		// Bail if settings don't allow us to interact with lights
		if (settingsComp)
		{
			SCR_AICharacterLightInteractionSettingBase setting = SCR_AICharacterLightInteractionSettingBase.Cast(settingsComp.GetCurrentSetting(SCR_AICharacterLightInteractionSettingBase));
			if (setting && !setting.IsLightInterractionAllowed())
				return ENodeResult.SUCCESS;
		}
		
		bool success = false;
		foreach (SCR_GadgetComponent gadget : gadgets)
		{
			if (!gadget)
				continue;

			InventoryItemComponent invComp = InventoryItemComponent.Cast(gadget.GetOwner().FindComponent(InventoryItemComponent));
			
			if (!invComp)
				continue;
			
			EquipmentStorageSlot slot = EquipmentStorageSlot.Cast(invComp.GetParentSlot());
			
			// Don't toggle gadgets not in slot
			if (!slot)
				continue;
			
			bool occluded = slot.IsOccluded();
			bool newToggledState = newState && !occluded;
			bool currentState = gadget.IsToggledOn();
			
			if (newToggledState != gadget.IsToggledOn())
				gadget.ToggleActive(newToggledState, SCR_EUseContext.FROM_ACTION);
			
			success |= true;
		}
			
		if (success)
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------------
	static protected override bool VisibleInPalette()
	{
		return false;
	}
};



class SCR_AIToggleFlashlightsOnSimulate : SCR_AIToggleFlashlights
{
	//------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{	
		if (!m_GadgetManager)
			return ENodeResult.FAIL;
		
		return ToggleFlashlights(m_GadgetManager, m_Settings, m_bEnable);
	};
	
	//------------------------------------------------------------------------------------------------------
	override static protected bool VisibleInPalette()
	{
		return true;
	}
};



class SCR_AIToggleFlashlightsOnAbort : SCR_AIToggleFlashlights
{
	//------------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (!m_GadgetManager)
			return;
		
		ToggleFlashlights(m_GadgetManager, m_Settings, m_bEnable);
	}
	
	//------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------------
	override static protected bool VisibleInPalette()
	{
		return true;
	}
};