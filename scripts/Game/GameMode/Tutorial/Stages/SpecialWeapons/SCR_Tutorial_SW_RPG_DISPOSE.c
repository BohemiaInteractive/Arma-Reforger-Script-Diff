[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_RPG_DISPOSEClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_SW_RPG_DISPOSE : SCR_BaseTutorialStage
{
	protected SCR_InventoryStorageManagerComponent m_PlayerInventory;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_PlayerInventory = m_TutorialComponent.GetPlayerInventory();
		IEntity courseRPG = GetGame().GetWorld().FindEntityByName("COURSE_RPG");

		if (!courseRPG || !m_PlayerInventory)
		{
			m_bFinished = true;
			return;
		}

		m_TutorialComponent.EnableRefunding(courseRPG, true);

		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 11);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsLauncherDropped()
	{
		if (!m_PlayerInventory)
			return false;

		array<IEntity> foundItems = {};		
		m_PlayerInventory.FindItemsWithComponents(foundItems, {BaseWeaponComponent}, EStoragePurpose.PURPOSE_ANY);
		foreach (IEntity item : foundItems)
		{
			const BaseWeaponComponent weapon = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));
			if (weapon.GetWeaponType() == EWeaponType.WT_ROCKETLAUNCHER)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return IsLauncherDropped();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_RPG_DISPOSE()
	{
		if (m_TutorialComponent)
			m_TutorialComponent.EnableArsenal("SW_ARSENAL_USSR", false);
	}
}
