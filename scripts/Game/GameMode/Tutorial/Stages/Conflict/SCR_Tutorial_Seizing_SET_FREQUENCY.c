[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_SET_FREQUENCYClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_SET_FREQUENCY : SCR_BaseTutorialStage
{
	const int DESIRED_FREQUENCY = 60000;
	protected ResourceName m_sRadioPrefab = "{73950FBA2D7DB5C5}Prefabs/Items/Equipment/Radios/Radio_ANPRC68.et";
	protected BaseRadioComponent m_RadioComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		SCR_InventoryStorageManagerComponent invComp = m_TutorialComponent.GetPlayerInventory();
		if (!invComp)
			return;
		
		PlayNarrativeCharacterStage("CONFLICT_Instructor_B", 5);
		
		IEntity radio = m_TutorialComponent.FindPrefabInPlayerInventory(m_sRadioPrefab);
		if (radio)
			return;
		
		radio = m_TutorialComponent.SpawnAsset("Seizing_RADIO", m_sRadioPrefab, null, true);
		if (radio)
			RegisterWaypoint(radio, "", "PICKUP");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FindPlayerRadio()
	{
		if (!m_TutorialComponent)
			return;
		
		IEntity radio = m_TutorialComponent.FindPrefabInPlayerInventory(m_sRadioPrefab);
		if (!radio)
			return;
		
		UnregisterWaypoint("Seizing_RADIO");
		
		m_RadioComponent = BaseRadioComponent.Cast(radio.FindComponent(BaseRadioComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		FindPlayerRadio();
		
		if (!m_RadioComponent || !m_RadioComponent.IsPowered())
			return false;
		
		BaseTransceiver transceiver = m_RadioComponent.GetTransceiver(0);
		if (transceiver && transceiver.GetFrequency() == DESIRED_FREQUENCY)
			return true;
		
		transceiver = m_RadioComponent.GetTransceiver(1);
		if (transceiver && transceiver.GetFrequency() == DESIRED_FREQUENCY)
			return true;
		
		return false;
	}
};