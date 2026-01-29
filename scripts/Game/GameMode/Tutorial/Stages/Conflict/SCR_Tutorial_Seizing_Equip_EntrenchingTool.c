[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_Equip_EntrenchingToolClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_Equip_EntrenchingTool: SCR_BaseTutorialStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	protected ResourceName m_sEToolResource = "{6E35D94130954509}Prefabs/Items/Equipment/Accessories/ETool_ALICE/ETool_ALICE_FreeRoamBuilding_Gadget.et";
	protected IEntity m_ETool;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		PlayNarrativeCharacterStage("CONFLICT_Instructor_C", 6);
		
		if (m_TutorialComponent.FindPrefabInPlayerInventory(m_sEToolResource))
			return;
		
		m_ETool = m_TutorialComponent.SpawnAsset("CONFLICT_ETOOL", m_sEToolResource, null, true);
		RegisterWaypoint(m_ETool, "", "PICKUP");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventory)
			return false;
		
		if (m_ETool && SCR_EntityHelper.GetMainParent(m_ETool) == m_Player)
			UnregisterWaypoint(m_ETool);
		
		IEntity ent = m_PlayerInventory.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_CampaignBuildingGadgetToolComponent gadget = SCR_CampaignBuildingGadgetToolComponent.Cast(ent.FindComponent(SCR_CampaignBuildingGadgetToolComponent));
		return gadget;
	}
};