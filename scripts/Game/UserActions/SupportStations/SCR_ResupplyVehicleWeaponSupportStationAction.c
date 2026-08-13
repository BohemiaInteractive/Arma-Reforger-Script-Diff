class SCR_ResupplyVehicleWeaponSupportStationAction : SCR_BaseItemHolderSupportStationAction
{
	[Attribute("1", desc: "If the action is hidden if the entity does not have a parent")]
	protected bool m_bHideIfNoParent;

	[Attribute(defvalue: "1.5", desc: "Value by which action progress is going to be multiplied to speed it up when character is a vehicle crewman", params: "0.1 inf 0.01")]
	protected float m_fQualifiedPersonnelBonusUseSpeedFactor;

	protected bool m_bIsQualified;

	protected const LocalizedString X_OUTOF_Y_FORMATTING = "#AR-SupportStation_ActionFormat_ItemAmount";
	
	protected const LocalizedString INVALID_FULL = "#AR-SupportStation_ActionInvalid_VehicleWeaponFullyReloaded";
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		//~ Not valid or has no parent
		if (m_bHideIfNoParent && (!GetOwner() || !GetOwner().GetParent()))
			return false;
		
		return super.CanBeShownScript(user);		
	}
	
	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		if (reasonInvalid == ESupportStationReasonInvalid.RESUPPLY_INVENTORY_FULL)
			return INVALID_FULL;
		
		return super.GetInvalidPerformReasonString(reasonInvalid);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return super.CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.VEHICLE_WEAPON;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		SCR_ResupplyItemSupportStationData itemData = SCR_ResupplyItemSupportStationData.Cast(m_ResupplyData);
		if (!itemData)
			return;
		
		m_sItemPrefab = itemData.GetItemPrefab();
		SetItemName();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo uiInfo = GetUIInfo();
		if (!uiInfo)
			return super.GetActionNameScript(outName);
					
		outName = WidgetManager.Translate(uiInfo.GetName(), m_sItemName);
		
		return super.GetActionNameScript(outName);
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
		return {SCR_EGroupRole.TRANSPORT, SCR_EGroupRole.HELI_TRANSPORT};
	}

	//------------------------------------------------------------------------------------------------
	//! Override this if you would want to change which character label should make a character qualified
	protected array<EEditableEntityLabel> GetQualifiedLabels()
	{
		return {EEditableEntityLabel.TRAIT_VEHICLE_CREW, EEditableEntityLabel.TRAIT_HELI_CREW};
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