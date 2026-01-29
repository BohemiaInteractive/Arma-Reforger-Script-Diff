[BaseContainerProps()]
class SCR_ExtendedCharacterIdentity : SCR_ExtendedIdentity
{		
	[Attribute("-1", desc: "Array of blood types is defined in Extended Identity manager. Leave AUTO to auto assign", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EBloodType))]
	protected SCR_EBloodType m_eBloodType;
	
	protected int m_iDayOfDeath = -1;
	protected int m_iMonthOfDeath = -1;
	protected int m_iYearOfDeath = -1;
	protected int m_iHourOfDeath = -1;
	protected int m_iMinuteOfDeath = -1;
		
	//------------------------------------------------------------------------------------------------
	//! Set date and time of death
	//! Use SCR_ExtendedCharacterIdentityComponent to set to make sure it is replicated!
	//! \param[in] deathDay Day of death
	//! \param[in] deathMonth Month of death
	//! \param[in] deathYear Year of death
	//! \param[in] deathHour Hour of death
	//! \param[in] deathMinute Minute of death
	void SetDeathDateAndTime(int deathDay, int deathMonth, int deathYear, int deathHour, int deathMinute)
	{
		m_iDayOfDeath = deathDay;
		m_iMonthOfDeath = deathMonth;
		
		if (deathYear > 4000)
		{
			Print(string.Format("'SCR_ExtendedCharacterIdentity' SetDeathDateAndTime tries to set year of death to %1 but because it would be higher than int.MAX as it does more then 4000 years in minutes. The value is set to 4000 instead!", deathYear), LogLevel.WARNING);
			deathYear = 4000;
		}
		m_iYearOfDeath = deathYear;
		m_iHourOfDeath = deathHour;
		m_iMinuteOfDeath = deathMinute;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get date and time of death
	//! \param[out] deathDay Day of death
	//! \param[out] deathMonth Month of death
	//! \param[out] deathYear Year of death
	//! \param[out] deathHour Hour of death
	//! \param[out] deathMinute Minute of death
	//! \return false if no date and/or time is set
	bool GetDeathDateAndTime(out int deathDay, out int deathMonth, out int deathYear, out int deathHour, out int deathMinute)
	{
		deathDay = m_iDayOfDeath;
		deathMonth = m_iMonthOfDeath;
		deathYear = m_iYearOfDeath;
		deathHour = m_iHourOfDeath;
		deathMinute = m_iMinuteOfDeath;
		
		return m_iDayOfDeath > 0 && m_iMonthOfDeath > 0 && m_iYearOfDeath > -1 && m_iHourOfDeath > -1 && m_iMinuteOfDeath > -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Blood type
	//! Use SCR_ExtendedCharacterIdentityComponent to set to make sure it is replicated!
	//! \param[in] bloodType Blood type to set
	void SetBloodType(SCR_EBloodType bloodType)
	{
		m_eBloodType = bloodType;
	}

	//------------------------------------------------------------------------------------------------
	/*
	//! Get bloodtype of character. Use SCR_IdentityManagerComponent to get UIinfo of bloodtype
	//! \return Bloodtype
	*/
	SCR_EBloodType GetBloodType()
	{
		return m_eBloodType;
	}	
}