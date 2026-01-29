class SCR_BaseWeaponElevationDisplay : SCR_BaseHeadingDisplay
{
	[Attribute("{EF0A00F789607F39}UI/layouts/HUD/MortarInfo/ScopeElevationSlots.layout", params: "layout")]
	protected ResourceName m_sElevationNotchHolderLayout;

	[Attribute("{2BDCF42A2713A90B}UI/layouts/HUD/MortarInfo/ScopeElevationSteps.layout", params: "layout")]
	protected ResourceName m_sElevationNotchItemLayout;

	//------------------------------------------------------------------------------------------------
	void UpdateElevationInfo(vector aimRotation)
	{
		float elevation = aimRotation[1];
		float elevationMils = elevation * (m_fMils / 360);

		m_iCurrentHeading = elevation;

		if (m_wMilsTextWidget)
			m_wMilsTextWidget.SetTextFormat(UIConstants.VALUE_UNIT_MILS, elevationMils.ToString(4, 0));

		if (m_wDegreesTextWidget)
			m_wDegreesTextWidget.SetTextFormat(UIConstants.VALUE_UNIT_DEGREES, elevation.ToString(3, 1));

		float elevationInPercent = elevation - m_iCurrentHeading;
		elevationInPercent = Math.Clamp(elevationInPercent, 0, 1);

		// We didn't move so we don't need to reset
		if (m_iCurrentHeading == m_iLastStoredHeading)
		{
			// Since SetSliderPosPixels() works in DPIScaled we need to first upscale our change to make it work in not FullHD resolutions.
			float newSliderPos = GetGame().GetWorkspace().DPIScale(m_fSliderPosY - (m_fDegreeWidgetSize * elevationInPercent));

			if (m_wHeadingTableHolderWidget)
				m_wHeadingTableHolderWidget.SetSliderPosPixels(0, newSliderPos);
		}
		else
		{
			UpdateElevationWidgets();
			m_iLastStoredHeading = m_iCurrentHeading;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateElevationWidgets()
	{
		if (m_aHeadingTextWidgets.IsEmpty())
			return;

		// Calculate how many numbers we need to go in every direction from the current elevation
		int elevation = m_iCurrentHeading + ((m_iMaxShownDegrees - 1) * 0.5);

		if (elevation < 0)
			elevation += 360;

		foreach (TextWidget w : m_aHeadingTextWidgets)
		{
			// If the next number would be 360, change it to 0 since they are both 0°
			if (elevation == 360)
				elevation = 0;
			else if (elevation == -1)
				elevation = 359;

			w.SetText(elevation.ToString());

			elevation--;
		}

		m_wHeadingTableHolderWidget.SetSliderPos(0, 0.5);
		m_wHeadingTableHolderWidget.GetSliderPosPixels(m_fSliderPosX, m_fSliderPosY);
		m_fSliderPosY = GetGame().GetWorkspace().DPIUnscale(m_fSliderPosY);
	}

	//------------------------------------------------------------------------------------------------
	override void SetupHeadingWidgets(ResourceName headingHolderLayout = "", ResourceName headingNotchLayout = "")
	{
		super.SetupHeadingWidgets(m_sElevationNotchHolderLayout, m_sElevationNotchItemLayout);
		UpdateElevationWidgets();
	}

	//------------------------------------------------------------------------------------------------
	override void CalculateWidgetScreenSizeUnscaled(Widget widget)
	{
		float w;
		widget.GetScreenSize(w, m_fDegreeWidgetSize);
		m_fDegreeWidgetSize = GetGame().GetWorkspace().DPIUnscale(m_fDegreeWidgetSize);

		super.CalculateWidgetScreenSizeUnscaled(widget);
	}

	//------------------------------------------------------------------------------------------------
	override void CalculateScreenSize(Widget degreeWidget)
	{
		super.CalculateScreenSize(degreeWidget);
		m_fDegreeWidgetDefaultSize = m_fDegreeWidgetDefaultSizeHeight;

		CalculateWidgetScreenSizeUnscaled(degreeWidget);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		vector aimRotation = GetMuzzleDirection().VectorToAngles();
		UpdateElevationInfo(aimRotation);

		if (m_iCurrentHeading == 0)
		{
			m_iCurrentHeading = aimRotation[1];
			UpdateElevationWidgets();
		}

		super.DisplayUpdate(owner, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		SetupHeadingWidgets();
	}
}
