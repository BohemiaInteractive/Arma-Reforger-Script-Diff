[EntityEditorProps(insertable: false)]
class SCR_Tutorial_LRS_Attach_OpticClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_LRS_Attach_Optic : SCR_BaseTutorialStage
{
	WeaponComponent m_WeaponComp;
	CharacterControllerComponent m_CharacterComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		IEntity gun = GetGame().GetWorld().FindEntityByName("Course_M21");
		if (!gun)
			return;
		
		m_WeaponComp = WeaponComponent.Cast(gun.FindComponent(WeaponComponent));
		m_CharacterComponent = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
		
		if (IsOpticAttached())
		{
			m_bFinished = true;
			return;
		}
		
		PlayNarrativeCharacterStage("LONGRANGESHOOTING_Instructor", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName != "SOUND_TUTORIAL_TA_RIFLE_PICKED_UP_INSTRUCTOR_K_02")
			return;
		
		RegisterWaypoint("Course_Optic", "", "GUNLOWER");
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsOpticAttached()
	{
		if (!m_WeaponComp)
			return false;
		
		array <AttachmentSlotComponent> attachments = {};
		m_WeaponComp.GetAttachments(attachments);
		
		return attachments.IsIndexValid(0) && attachments[0].GetAttachedEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		bool opticsAttached = IsOpticAttached();
		
		if (opticsAttached)
		{
			UnregisterWaypoint("Course_Optic");
			return true;
		}
		
		if (m_CharacterComponent)
		{
			if (m_TutorialComponent.IsEntityInPlayerInventory("Course_Optic"))
			{
				if (m_CharacterComponent.GetInspect())
					ShowHint(2);
				else
					ShowHint(1);
			}
		}
		
		return false;
	}
};