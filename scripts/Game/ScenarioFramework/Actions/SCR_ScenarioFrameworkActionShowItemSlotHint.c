[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowItemSlotHint : SCR_ScenarioFrameworkActionShowHint
{
	[Attribute(desc: "Get an item you want to find in player quick slots.")]
	ref SCR_ScenarioFrameworkGet m_ItemGetter;
	
	[Attribute()]
	string m_sItemDescriptionText;
	
	//------------------------------------------------------------------------------------------------
	override void ComposeHintText()
	{
		m_sText = m_sText + DOUBLE_LINE_SPACE +  m_sItemDescriptionText + DOUBLE_LINE_SPACE + string.Format("<action name='%1' scale='1.7'/>", GetQuickSlotAction());
		
		super.ComposeHintText();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetQuickSlotAction()
	{
		IEntity item;
		if (m_ItemGetter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_ItemGetter.Get());
			if (!entityWrapper)
				return string.Empty;
			
			item  = entityWrapper.GetValue();
		}
		
		SCR_CharacterInventoryStorageComponent PlayerInventoryManager = SCR_CharacterInventoryStorageComponent.Cast(EntityUtils.GetPlayer().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!PlayerInventoryManager)
			return string.Empty;
		
		// Index needs to be increased +1 as the slots are numbered 1 to 10.
		int itemIndex = PlayerInventoryManager.GetEntityIndexInQuickslots(item) +1;
		
		return "InventoryQuickSlot" + string.ToString(itemIndex);
	}
}
