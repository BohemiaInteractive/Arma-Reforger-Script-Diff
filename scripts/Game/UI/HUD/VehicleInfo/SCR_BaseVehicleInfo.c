//#define DEBUG_VEHICLE_UI

//------------------------------------------------------------------------------------------------
//! UI indicator state, controlling colors and opacity
enum EVehicleInfoState
{
	NOT_INITIALIZED,
	DISABLED,
	ENABLED,
	WARNING,
	ERROR
};

//------------------------------------------------------------------------------------------------
//! Predefined colors that can be assigned in the InfoDisplay configuration
enum EVehicleInfoColor
{
	WHITE,
	BLUE,
	GREEN,
	ORANGE,
	RED
};

//------------------------------------------------------------------------------------------------
//! Base class for all vehicle UI state and damage indicators
[BaseContainerProps(configRoot: true)]
class SCR_BaseVehicleInfo : SCR_InfoDisplayExtended
{
	protected ResourceName m_Imageset = "{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset";
	protected ResourceName m_ImagesetGlow = "{00FE3DBDFD15227B}UI/Textures/Icons/icons_wrapperUI-glow.imageset";

	protected ImageWidget m_wIcon;
	protected ImageWidget m_wGlow;
	protected OverlayWidget m_wSizeOverlay;

	[Attribute("", UIWidgets.EditBox, "Indicator icon to be displayed.")]
	protected string m_sIcon;
	
	[Attribute(SCR_Enum.GetDefault(EVehicleInfoColor.WHITE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleInfoColor))]
	EVehicleInfoColor m_eColor;		

	[Attribute("0.8", UIWidgets.Slider, "Indicator scale", "0.25 2 0.05")]
	protected float m_fWidgetScale;

	[Attribute("1", UIWidgets.CheckBox, "Show the greyed-out indicator, if it is inactive, otherwise it will be hidden.")]
	protected bool m_bShowGhost;

	protected EVehicleInfoState m_eState = EVehicleInfoState.ERROR;
	protected bool m_bIsBlinking;
	protected int m_iBlinkingOffset;

    protected Color m_aColors[5];
	protected Color m_aColorsGlow[5];
	
	protected const int ICON_SIZE = 64;
  
	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	protected EVehicleInfoState GetState()
	{
		return m_eState;
	}

	//------------------------------------------------------------------------------------------------
	//! Can be overridden to get state of actual system or linked to an event
	protected bool IsBlinking()
	{
		return m_bIsBlinking;
	}

	//------------------------------------------------------------------------------------------------
	protected void Scale(ImageWidget widget, float scale)
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
	protected void Scale(TextWidget widget, float scale)
	{
		if (!widget)
			return;

		float sizeY = FrameSlot.GetSizeY(widget);

		FrameSlot.SetSizeY(widget, sizeY * scale);
	}

	//------------------------------------------------------------------------------------------------
	protected void Scale(OverlayWidget widget, float scale)
	{
		if (!widget)
			return;

		FrameSlot.SetSizeX(widget, scale * ICON_SIZE);
		FrameSlot.SetSizeY(widget, scale * ICON_SIZE);		
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void SetIcon(string icon)
	{
		if (!m_wIcon || !m_wGlow)
			return;

		m_wIcon.LoadImageFromSet(0, m_Imageset, icon);
		m_wGlow.LoadImageFromSet(0, m_ImagesetGlow, icon);

		Scale(m_wSizeOverlay, m_fWidgetScale);
	}	

	//------------------------------------------------------------------------------------------------
	protected void SetColor(EVehicleInfoState state, EVehicleInfoColor color)
	{
		if (!m_wIcon || !m_wGlow)
			return;

		switch(state)
		{
			case EVehicleInfoState.DISABLED:
				m_wIcon.SetColor(Color.FromInt(GUIColors.DISABLED.PackToInt()));
				m_wGlow.SetColor(Color.FromInt(GUIColors.DISABLED_GLOW.PackToInt()));
				break;
			
			case EVehicleInfoState.ENABLED:
				m_wIcon.SetColor(Color.FromInt(m_aColors[color].PackToInt()));
				m_wGlow.SetColor(Color.FromInt(m_aColorsGlow[color].PackToInt()));
				break;
			
			case EVehicleInfoState.WARNING:
				m_wIcon.SetColor(Color.FromInt(GUIColors.ORANGE.PackToInt()));
				m_wGlow.SetColor(Color.FromInt(GUIColors.ORANGE_DARK.PackToInt()));
				break;			
			
			case EVehicleInfoState.ERROR:
				m_wIcon.SetColor(Color.FromInt(GUIColors.RED.PackToInt()));
				m_wGlow.SetColor(Color.FromInt(GUIColors.RED_DARK.PackToInt()));
				break;
		}		
	}	

	//------------------------------------------------------------------------------------------------
	protected bool UpdateRequired(EVehicleInfoState state)
	{
		return m_eState != state;
	}		
			
	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return;

		EVehicleInfoState state = GetState();
		
		bool isBlinking = IsBlinking();
		int time = GetGame().GetWorld().GetWorldTime();
		
		if (isBlinking)
		{
			if (Math.Mod(time - m_iBlinkingOffset, 1000) < 500)
				state = Math.Max(state - 1, EVehicleInfoState.DISABLED);
		}
		else
		{
			m_iBlinkingOffset = Math.Mod(time, 1000);
		}

		if (UpdateRequired(state))
		{
			#ifdef DEBUG_VEHICLE_UI
			PrintFormat("%1 Update -> state: %2 | icon: %3", this, state, m_sIcon);
			#endif

			m_eState = state;
			
			SetIcon(m_sIcon);
			SetColor(state, m_eColor);

			bool show = isBlinking || m_bShowGhost || state != EVehicleInfoState.DISABLED;
			
			if (m_bShown != show)
				Show(show);		
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// Terminate if widget already exists
		if (m_wRoot)
			return false;

		// Fallback to avoid the need to fill-in always the same layout filename
		if (m_LayoutPath == "")
			m_LayoutPath = "{D2E54F91C85CAB6C}UI/layouts/HUD/VehicleInfo/VehicleInfoIcon.layout";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Create the UI
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_wIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon"));
		m_wGlow = ImageWidget.Cast(m_wRoot.FindAnyWidget("Glow"));
		m_wSizeOverlay = OverlayWidget.Cast(m_wRoot.FindAnyWidget("SizeOverlay"));

		m_eState = EVehicleInfoState.NOT_INITIALIZED;
		
		DisplayUpdate(owner, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Destroy the UI
	override void DisplayStopDraw(IEntity owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Init the UI, runs 1x at the start of the game
	override void DisplayInit(IEntity owner)
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		
		m_aColors[EVehicleInfoColor.WHITE] = GUIColors.ENABLED;
		m_aColors[EVehicleInfoColor.BLUE] = GUIColors.BLUE_BRIGHT2;
		m_aColors[EVehicleInfoColor.GREEN] = GUIColors.GREEN_BRIGHT2;
		m_aColors[EVehicleInfoColor.ORANGE] = GUIColors.ORANGE_BRIGHT2;
		m_aColors[EVehicleInfoColor.RED] = GUIColors.RED_BRIGHT2;
		
		m_aColorsGlow[EVehicleInfoColor.WHITE] = GUIColors.ENABLED_GLOW;
		m_aColorsGlow[EVehicleInfoColor.BLUE] = GUIColors.BLUE;
		m_aColorsGlow[EVehicleInfoColor.GREEN] = GUIColors.GREEN;
		m_aColorsGlow[EVehicleInfoColor.ORANGE] = GUIColors.ORANGE;
		m_aColorsGlow[EVehicleInfoColor.RED] = GUIColors.RED;		
	}
};
