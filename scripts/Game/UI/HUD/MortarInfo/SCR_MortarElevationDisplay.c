class SCR_MortarElevationDisplay : SCR_BaseWeaponElevationDisplay
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
