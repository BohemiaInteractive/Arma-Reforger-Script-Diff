class SCR_WeaponGroup : WeaponsGroup
{
	
	string m_sWeaponGroupName = "";
	ref array<int> m_aWeaponsGroupIds = {};
	SCR_EWeaponGroupFireMode m_eFireMode;
	ref array<int> m_aRippleFireQuantities = {};
	
	void SCR_WeaponGroup()
	{
	}
	
	void InitFromWeaponsGroup(WeaponsGroup data)
	{
		if (!data)
			return;
		
		m_sWeaponGroupName = data.GetName();
		data.GetWeaponIndices(m_aWeaponsGroupIds);
		data.GetAvailableRippleQuantities(m_aRippleFireQuantities);
		m_eFireMode = data.GetFireMode();
	}
	
	void InitFromPrefab(FireModeManagerComponentClass data, int index)
	{
		if (!data)
			return;
		
		m_sWeaponGroupName = data.GetWeaponsGroupName(index);
		data.GetWeaponIndices(index, m_aWeaponsGroupIds);
		data.GetAvailableRippleQuantities(index, m_aRippleFireQuantities);
		m_eFireMode = data.GetFireMode(index);
	}
}