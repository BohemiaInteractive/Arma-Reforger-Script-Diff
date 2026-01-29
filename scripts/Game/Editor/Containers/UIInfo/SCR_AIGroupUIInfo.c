class SCR_AIGroupUIInfo : SCR_UIInfo
{
	protected ResourceName m_sGroupFlag;
	protected bool m_bFlagIsFromImageSet;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param value
	void SetGroupFlag(ResourceName value)
	{
		m_sGroupFlag = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return group flag's resourcename
	ResourceName GetGroupFlag()
	{
		return m_sGroupFlag;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param value
	void SetFlagIsFromImageSet(bool value)
	{
		m_bFlagIsFromImageSet = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if flag is from an imageset, false if not
	bool GetFlagIsFromImageSet()
	{
		return m_bFlagIsFromImageSet;
	}
}