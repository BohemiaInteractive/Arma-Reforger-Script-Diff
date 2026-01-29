[BaseContainerProps()]
class SCR_FPSNetworkedStat : SCR_BaseNetworkedStat
{
	// Last checked local FPS.
	protected int m_iLocalFPS;
	// Last checked authority FPS.
	protected int m_iAuthorityFPS;

	//------------------------------------------------------------------------------------------------
	int GetLastCheckedLocalFPS()
	{
		return m_iLocalFPS;
	}

	//------------------------------------------------------------------------------------------------
	int GetLastCheckedAuthorityFPS()
	{
		return m_iAuthorityFPS;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateStat()
	{
		int localFPS = System.GetFPS();
		if (localFPS == m_iLocalFPS)
			return;

		m_iLocalFPS = localFPS;
		UpdateStatValueIfAuthority();
		InvokeStatValueChange();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateStatValueIfAuthority()
	{
		RplComponent rplComponent = m_Owner.GetRplComponent();
		if (!rplComponent || rplComponent.Role() != RplRole.Authority)
			return;

		m_iAuthorityFPS = m_iLocalFPS;
		m_Owner.SetAuthorityStatValue(m_eStatType, m_iAuthorityFPS);
	}

	//------------------------------------------------------------------------------------------------
	override void OnAuthorityStatValueSet(int value)
	{
		m_iAuthorityFPS = value;
		InvokeStatValueChange();
	}
}
