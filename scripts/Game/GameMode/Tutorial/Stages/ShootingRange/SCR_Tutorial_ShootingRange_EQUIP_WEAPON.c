[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_EQUIP_WEAPONClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_EQUIP_WEAPON : SCR_BaseTutorialStage
{
	IEntity m_CourseWeapon;
	SCR_CharacterInventoryStorageComponent m_StorageComp;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_StorageComp = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		m_CourseWeapon = GetGame().GetWorld().FindEntityByName("M16");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_StorageComp)
			return false;
		
		return m_StorageComp.GetCurrentItem() == m_CourseWeapon;
	}
};