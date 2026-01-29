class SCR_BaseHeadingDisplay : SCR_InfoDisplayExtended
{
	[Attribute("9", desc: "The amount of azimuth Widgets that will be created. Should always be an uneven number and should be at least 2 more then actually needed.")]
	protected int m_iMaxShownDegrees;

	[Attribute(defvalue: "6400", desc: "Mils in full 360 degrees angle \nSoviet: 6000\nNATO: 6400")]
	protected float m_fMils;

	[Attribute("0.5", UIWidgets.Slider, "Time in seconds after which the Display should fade in", "0 100 0.1")]
	protected float m_fFadeInDelay;

	[Attribute("1")]
	protected bool m_bShowMils;

	[Attribute("1")]
	protected bool m_bShowDegrees;

	[Attribute("m_HeadingTableHolder", "Scroll widget, that holds all the heading notches.")]
	protected string m_sHeadingTableHolderWidgetName;

	[Attribute("DegreeText", "TestWidget below the notch, where the heading is gonna be displayed.")]
	protected string m_sTxtHeadingDegreeNotchWidgetName;

	[Attribute("m_TxtMils", "Text widget where the current heading in mils will be shown.")]
	protected string m_sMilsTextWidgetName;

	[Attribute("m_TxtDegrees", "Text widget where the current heading in degrees will be shown.")]
	protected string m_sDegreeTextWidgetName;

	[Attribute(desc: "SizeLayoutWidgets to which the padding gets applied, to have the correct distance between 2 degrees.")]
	protected ref array<string> m_aPaddingWidgetNames;

	[Attribute(desc: "Widgets defined in 'Padding Widget Names' that should only have half the padding then the others.")]
	protected ref array<string> m_aHalfPaddingWidgetNames;

	protected float m_fDegreeWidgetDefaultSize;
	protected float m_fDegreeWidgetDefaultSizeWidth;
	protected float m_fDegreeWidgetDefaultSizeHeight;
	protected float m_fSliderPosX;
	protected float m_fSliderPosY;
	protected float m_fCurrentFOV;
	// Used to know how many pixels are between 2 degree Widgets
	protected float m_fDegreeWidgetSize;

	// Store the last full degree to know when we moved 1 degree
	protected int m_iLastStoredHeading;
	protected int m_iCurrentHeading;

	protected CameraManager m_CameraManager;
	protected CameraBase m_Camera;

	protected Widget m_wSavedHeadingWidget;
	protected TextWidget m_wMilsTextWidget;
	protected TextWidget m_wDegreesTextWidget;
	protected ScrollLayoutWidget m_wHeadingTableHolderWidget;

	protected TurretControllerComponent m_TurretController;

	protected ref array<TextWidget> m_aHeadingTextWidgets = {};
	protected ref array<SizeLayoutWidget> m_aHeadingPaddingWidgets = {};

	//------------------------------------------------------------------------------------------------
	override void Show(bool show, float speed = UIConstants.FADE_RATE_INSTANT, EAnimationCurve curve = EAnimationCurve.LINEAR)
	{
		speed = UIConstants.FADE_RATE_DEFAULT;

		// If it can be shown, do so after a delay to prevent it from fading in to early
		if (m_bCanShow)
			GetGame().GetCallqueue().CallLater(ShowDelay, m_fFadeInDelay * 1000, false, show, speed);
		else
			super.Show(show, speed);
	}

	//------------------------------------------------------------------------------------------------
	//! Calls the SCR_InfoDisplayExtended.Show method after some delay
	//! \param[in] True if it can be shown, false otherwise
	//! \param[in] Fade speed
	protected void ShowDelay(bool show, float speed = UIConstants.FADE_RATE_INSTANT)
	{
		super.Show(show, speed);
	}

	//------------------------------------------------------------------------------------------------
	//! Update padding and size based on current FOV
	//! \param[in] FOV of current camera
	protected void UpdateFOV(float fov)
	{
		if (fov <= 0)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		float screenW, screenH;
		workspace.GetScreenSize(screenW, screenH);
		screenH = workspace.DPIUnscale(screenH);

		float pixelsPerDegree = screenH / fov;
		float padding = pixelsPerDegree - m_fDegreeWidgetDefaultSize;

		foreach (SizeLayoutWidget w : m_aHeadingPaddingWidgets)
		{
			if (m_aHalfPaddingWidgetNames.Contains(w.GetName()))
			{
				w.SetWidthOverride(padding * 0.5);
				w.SetHeightOverride(padding * 0.5);
			}
			else
			{
				w.SetWidthOverride(padding);
				w.SetHeightOverride(padding);
			}
		}

		CalculateWidgetScreenSizeUnscaled(m_wSavedHeadingWidget);
	}

	//------------------------------------------------------------------------------------------------
	//! Get muzzle rotation global vector
	//! \return azimuth & elevation of turret
	protected vector GetMuzzleDirection()
	{
		BaseWeaponManagerComponent weaponManager = m_TurretController.GetWeaponManager();
		if (!weaponManager)
			return vector.Zero;

		vector transform[4];
		weaponManager.GetCurrentMuzzleTransform(transform);

		return transform[2];
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate the size in screenspace 1 widget takes
	//! \param[in] widget that will be used to get the size from
	protected void CalculateWidgetScreenSizeUnscaled(Widget widget)
	{
		m_wHeadingTableHolderWidget.SetSliderPos(0.5, 0.5);
		m_wHeadingTableHolderWidget.GetSliderPosPixels(m_fSliderPosX, m_fSliderPosY);
		m_fSliderPosX = GetGame().GetWorkspace().DPIUnscale(m_fSliderPosX);
		m_fSliderPosY = GetGame().GetWorkspace().DPIUnscale(m_fSliderPosY);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate the current screensize in DPI unscaled format
	//! \param[in] Widget which will be used to get the size from
	void CalculateScreenSize(Widget degreeWidget)
	{
		if (!degreeWidget)
			return;

		degreeWidget.GetScreenSize(m_fDegreeWidgetDefaultSizeWidth, m_fDegreeWidgetDefaultSizeHeight);
		m_fDegreeWidgetDefaultSizeWidth = GetGame().GetWorkspace().DPIUnscale(m_fDegreeWidgetDefaultSizeWidth);
		m_fDegreeWidgetDefaultSizeHeight = GetGame().GetWorkspace().DPIUnscale(m_fDegreeWidgetDefaultSizeHeight);
	}

	//------------------------------------------------------------------------------------------------
	//! Create all needed Widgets that display the current and next / previous heading
	//! \param[in] headingHolderLayout resource name of the layout that will be used for holding the notches
	//! \param[in] headingNotchLayout resource name of the layout which will be used to create notches
	protected void SetupHeadingWidgets(ResourceName headingHolderLayout = string.Empty, ResourceName headingNotchLayout = string.Empty)
	{
		WorkspaceWidget workspaceWidget = GetGame().GetWorkspace();
		Widget elevationNotchHolder = workspaceWidget.CreateWidgets(headingHolderLayout, m_wHeadingTableHolderWidget);

		if (!elevationNotchHolder)
			return;

		m_aHeadingTextWidgets.Clear();
		m_aHeadingPaddingWidgets.Clear();

		SizeLayoutWidget paddingWidget;

		// Check if there are some Padding widgets on the root to also apply the correct padding to them.
		foreach (string widgetName : m_aPaddingWidgetNames)
		{
			paddingWidget = SizeLayoutWidget.Cast(m_wRoot.FindAnyWidget(widgetName));

			if (paddingWidget)
				m_aHeadingPaddingWidgets.Insert(paddingWidget);
		}

		Widget widget;
		TextWidget degreeTextWidget;

		for (int i = 0; i < m_iMaxShownDegrees; i++)
		{
			widget = workspaceWidget.CreateWidgets(headingNotchLayout, elevationNotchHolder);

			if (!widget)
				continue;

			degreeTextWidget = TextWidget.Cast(widget.FindAnyWidget(m_sTxtHeadingDegreeNotchWidgetName));

			if (degreeTextWidget)
				m_aHeadingTextWidgets.Insert(degreeTextWidget);

			foreach (string widgetName : m_aPaddingWidgetNames)
			{
				paddingWidget = SizeLayoutWidget.Cast(widget.FindAnyWidget(widgetName));

				if (paddingWidget)
					m_aHeadingPaddingWidgets.Insert(paddingWidget);
			}
		}

		m_wSavedHeadingWidget = widget;

		// Needs to run 1 frame later so the size of the widget is the correct one.
		GetGame().GetCallqueue().Call(CalculateScreenSize, widget);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// Only valid if there is main camera
		if (!m_Camera)
			return;

		float fov = m_Camera.GetVerticalFOV();

		// Only update everything if needed
		if (fov != m_fCurrentFOV)
		{
			UpdateFOV(fov);
			m_fCurrentFOV = fov;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_TurretController = TurretControllerComponent.Cast(owner.FindComponent(TurretControllerComponent));

		m_wMilsTextWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sMilsTextWidgetName));
		m_wDegreesTextWidget = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sDegreeTextWidgetName));

		if (!m_bShowMils && m_wMilsTextWidget)
			m_wMilsTextWidget.SetVisible(false);

		if (!m_bShowDegrees && m_wDegreesTextWidget)
			m_wDegreesTextWidget.SetVisible(false);

		m_wHeadingTableHolderWidget = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget(m_sHeadingTableHolderWidgetName));

		m_CameraManager = GetGame().GetCameraManager();

		if (m_CameraManager)
			m_Camera = m_CameraManager.CurrentCamera();
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}

}
