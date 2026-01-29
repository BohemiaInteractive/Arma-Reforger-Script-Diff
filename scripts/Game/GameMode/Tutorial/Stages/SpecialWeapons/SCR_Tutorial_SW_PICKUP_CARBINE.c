[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_PICKUP_CARBINEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_PICKUP_CARBINE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		if (m_TutorialComponent.FindPrefabInPlayerInventory("{FAED8801F4AAE136}Prefabs/Weapons/Rifles/M16/Rifle_M16A2_carbine_M203_Tutorial.et"))
		{
			m_bFinished = true;
			return;
		}
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		
		RegisterWaypoint("COURSE_Carbine", "", "PICKUP");

		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
		if (!weaponComp)
			return false;
		 
		array <AttachmentSlotComponent> attachments = {};
		weaponComp.GetAttachments(attachments);
		
		if (!attachments.IsEmpty())
		{
			foreach (AttachmentSlotComponent slot : attachments)
			{
				if (!slot.GetAttachmentSlotType().IsInherited(AttachmentUnderBarrel))
					continue;
				
				m_bFinished = true;
				return true;
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_PICKUP_CARBINE()
	{
		if (!m_Player)
			return;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
	}
};