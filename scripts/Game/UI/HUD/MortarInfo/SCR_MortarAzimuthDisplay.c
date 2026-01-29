class SCR_MortarAzimuthDisplay : SCR_BaseWeaponAzimuthDisplay
{
	protected SCR_MortarInfo m_MortarInfo;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);

		m_MortarInfo = SCR_MortarInfo.Cast(m_pParentDisplay);

		if (m_MortarInfo)
			m_fMils = m_MortarInfo.GetMils();
	}
}
