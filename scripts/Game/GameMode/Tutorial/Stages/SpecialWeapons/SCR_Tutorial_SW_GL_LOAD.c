[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_GL_LOADClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_GL_LOAD : SCR_BaseTutorialStage
{
	MuzzleInMagComponent m_MuzzleInMagComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		RegisterWaypoint("SW_ARSENAL_AMMO", "", "GUNUP");
		
		IEntity carbine = GetGame().GetWorld().FindEntityByName("COURSE_Carbine");
		if (!carbine)
			return;
		
		BaseWeaponComponent weaponComponent = BaseWeaponComponent.Cast(carbine.FindComponent(BaseWeaponComponent));
		if (!weaponComponent)
			return;
		
		array <AttachmentSlotComponent> attachments = {};
		weaponComponent.GetAttachments(attachments);
		if (!attachments || attachments.IsEmpty())
			return;
		
		IEntity ugl;
		foreach (AttachmentSlotComponent attachment : attachments)
		{
			if (attachment.GetAttachmentSlotType().Type() != AttachmentUnderBarrelM203)
				continue;
			
			ugl = attachment.GetAttachedEntity();
			break;
		}
		
		m_MuzzleInMagComponent = MuzzleInMagComponent.Cast(ugl.FindComponent(MuzzleInMagComponent));

		m_TutorialComponent.EnableArsenal("SW_ARSENAL_AMMO", true);
		
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_MuzzleInMagComponent)
			return false;
		
		return m_MuzzleInMagComponent.IsCurrentBarrelChambered();
	}
	
};