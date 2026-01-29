[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_EquipEntrenchingToolClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_EquipEntrenchingTool: SCR_BaseTutorialStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	protected ResourceName m_sEToolResource = "{6E35D94130954509}Prefabs/Items/Equipment/Accessories/ETool_ALICE/ETool_ALICE_FreeRoamBuilding_Gadget.et";
	protected IEntity m_ETool, m_SpawnedEntity;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		PlayNarrativeCharacterStage("COMBATENGINEERING_Instructor_E", 3);
		
		m_SpawnedEntity = GetGame().GetWorld().FindEntityByName("BUILDING_GUNNEST");
		
		if (m_TutorialComponent.FindPrefabInPlayerInventory(m_sEToolResource))
			return;
		
		m_ETool = m_TutorialComponent.SpawnAsset("BUILDING_ETOOL", m_sEToolResource, null, true);
		RegisterWaypoint(m_ETool, "", "PICKUP");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_SpawnedEntity)
		{
			m_TutorialComponent.SetStage("ENTER_BUILDING_2");
			return false;
		}
		
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