[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_MoonPhaseUIInfo : SCR_UIInfo
{
	[Attribute()]
	protected LocalizedString m_sSimplifiedName;
	
	protected float m_fMoonphaseImageRotation = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Get Simplified Name rather then full Waxing or waning Crescent it will just return Crescent
	//! \return simplified moon phase
	string GetSimplifiedName()
	{
		return m_sSimplifiedName;
	}

	//------------------------------------------------------------------------------------------------
	//! Set rotation of Moon image. This is set to 0 if Northern Hemisphere and 180 if southern Hemisphere
	//! \param if is northern or southern Hemisphere
	void SetMoonphaseImageRotation(bool northernHemisphere)
	{
		if (northernHemisphere)
			m_fMoonphaseImageRotation = 0;
		else 
			m_fMoonphaseImageRotation = 180;
	}

	//------------------------------------------------------------------------------------------------
	//! Get rotation of Moon image. This will return 0 if UIInfo was set to northern Hemisphere and 180 if southern Hemisphere
	//! Use TimeAndWeatherManagerEntity.GetMoonPhaseInfoForDate() to make sure the value is set correctly
	//! \return rotation of image
	float GetMoonphaseImageRotation()
	{
		return m_fMoonphaseImageRotation;
	}
}
