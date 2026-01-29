class SCR_BaseAnalogGaugeData
{
	// Class defined gauge-specific settings
	float m_fRingLabelOffset = 0.8;
	int m_fRingLabelFontSize = 20;
	ResourceName m_AnalogGauge = "{54F7755D48B394FD}UI/Textures/VehicleInfo/AnalogGaugeImageset.imageset";
	string m_sRingMarkSection = "GaugeMarkSection";
	string m_sRingMarkSubsection = "GaugeMarkSubsection";
	string m_sDefaultLayout = "{00F28F3165DB7C83}UI/layouts/HUD/VehicleInfo/VehicleGauge_Default.layout";
	
	// Custom attributes	
	string m_sOverlay = "";
	string m_sCustomNeedle = "";	
};

class SCR_BaseAnalogGauge : SCR_InfoDisplayExtended
{	
	// Attributes: widget setup
	[Attribute("270", UIWidgets.Slider, "Gauge angular range (in degrees)", "30 360 5")]
	protected float m_fWidgetRange;
	
	[Attribute("0", UIWidgets.Slider, "Gauge angular rotation (in degrees)", "0 360 5")]
	protected float m_fWidgetRotation;
	
	[Attribute("0.6", UIWidgets.Slider, "Gauge global scale", "0.25 2 0.05")]
	protected float m_fWidgetScale;

	
	// Attributes: info texts
	[Attribute("", UIWidgets.EditBox, "Custom text (e.g. units or gauge name). Hidden if empty.")]
	protected string  m_sInfoText;
	
	[Attribute("0.5", UIWidgets.Slider, "Units text anchor X-coordinate %-base of widget width.", "0 1 0.05")]
	protected float m_fInfoPosX;	
	
	[Attribute("0.15", UIWidgets.Slider, "Units text anchor Y-coordinate %-base of widget height.", "0 1 0.05")]
	protected float m_fInfoPosY;	

	[Attribute("", UIWidgets.EditBox, "Custom text (e.g. units or gauge name). Hidden if empty.")]
	protected string  m_sInfo2Text;
	
	[Attribute("0.5", UIWidgets.Slider, "Units text anchor X-coordinate %-base of widget width.", "0 1 0.05")]
	protected float m_fInfo2PosX;	
	
	[Attribute("0.85", UIWidgets.Slider, "Units text anchor Y-coordinate %-base of widget height.", "0 1 0.05")]
	protected float m_fInfo2PosY;		
	
	// Attributes: labels
	[Attribute("1", UIWidgets.SpinBox, "Label value multiplier.")]
	protected float m_fLabelValueMultiplier;	

	[Attribute("0", UIWidgets.SpinBox, "Label value precision.","-6 6 1")]
	protected int m_iLabelValuePrecision;
	
	[Attribute("1", UIWidgets.CheckBox, "Display absolute values on labels.")]
	protected bool m_bAbsLabelValues;		

	[Attribute("1", UIWidgets.CheckBox, "Display label for ZERO value.")]
	protected bool m_bShowLabel0;	

	[Attribute("1", UIWidgets.CheckBox, "Display label for MAX value.")]
	protected bool m_bShowLabelMax;	

	[Attribute("1", UIWidgets.CheckBox, "Display label for MIN value; both MIN && ZERO needs to be checked.")]
	protected bool m_bShowLabelMin;
		
	
	protected ref SCR_BaseAnalogGaugeData m_pGaugeData;
	
	protected TextWidget m_wInfo;
	protected TextWidget m_wInfo2;
	protected Widget m_wInfoAnchor;
	protected Widget m_wInfo2Anchor;
	
	protected ImageWidget m_wRing;
	protected ImageWidget m_wBackground;
	protected ImageWidget m_wNeedle;
	protected ImageWidget m_wNeedleShadow;
	protected ImageWidget m_wOverlay;
	
	protected float m_fGaugeSizeX;
	protected float m_fGaugeSizeY;
	
	protected Widget m_wRingMarks;
	protected Widget m_wRingLabels;
	
	protected float m_fZeroValueRotation;		//calculated from m_fWidgetRotation & m_fWidgetRange
	
	// Interface for getting value from signal(s) or component
	//------------------------------------------------------------------------------------------------
	protected float GetValue()
	{
		return 0;
	}

	// Interface for getting percentage representation of the value - doesn't need to be always linear
	//------------------------------------------------------------------------------------------------
	protected float GetValuePerc(float value)
	{
		return 0;
	}

	// Interface for getting gauge call specific data
	//------------------------------------------------------------------------------------------------
	protected ref SCR_BaseAnalogGaugeData GetGaugeData()
	{
		return null;
	}	

	// Interface for initialization and preprocessing of attribute values
	//------------------------------------------------------------------------------------------------
	protected void InitGauge(IEntity owner, out bool bSuccess)
	{
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void CreateGauge(IEntity owner, out bool bSuccess)
	{
		// Gauge (ring, marks, labels)
		bSuccess = CreateGaugeRing(owner);

		if (!bSuccess)
			return;
						
		// Gauge needle
		m_wNeedle = ImageWidget.Cast(m_wRoot.FindAnyWidget("Needle"));
		m_wNeedleShadow = ImageWidget.Cast(m_wRoot.FindAnyWidget("NeedleShadow"));
		bSuccess = CreateGaugeNeedle(owner, m_wNeedle, m_wNeedleShadow, m_pGaugeData.m_sCustomNeedle);

		if (!bSuccess)
			return;				

		// Gauge overlay (e.g. icon of "fuel stand")
		m_wOverlay = ImageWidget.Cast(m_wRoot.FindAnyWidget("Overlay"));
		bSuccess = CreateGaugeOverlay(owner, m_wOverlay, m_pGaugeData.m_sOverlay);

		if (!bSuccess)
			return;			
				
		// Gauge info text
		m_wInfo = TextWidget.Cast(m_wRoot.FindAnyWidget("InfoText"));
		m_wInfoAnchor = m_wRoot.FindAnyWidget("InfoAnchor");
		CreateGaugeInfoText(owner, m_wInfo, m_sInfoText, m_wInfoAnchor, m_fInfoPosX, m_fInfoPosY);

		// Gauge info text2
		m_wInfo2 = TextWidget.Cast(m_wRoot.FindAnyWidget("Info2Text"));
		m_wInfo2Anchor = m_wRoot.FindAnyWidget("Info2Anchor");
		CreateGaugeInfoText(owner, m_wInfo2, m_sInfo2Text, m_wInfo2Anchor, m_fInfo2PosX, m_fInfo2PosY);
	}	
	
	// Creates gauge ring, markings and labels based on dozen of data feeded from gauge attributes	
	//------------------------------------------------------------------------------------------------	
	protected bool CreateGaugeRing(IEntity owner)
	{
		m_fZeroValueRotation = m_fWidgetRotation + (360 - m_fWidgetRange) / 2;
		
		m_wRing = ImageWidget.Cast(m_wRoot.FindAnyWidget("Ring"));
		m_wBackground = ImageWidget.Cast(m_wRoot.FindAnyWidget("Background"));
		m_wRingMarks = m_wRoot.FindAnyWidget("RingMarks");
		m_wRingLabels = m_wRoot.FindAnyWidget("RingLabels");	
		
		if (!m_wRing)
			return false;
		
		// Setup gauge ring		
		m_wRing.SetMaskProgress(m_fWidgetRange/360);
		m_wRing.SetRotation(m_fZeroValueRotation);	
		Scale(m_wRing, m_fWidgetScale);
		
		// Setup gauge background
		Scale(m_wBackground, m_fWidgetScale);

		// Get scaled gauge size from the Ring, used for dynamic positionning of other elements		
		vector size = m_wRing.GetSize();		
		m_fGaugeSizeX = size[0];
		m_fGaugeSizeY = size[1];		

		// Terminate if there is no ring marks parent widget
		if (!m_wRingMarks)
			return false;
		
		return true;	
	}
	
	//------------------------------------------------------------------------------------------------	
	protected bool CreateGaugeNeedle(IEntity owner, ImageWidget wNeedle, ImageWidget wNeedleShadow, string sCustomNeedle = "", bool bVisible = true)
	{
		if (!wNeedle || !wNeedleShadow)
			return false;

		if (sCustomNeedle)
		{
			wNeedle.LoadImageFromSet(0, m_pGaugeData.m_AnalogGauge, sCustomNeedle);
			wNeedleShadow.LoadImageFromSet(0, m_pGaugeData.m_AnalogGauge, sCustomNeedle);
		}			
		Scale(wNeedle, m_fWidgetScale);
		Scale(wNeedleShadow, m_fWidgetScale);		
	
		wNeedle.SetVisible(bVisible);
		wNeedleShadow.SetVisible(bVisible);
		
		return true;	
	}	
	
	//------------------------------------------------------------------------------------------------	
	protected bool CreateGaugeOverlay(IEntity owner, ImageWidget wOverlay, string sOverlayTexture, bool bVisible = true)
	{
		if (!m_wOverlay)
			return false;
		
		if (sOverlayTexture == "")
		{
			wOverlay.SetVisible(false);
			return true;
		}

		wOverlay.LoadImageFromSet(0, m_pGaugeData.m_AnalogGauge, sOverlayTexture);
		wOverlay.SetVisible(bVisible);
		Scale(wOverlay, m_fWidgetScale);		
	
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CreateGaugeInfoText(IEntity owner, TextWidget wInfo, string sText, Widget wAnchor, float fPosX, float fPosY, bool bVisible = true)
	{
		if (!wInfo || !wAnchor)
			return false;
		
		if (sText == "")
		{
			wInfo.SetVisible(false);
			return true;
		}

		wInfo.SetVisible(bVisible);
		wInfo.SetText(sText);
		Scale(wInfo, m_fWidgetScale);
		MovePerc(wAnchor, fPosX, fPosY);		
	
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateGauge(IEntity owner, float timeSlice, float value)
	{
		// Get unclamped percentage value of the gauge range
		float fValuePerc = GetValuePerc(value);
		
		UpdateGaugeNeedle(owner, timeSlice, m_wNeedle, m_wNeedleShadow, fValuePerc);	
	}	
	
	// Rotates gauge needle based on % of the value range; supports Clamped / Free360 needle movement behavior
	//------------------------------------------------------------------------------------------------
	protected void UpdateGaugeNeedle(IEntity owner, float timeSlice, ImageWidget wNeedle, ImageWidget wNeedleShadow, float fValuePerc, float fScaleValue = 1.00, bool bClamp = true)
	{
		if (!m_wNeedle)
			return;
		
		// Scale the value
		fValuePerc = fValuePerc * fScaleValue;
		
		// Ajust the %value for Clamped / Free360 movement behavior
		if (bClamp)
		{
			fValuePerc = Math.Clamp(fValuePerc, 0, 1);
		}
		else
		{
			fValuePerc = fValuePerc - Math.Floor(fValuePerc);
			
			if (fValuePerc < 0)
				fValuePerc = 1 + fValuePerc;
		}
		
		// Rotate the gauge needle
		float fAngle = m_fZeroValueRotation + (fValuePerc * m_fWidgetRange);	

		wNeedle.SetRotation(fAngle);
		
		if (wNeedleShadow)
			wNeedleShadow.SetRotation(fAngle);
	}		
		
	//------------------------------------------------------------------------------------------------
	protected void DestroyGauge()
	{
		m_pGaugeData = null;		
				
		if (!m_wRoot)
			return;		

		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		
		m_wRoot = null;		
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return;		
		
		// Get actual value
		float value = GetValue();

		UpdateGauge(owner, timeSlice, value);
	}	

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{				
		// Terminate if widget already exists
		if (m_wRoot)
			return false;

		// Get gauge setup data
		m_pGaugeData = GetGaugeData();
		
		if (!m_pGaugeData)
			return false;		
		
		// Force default layout, if layout is not set
		if (m_LayoutPath == "")
			m_LayoutPath = m_pGaugeData.m_sDefaultLayout;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{				
		if (!m_wRoot)
		{
			m_pGaugeData = null;
			return;
		}
		
		// Init gauge settings & attributes
		bool bSuccess = true;
		InitGauge(owner, bSuccess);

		if (!bSuccess)
			DestroyGauge();		
			
		// Create gauge with all its elements; some are optional and can fail
		CreateGauge(owner, bSuccess);

		if (!bSuccess)
			DestroyGauge();
	}	
		
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		DestroyGauge();
	}
		
	//------------------------------------------------------------------------------------------------
	override void DisplayInit(IEntity owner)
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------
	private void MovePerc(Widget widget, float x, float y)
	{
		if (!widget || !m_wRing)
			return;
		
		// Re-adjust x & y as 0,0 should mean the middle
		x -= 0.5;
		y -= 0.5;
		
		OverlaySlot.SetPadding(widget, x * m_fGaugeSizeX, y * m_fGaugeSizeY, 0, 0);
	}		
		
	//------------------------------------------------------------------------------------------------
	private void Scale(ImageWidget widget, float scale)
	{
		if (!widget)
			return;
		
		int imageWidth = 0;
		int imageHeight = 0;
		int image = widget.GetImage();
		
		widget.GetImageSize(image, imageWidth, imageHeight);
		widget.SetSize((float)imageWidth * scale, (float)imageHeight * scale);		
	}	

	//------------------------------------------------------------------------------------------------	
	private void Scale(TextWidget widget, float scale)
	{
		if (!widget)
			return;

		float sizeY = FrameSlot.GetSizeY(widget);
		FrameSlot.SetSizeY(widget, sizeY * scale);
	}	

	//------------------------------------------------------------------------------------------------
	protected void CreateRingLabel(WorkspaceWidget workspace, float angle, float value, bool absValues = true, float labelValueMultiplier = 1, int labelValuePrecision = 0)
	{
		value = value * labelValueMultiplier;		//doesn't always need to start from 0, even if it's most common
		
		if (labelValuePrecision == 0)
		{
			value = Math.Round(value);
		}
		else
		{
			float decimals = labelValuePrecision * 10;
		
			if (decimals < 0)
				decimals = -1 / decimals;
			
			value = Math.Round(value * decimals);
			value = value / decimals;
		}
		
		if (value == 0 && !m_bShowLabel0)
			return;

		if (absValues)
			value = Math.AbsFloat(value);
				
		float rads = angle / Math.RAD2DEG;
		
		float xcoef = - Math.Sin(rads);
		float ycoef = Math.Cos(rads);

		float x = 0.5 * m_fGaugeSizeX * xcoef * m_pGaugeData.m_fRingLabelOffset;
		float y = 0.5 * m_fGaugeSizeY * ycoef * m_pGaugeData.m_fRingLabelOffset;			
		
		Widget w = workspace.CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.INHERIT_CLIPPING | WidgetFlags.VISIBLE | WidgetFlags.ADDITIVE | WidgetFlags.STRETCH,  null, 0, m_wRingLabels);
		TextWidget wText = TextWidget.Cast(w);		
		
		wText.SetText(value.ToString());
		wText.SetFont("{EABA4FE9D014CCEF}UI/Fonts/RobotoCondensed/RobotoCondensed_Bold.fnt");
		wText.SetExactFontSize(m_pGaugeData.m_fRingLabelFontSize * m_fWidgetScale);

		float alignx = xcoef / 2 + 0.5;
		float aligny = ycoef / 2 + 0.5;
				
		FrameSlot.SetAlignment(wText, alignx, aligny);
		FrameSlot.SetAnchorMin(wText, 0.5, 0.5);
		FrameSlot.SetAnchorMax(wText, 0.5, 0.5);
		FrameSlot.SetSizeToContent(wText, true);
		FrameSlot.SetPos(wText, x, y);		
	}	

	//------------------------------------------------------------------------------------------------		
	protected void CreateRingMark(WorkspaceWidget workspace, float angle, string texture)
	{
		Widget w = workspace.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.INHERIT_CLIPPING | WidgetFlags.VISIBLE | WidgetFlags.ADDITIVE | WidgetFlags.STRETCH,  null, 0, m_wRingMarks);
		ImageWidget wImage = ImageWidget.Cast(w);

		wImage.LoadImageFromSet(0, m_pGaugeData.m_AnalogGauge, texture);
		FrameSlot.SetAlignment(wImage, 0.5, 0);
		FrameSlot.SetAnchorMin(wImage, 0.5, 0.5);
		FrameSlot.SetAnchorMax(wImage, 0.5, 0.5);
		FrameSlot.SetSizeToContent(wImage, true);
		FrameSlot.SetPos(wImage, 0, 0);

		Scale(wImage, m_fWidgetScale);		
		
		wImage.SetPivot(0.5, 0);		
		wImage.SetRotation(angle);
	}	
};

