class SCR_ResupplyVehicleWeaponSupportStationAction : SCR_BaseItemHolderSupportStationAction
{
	[Attribute("1", desc: "If the action is hidden if the entity does not have a parent")]
	protected bool m_bHideIfNoParent;

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
}