class SCR_CampaignOperationsMapUIBase : SCR_CampaignMapUIBase
{
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_Base)
		{
			m_Base.GetOnEnemyPresenceChanged().Insert(OnEnemyPresenceChanged);
			SetOpenButtonEnabled(!m_Base.AreEnemiesPresent());
		}

		return super.OnMouseEnter(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_Base)
			m_Base.GetOnEnemyPresenceChanged().Remove(OnEnemyPresenceChanged);

		return super.OnMouseLeave(w, enterW, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DeinitOpenButton(Widget w)
	{
		super.DeinitOpenButton(w);

		if (m_Base)
			m_Base.GetOnEnemyPresenceChanged().Remove(OnEnemyPresenceChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Enables or disables button for entering the build mode
	//! \param[in] enable
	protected void SetOpenButtonEnabled(bool enable)
	{
		if (m_OpenInputButtonComponent)
			m_OpenInputButtonComponent.SetEnabled(enable);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEnemyPresenceChanged(bool enemyInBase)
	{
		SetOpenButtonEnabled(!enemyInBase);
	}
}