class SCR_PreviewRichText : SCR_InfoDisplayExtended
{
	protected TextWidget m_wGlowText;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		/*
		m_wGlowText = TextWidget.Cast(m_wRoot.FindAnyWidget("14 RobotoCondensed_Regular-96-16"));
		m_wGlowText.SetShadow(24, 0xFFFFFFFF);
		
		m_wGlowText = TextWidget.Cast(m_wRoot.FindAnyWidget("14 RobotoCondensed_Regular-64-32"));
		m_wGlowText.SetShadow(24, 0xFFFFFFFF);
		
		m_wGlowText = TextWidget.Cast(m_wRoot.FindAnyWidget("14 RobotoCondensed_Regular-64-16"));
		m_wGlowText.SetShadow(24, 0xFFFFFFFF);
		
		m_wGlowText = TextWidget.Cast(m_wRoot.FindAnyWidget("14 RobotoCondensed_Regular-64-24"));
		m_wGlowText.SetShadow(24, 0xFFFFFFFF);		
		*/

		/*
		m_wGlowText = TextWidget.Cast(m_wRoot.FindAnyWidget("14 RobotoCondensed_Regular-96-16"));
		m_wGlowText.SetShadow(32, 0xFF000000, 0.5);
		
		m_wGlowText = TextWidget.Cast(m_wRoot.FindAnyWidget("14 RobotoCondensed_Regular-64-32"));
		m_wGlowText.SetShadow(32, 0xFF000000, 0.5);
		
		m_wGlowText = TextWidget.Cast(m_wRoot.FindAnyWidget("14 RobotoCondensed_Regular-64-16"));
		m_wGlowText.SetShadow(32, 0xFF000000, 0.5);
		
		m_wGlowText = TextWidget.Cast(m_wRoot.FindAnyWidget("14 RobotoCondensed_Regular-64-24"));
		m_wGlowText.SetShadow(32, 0xFF000000, 0.5);
		*/
	}	
		
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
	}
};
