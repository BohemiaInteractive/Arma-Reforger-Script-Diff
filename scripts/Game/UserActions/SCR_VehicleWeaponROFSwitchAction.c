class SCR_VehicleWeaponROFSwitchAction : SCR_ScriptedUserAction
{
	[Attribute(defvalue: "", desc: "ID of the weapon for which this action will be available")]
	protected ref array<int> m_aWeaponIds;

	//------------------------------------------------------------------------------------------------
	protected BaseMuzzleComponent GetCurrentMuzzle()
	{
		TurretControllerComponent turretCtrl = TurretControllerComponent.Cast(GetOwner().FindComponent(TurretControllerComponent));
		if (!turretCtrl)
			return null;

		BaseWeaponManagerComponent weaponMgr = turretCtrl.GetWeaponManager();
		if (!weaponMgr)
			return null;

		WeaponSlotComponent weaponSlot = weaponMgr.GetCurrentSlot();
		if (!weaponSlot)
			return null;

		if (!m_aWeaponIds.Contains(weaponSlot.GetWeaponSlotIndex()))
			return null;

		return weaponSlot.GetCurrentMuzzle();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		BaseMuzzleComponent muzzle = GetCurrentMuzzle();
		return muzzle && muzzle.GetFireModesCount() > 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);

		BaseMuzzleComponent muzzle = GetCurrentMuzzle();
		if (!muzzle)
			return;

		muzzle.SetFireMode(muzzle.GetNextFireModeIndex());
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo info = GetUIInfo();
		if (!info)
			return false;

		BaseMuzzleComponent muzzle = GetCurrentMuzzle();
		if (!muzzle)
			return false;

		BaseFireMode currentMode = muzzle.GetCurrentFireMode();
		if (!currentMode)
			return false;

		string modeName = currentMode.GetUIName();
		if (modeName.IsEmpty())
			return false;

		int rpm = modeName.ToInt();
		if (rpm > 0)
			ActionNameParams[0] = WidgetManager.Translate(UIConstants.VALUE_UNIT_RPM, rpm);
		else
			ActionNameParams[0] = modeName;
		
		outName = info.GetName();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
}
