class SCR_BaseWeaponAzimuthDisplay : SCR_BaseHeadingDisplay
{
	[Attribute("{CF4D101CDF040CD7}UI/layouts/HUD/MortarInfo/ScopeAzimuthSlots.layout", params: "layout")]
	protected ResourceName m_sAzimuthNotchHolderLayout;

	[Attribute("{0B9BE4C17177DAE5}UI/layouts/HUD/MortarInfo/ScopeAzimuthSteps.layout", params: "layout")]
	protected ResourceName m_sAzimuthNotchItemLayout;

	//------------------------------------------------------------------------------------------------
	void UpdateAzimuthInfo(vector aimRotation)
	{
		float azimuth = aimRotation[0];
		float azimuthMils = azimuth * (m_fMils / 360);

		m_iCurrentHeading = azimuth;

		if (m_wMilsTextWidget)
			m_wMilsTextWidget.SetTextFormat(UIConstants.VALUE_UNIT_MILS, azimuthMils.ToString(4, 0));

		if (m_wDegreesTextWidget)
			m_wDegreesTextWidget.SetTextFormat(UIConstants.VALUE_UNIT_DEGREES, azimuth.ToString(3, 1));

		float azimuthInPercent = azimuth - m_iCurrentHeading;
		azimuthInPercent = Math.Clamp(azimuthInPercent, 0, 1);

		// We didn't move so we don't need to reset
		if (m_iCurrentHeading == m_iLastStoredHeading)
		{
			// Since SetSliderPosPixels() works in DPIScaled we need to first upscale our change to make it work in not FullHD resolutions.
			float newSliderPos = GetGame().GetWorkspace().DPIScale(m_fSliderPosX + (m_fDegreeWidgetSize * azimuthInPercent));

			if (m_wHeadingTableHolderWidget)
				m_wHeadingTableHolderWidget.SetSliderPosPixels(newSliderPos, 0);
		}
		else
		{
			UpdateAzimuthWidgets();
			m_iLastStoredHeading = m_iCurrentHeading;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void CalculateWidgetScreenSizeUnscaled(Widget widget)
	{
		float h;
		widget.GetScreenSize(m_fDegreeWidgetSize, h);
		m_fDegreeWidgetSize = GetGame().GetWorkspace().DPIUnscale(m_fDegreeWidgetSize);

		super.CalculateWidgetScreenSizeUnscaled(widget);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAzimuthWidgets()
	{
		if (m_aHeadingTextWidgets.IsEmpty())
			return;

		// Calculate how many numbers we need to go in every direction from the current azimuth
		int azimuth = m_iCurrentHeading - ((m_iMaxShownDegrees - 1) * 0.5);

		if (azimuth < 0)
			azimuth += 360;

		foreach (TextWidget w : m_aHeadingTextWidgets)
		{
			// If the next number would be 360, change it to 0 since they are both 0°
			if (azimuth == 360)
				azimuth = 0;

			w.SetText(azimuth.ToString());

			azimuth++;
		}

		m_wHeadingTableHolderWidget.SetSliderPos(0.5, 0);
		m_wHeadingTableHolderWidget.GetSliderPosPixels(m_fSliderPosX, m_fSliderPosY);
		m_fSliderPosX = GetGame().GetWorkspace().DPIUnscale(m_fSliderPosX);
	}

	//------------------------------------------------------------------------------------------------
	override void SetupHeadingWidgets(ResourceName headingHolderLayout = "", ResourceName headingNotchLayout = "")
	{
		super.SetupHeadingWidgets(m_sAzimuthNotchHolderLayout, m_sAzimuthNotchItemLayout);
		UpdateAzimuthWidgets();
	}

	//------------------------------------------------------------------------------------------------
	override void CalculateScreenSize(Widget degreeWidget)
	{
		super.CalculateScreenSize(degreeWidget);
		m_fDegreeWidgetDefaultSize = m_fDegreeWidgetDefaultSizeWidth;

		CalculateWidgetScreenSizeUnscaled(degreeWidget);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		vector aimRotation = GetMuzzleDirection().VectorToAngles();
		UpdateAzimuthInfo(aimRotation);

		super.DisplayUpdate(owner, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		SetupHeadingWidgets();
	}
}
