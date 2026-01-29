[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_ENABLE_RADIOClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_ENABLE_RADIO : SCR_BaseTutorialStage
{
	BaseRadioComponent m_RadioComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_InventoryStorageManagerComponent invComp = m_TutorialComponent.GetPlayerInventory();
		if (!invComp)
			return;
		
		array <typename> components = {};
		components.Insert(BaseRadioComponent);
		
		IEntity radio = invComp.FindItemWithComponents(components);
		if (!radio)
			return;
		
		m_RadioComponent = BaseRadioComponent.Cast(radio.FindComponent(BaseRadioComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_RadioComponent)
			return false;
		
		return m_RadioComponent.IsPowered();
	}
};