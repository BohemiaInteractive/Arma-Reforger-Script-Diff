[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_PICKUP_WEAPONClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_PICKUP_WEAPON : SCR_BaseTutorialStage
{
	IEntity m_CourseWeapon;
	SCR_CharacterInventoryStorageComponent m_StorageComp;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_CourseWeapon = GetGame().GetWorld().FindEntityByName("M16");
		if (m_CourseWeapon)
			RegisterWaypoint(m_CourseWeapon, "", "PICKUP");

		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 2);

		m_StorageComp = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_StorageComp && m_StorageComp.GetCurrentItem() == m_CourseWeapon)
			return true;
		
		if (m_TutorialComponent.IsEntityInPlayerInventory(m_CourseWeapon))
		{
			m_TutorialComponent.InsertStage("EQUIP_WEAPON");
			return true;
		}
		
		return false;
	}
};