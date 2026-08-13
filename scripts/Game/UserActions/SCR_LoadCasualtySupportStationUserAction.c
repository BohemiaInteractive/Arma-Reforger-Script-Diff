class SCR_LoadCasualtySupportStationUserAction : SCR_BaseUseSupportStationAction
{
	[Attribute("#AR-UserAction_LoadCasualty_NoSpace", desc: "Text shown when character can not be loaded as there is no space in the vehicle", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidNoSpace;

	[Attribute(defvalue: "2", desc: "Value by which action progress is going to be multiplied to speed it up when character is a medic", params: "0.1 inf 0.01")]
	protected float m_fQualifiedPersonnelBonusUseSpeedFactor;

	protected bool m_bIsQualified;	
	protected CharacterControllerComponent m_CharacterController;
	protected FactionAffiliationComponent m_FactionAffiliation;
		
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.LOAD_CASUALTY;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return;
		
		m_CharacterController = character.GetCharacterController();
		m_FactionAffiliation = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		//~ No space to load casualty
		if (reasonInvalid == ESupportStationReasonInvalid.LOAD_CASUALTY_NO_SPACE)
			return m_sInvalidNoSpace;
		
		return super.GetInvalidPerformReasonString(reasonInvalid);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		if (!m_CharacterController || m_CharacterController.GetLifeState() != ECharacterLifeState.INCAPACITATED)
			return false;
		
		if (m_FactionAffiliation)
		{
			FactionAffiliationComponent userFactionAffiliation = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
			if (userFactionAffiliation)
			{
				//~ Do not show if character is an enemy
				Faction ownerFaction = m_FactionAffiliation.GetAffiliatedFaction();
				if (ownerFaction && ownerFaction.IsFactionEnemy(userFactionAffiliation.GetAffiliatedFaction()))
					return false;
			}
		}
		
		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
		m_bIsQualified = character && (character.HasRole(GetQualifiedRoles()) || character.HasLabel(GetQualifiedLabels()));

		super.OnActionStart(pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Override this if you would want to change which squad roles should make a character qualified
	protected array<SCR_EGroupRole> GetQualifiedRoles()
	{
		return {SCR_EGroupRole.MEDIC};
	}

	//------------------------------------------------------------------------------------------------
	//! Override this if you would want to change which character label should make a character qualified
	protected array<EEditableEntityLabel> GetQualifiedLabels()
	{
		return {EEditableEntityLabel.ROLE_MEDIC};
	}

	//------------------------------------------------------------------------------------------------
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		if (m_bIsQualified)
			timeSlice *= m_fQualifiedPersonnelBonusUseSpeedFactor;

		return super.GetActionProgressScript(fProgress, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected override bool LoopActionUpdate(float timeSlice)
	{
		if (m_bIsQualified)
			timeSlice *= m_fQualifiedPersonnelBonusUseSpeedFactor;

		return super.LoopActionUpdate(timeSlice);
	}
}
