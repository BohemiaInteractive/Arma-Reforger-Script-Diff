//------------------------------------------------------------------------------------------------
//! Contains various global constants
//------------------------------------------------------------------------------------------------

const float METERS_PER_SEC_TO_KILOMETERS_PER_HOUR = 3.6;
const float KILOMETERS_PER_HOUR_TO_METERS_PER_SEC = 0.277778;	// 1 / 3.6

const float METERS_TO_KNOTS = 1.943844;
const float KNOTS_TO_METERS = 0.514444;

const float COLOR_255_TO_1 = 0.003921568;						// 1 / 255
const float COLOR_1_TO_255 = 255;

const int TRACE_LAYER_CAMERA = EPhysicsLayerDefs.Camera;

class UIColors
{
	static const ref Color DARK_SAGE        			= Color.FromSRGBA(86, 92, 84, 255);
	static const ref Color DARK_GREY					= Color.FromSRGBA(85, 85, 85, 255);
	static const ref Color TRANSPARENT      			= Color.FromSRGBA(0, 0, 0, 0);

	// Refined UI colors
	static const ref Color CONTRAST_DISABLED			= Color.FromSRGBA(0, 0, 0, 38);
	static const ref Color CONTRAST_DEFAULT   			= Color.FromSRGBA(226, 167, 79, 76);
	static const ref Color CONTRAST_HOVERED				= Color.FromSRGBA(239, 199, 139, 102);
	static const ref Color CONTRAST_CLICKED  			= Color.FromSRGBA(226, 167, 79, 255);
	static const ref Color CONTRAST_CLICKED_HOVERED		= Color.FromSRGBA(226, 167, 79, 255);
	
	static const ref Color WHITE_DISABLED				= Color.FromSRGBA(255, 255, 255, 38);
	static const ref Color WHITE_DEFAULT				= Color.FromSRGBA(255, 255, 255, 25);
	static const ref Color WHITE_HOVERED				= Color.FromSRGBA(255, 255, 255, 102);
	
	static const ref Color BACKGROUND_DISABLED			= Color.FromSRGBA(0, 0, 0, 38);
	static const ref Color BACKGROUND_DEFAULT			= Color.FromSRGBA(0, 0, 0, 102);
	static const ref Color BACKGROUND_HOVERED			= Color.FromSRGBA(0, 0, 0, 153);

	static const ref Color INFO							= Color.FromSRGBA(0, 128, 255, 255);
	
	// From Manual
	static const ref Color HIGHLIGHTED					= Color.FromSRGBA(255, 203, 123, 255);	//#FFCB7B
	static const ref Color NEUTRAL_INFORMATION			= Color.FromSRGBA(255, 255, 255, 255);	//#FFFFFF
	static const ref Color NEUTRAL_ACTIVE_STANDBY		= Color.FromSRGBA(189, 189, 188, 255); 	//#BDBDBC
	static const ref Color IDLE_ACTIVE					= Color.FromSRGBA(255, 255, 255, 179);	//#FFFFFF at 70% Alpha
	static const ref Color IDLE_DISABLED				= Color.FromSRGBA(77, 77, 77, 255);		//#4D4D4D
	static const ref Color IDLE_DISABLED_TRANSPARENT	= Color.FromSRGBA(77, 77, 77, 153);		//60% #4D4D4D
	static const ref Color WARNING						= Color.FromSRGBA(249, 67, 67, 255);	//#f94343 red
	static const ref Color WARNING_DISABLED				= Color.FromSRGBA(146, 72, 72, 255);	//#924848 dark red
	static const ref Color WARNING_DISABLED_FOCUSED		= Color.FromSRGBA(173, 71, 71, 255);  	//#ad4747
	static const ref Color SLIGHT_WARNING				= Color.FromSRGBA(234, 203, 131, 255);	//#EACB83 light yellow
	static const ref Color CONFIRM						= Color.FromSRGBA(67, 194, 93, 255);	//#43C25D green, Manual calls it POSITIVE
	static const ref Color CONTRAST_COLOR	 			= Color.FromSRGBA(226, 167, 79, 255); 	//#e2a74f REFORGER ORANGE
	static const ref Color ONLINE	 					= Color.FromSRGBA(0, 128, 255, 255); 	//#0080FF blue
	static const ref Color EDIT_WIDGET_BACKGROUND 		= Color.FromSRGBA(42, 42, 42, 255);		//#2a2a2a
	static const ref Color SUB_HEADER 					= Color.FromSRGBA(255, 220, 105, 255);	//#ffdc69 yellow
	
	//~ editor
	static const ref Color EDITOR_ICON_COLOR_NEUTRAL	= Color.FromSRGBA(255, 255, 255, 255);	///< Colors for Editor Icons when an entity does not have an assigned faction and is not destroyed
	static const ref Color EDITOR_ICON_COLOR_DESTROYED	= Color(0.25, 0.25, 0.25, 1); 			///< Colors for Editor Icons when an entity is dead or destroyed
	static const ref Color EDITOR_MODE_GM_MODE 			= Color.FromRGBA(235, 46, 9, 255);		// Color for GM mode and LOGISTICS
	static const ref Color EDITOR_MODE_ADMIN_MODE 		= Color.FromRGBA(0, 55, 255, 255);		// Color for ADMIN mode and COMBAT SUPPORT
	static const ref Color EDITOR_MODE_ARMAVISION		= Color.FromRGBA(1, 50, 12, 255);		// Color for ARMAVISION and OPERATIONS
	
	// Nearby Interactions
	static const ref Color INTERACT_GENERIC_PRIMARY        			= Color.FromSRGBA(120, 120, 120, 180);
	static const ref Color INTERACT_GENERIC_SECONDARY        			= Color.FromSRGBA(120, 120, 120, 70);

	//------------------------------------------------------------------------------------------------
	//! Get the provided colour in a format usable as default of Attributes
	//! \param color if null, considered white
	//! \return the provided colour in "R G B A" string format (values in range 0..1)
	static string GetColorAttribute(Color color)
	{
		if (color)
			return string.Format("%1 %2 %3 %4", color.R(), color.G(), color.B(), color.A());

		return "1 1 1 1";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the provided color in a format usable for Rich Text formatting
	//! \param color if null, considered white
	//! \return the provided colour in "R, G, B, A" string format (note the commas, values in range 0..255)
	static string FormatColor(Color color)
	{
		if (!color)
			return "255, 255, 255, 255";
		
		Color returnColor = new Color(color.R(), color.G(), color.B(), color.A());
		
		//--- Convert to sRGBA format for rich text
		returnColor.LinearToSRGB();

		//--- Convert to ints, no fractions allowed in rich text
		int colorR = returnColor.R() * 255;
		int colorG = returnColor.G() * 255;
		int colorB = returnColor.B() * 255;
		int colorA = returnColor.A() * 255;
		
		return string.Format("%1, %2, %3, %4", colorR, colorG, colorB, colorA);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Use this to fetch colors, for safety
	static Color CopyColor(Color uiConstColor)
	{
		return Color.FromInt(uiConstColor.PackToInt());
	}
}

class GUIColors
{
	static const ref Color DISABLED_GRAY  			= Color.FromSRGBA(128, 128, 128, 255); //Dark Gray
	static const ref Color DISABLED 				= Color.FromSRGBA(200, 200, 200, 100);	//WHITE with 30% alpha converted to GREY with 100% alpha
	static const ref Color DISABLED_GLOW 			= Color.FromSRGBA(0, 0, 0, 100);

	static const ref Color ENABLED 					= Color.FromSRGBA(255, 255, 255, 255);	//WHITE
	static const ref Color ENABLED_GLOW 			= Color.FromSRGBA(162, 162, 162, 255);	//GREY
	
	static const ref Color DEFAULT 					= Color.FromSRGBA(255, 255, 255, 255);	//WHITE
	static const ref Color DEFAULT_GLOW 			= Color.FromSRGBA(0, 0, 0, 255);		//BLACK				
	
	static const ref Color WHITE_GLOW				= Color.FromSRGBA(225, 225, 225, 178); 	//WHITE with 70% alpha															  
	//---

	static const ref Color ORANGE 					= Color.FromSRGBA(226, 167, 80, 255);	//ORANGE, standard UI orange, warnings
	static const ref Color ORANGE_BRIGHT 			= Color.FromSRGBA(255, 207, 136, 255);	//ORANGE (bright)
	static const ref Color ORANGE_BRIGHT2 			= Color.FromSRGBA(255, 233, 200, 255);	//ORANGE (bright++)
	static const ref Color ORANGE_DARK 				= Color.FromSRGBA(162, 97, 0, 255);		//DARK ORANGE

	static const ref Color RED 						= Color.FromSRGBA(236, 80, 80, 255);	//RED, error states
	static const ref Color RED_BRIGHT 				= Color.FromSRGBA(255, 134, 134, 255);	//RED (bright)
	static const ref Color RED_BRIGHT2 				= Color.FromSRGBA(255, 150, 150, 255);	//RED (bright++)
	static const ref Color RED_DARK 				= Color.FromSRGBA(162, 0, 0, 255);		//DARK RED

	static const ref Color BLUE 					= Color.FromSRGBA(41, 127, 240, 255);	//BLUE
	static const ref Color BLUE_BRIGHT 				= Color.FromSRGBA(122, 175, 255, 255);	//BLUE (bright)
	static const ref Color BLUE_BRIGHT2 			= Color.FromSRGBA(184, 212, 255, 255);	//BLUE (bright++)
	static const ref Color BLUE_DARK 				= Color.FromSRGBA(27, 92, 189, 255);	//DARK BLUE	

	static const ref Color GREEN 					= Color.FromSRGBA(37, 209, 29, 255);	//GREEN
	static const ref Color GREEN_BRIGHT 			= Color.FromSRGBA(157, 250, 153, 255);	//GREEN (bright)
	static const ref Color GREEN_BRIGHT2			= Color.FromSRGBA(216, 255, 214, 255);	//GREEN (bright++)
	static const ref Color GREEN_DARK 				= Color.FromSRGBA(28, 157, 22, 255);	//DARK GREEN
	
	static const ref Color LIGHT_GRAY				= Color.FromSRGBA(189, 189, 188, 255);	//LIGHT GRAY
	static const ref Color DARK_GRAY  				= Color.FromSRGBA(128, 128, 128, 255); //Dark Gray
	static const ref Color DARKER_GRAY  				= Color.FromSRGBA(98, 98, 98, 200); //Darker Gray
}

class UIConstants
{
	// Animation
	static const float FADE_RATE_INSTANT =		0;
	static const float FADE_RATE_SUPER_FAST = 	20;
	static const float FADE_RATE_FAST = 		10;
	static const float FADE_RATE_DEFAULT = 		5; 		// Used for near instant actions
	static const float FADE_RATE_SLOW = 		1; 		// Used for fading out elements that should be visible for some time
	static const float FADE_RATE_SUPER_SLOW = 	0.2; 	// Very slow fade out
	
	// Spinners 
	static const float PROCESSING_SPINNER_ANIMATION_SPEED = 0.75;
	
	// Common labels
	static const LocalizedString FAVORITE_LABEL_ADD = 		"#AR-Workshop_ButtonAddToFavourites";
	static const LocalizedString FAVORITE_LABEL_REMOVE = 	"#AR-Workshop_ButtonRemoveFavourites";
	static const LocalizedString BOHEMIA_INTERACTIVE_LOC =	"#AR-Author_BI";
	static const string BOHEMIA_INTERACTIVE = 				"Bohemia Interactive";
	
	static const LocalizedString VALUE_UNIT_PERCENTAGE = 	"#AR-ValueUnit_Percentage";
	static const LocalizedString VALUE_UNIT_SHORT_PLUS =	"#AR-ValueUnit_Short_Plus";
	static const LocalizedString VALUE_UNIT_MILS =			"#AR-ValueUnit_Mils";
	static const LocalizedString VALUE_UNIT_DEGREES =		"#AR-ValueUnit_Degrees";
	static const LocalizedString VALUE_UNIT_METERS =		"#AR-ValueUnit_Short_Meters";
	static const LocalizedString VALUE_UNIT_SECONDS =		"#AR-ValueUnit_Short_Seconds";
	static const LocalizedString VALUE_MUTLIPLIER_SHORT =	"#AR-ValueUnit_Short_Times";
	
	static const LocalizedString VALUE_OUT_OF =				"#AR-SupportStation_ActionFormat_ItemAmount";
	static const LocalizedString VALUE_OUT_OF_SPACED =		"#AR-Filters_EntriesFound_Condensed";
	
	static const LocalizedString TIME_DISPLAY_DAYS_HOURS_MINUTES_SECONDS =	"#AR-TimeDisplay_Days_Hours_Minutes_Seconds";
	static const LocalizedString TIME_DISPLAY_HOURS_MINUTES_SECONDS = 		"#AR-TimeDisplay_Hours_Minutes_Seconds";
	static const LocalizedString TIME_DISPLAY_MINUTES_SECONDS = 			"#AR-TimeDisplay_Minutes_Seconds";
	static const LocalizedString TIME_DISPLAY_SECONDS =						"#AR-TimeDisplay_Seconds";
	
	// Common icons
	static const ResourceName ICONS_IMAGE_SET = 		"{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset";
	static const ResourceName ICONS_GLOW_IMAGE_SET = 	"{00FE3DBDFD15227B}UI/Textures/Icons/icons_wrapperUI-glow.imageset";

	//! Rich text format used to add an icon to the rich text widget
	static const string NAME_WITH_PLATFORM_ICON_FORMAT = "<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>%5";
	static const string RICH_TEXT_LINE_BREAK = "<br/>";
	static const string RICH_TEXT_LIST_POINT = "<ucs codepoints=\"u2022\"/>";
	
	// Platform Icons
	static const string PLATFROM_PC_ICON_NAME = "platform-windows";
	static const string PLATFROM_XBOX_ICON_NAME = "platform-xbox";
	static const string PLATFROM_PLAYSTATION_ICON_NAME = "platform-playstation";
	static const string PLATFROM_GENERIC_ICON_NAME = "generic-platform";

	static const string ICON_INTERACT_DEFAULT = 	"ingameInteraction";
	static const string ICON_INTERACT_DISABLED = 	"disable";
	static const string ICON_WARNING = 			"warning";
	static const string ICON_OK = 				"okCircle";
	static const string ICON_CANCEL =			"cancelCircle";
	static const string ICON_CHECK = 			"check";
	static const string ICON_NOT_AVAILABLE = 	"not-available";
	
	// Action rich text
	static const string ACTION_DISPLAY_ICON_SCALE_BIG = 		"1.25";
	static const string ACTION_DISPLAY_ICON_SCALE_VERY_BIG =	"1.5";
	static const string ACTION_DISPLAY_ICON_SCALE_HUGE =		"1.75";
	
	// Default Color Enum for nearby interaction
	static const int NEARBY_INTERACTION_DEFAULT_STATE = SCR_ENearbyInteractionContextColors.DEFAULT;
	
	// Values
    static const float DISABLED_WIDGET_OPACITY = 	0.3;
    static const float ENABLED_WIDGET_OPACITY = 	1;
	static const float DISABLED_WIDGET_SATURATION = 0.5;
	static const float ENABLED_WIDGET_SATURATION = 	1;
	
	// Menu base actions
	static const string MENU_ACTION_LEFT = 			"MenuLeft";
	static const string MENU_ACTION_RIGHT = 		"MenuRight";
	static const string MENU_ACTION_UP = 			"MenuUp";
	static const string MENU_ACTION_DOWN = 			"MenuDown";
	static const string MENU_ACTION_BACK = 			"MenuBack";
	
	static const string MENU_ACTION_MOUSE_WHEEL =	"MouseWheel";
	
	static const string MENU_ACTION_SELECT = 		"MenuSelect";
	static const string MENU_ACTION_SELECT_HOLD = 	"MenuSelectHold";
	static const string MENU_ACTION_ENABLE = 		"MenuEnable";
	static const string MENU_ACTION_ENABLE_ALL =	"MenuEnableAll";
	static const string MENU_ACTION_DOUBLE_CLICK =	"MenuSelectDouble";
	static const string MENU_ACTION_FAVORITE =		"MenuFavourite";
	static const string MENU_ACTION_OPEN =			"MenuOpen";
	
	static const string MENU_ACTION_OPEN_WB =		"MenuOpenWB";
	static const string MENU_ACTION_BACK_WB =		"MenuBackWB";
	
	// Input devices
	static const string DEVICE_KEYBOARD = 	"keyboard";
	static const string DEVICE_GAMEPAD = 	"gamepad";
	
	// Mouse buttons
	static const int MOUSE_LEFT_CLICK =		0;
	static const int MOUSE_RIGHT_CLICK =	1;
	
	// Footer buttons
	static const string BUTTON_BACK = "Back";
	
	//------------------------------------------------------------------------------------------------
	//! Convert the action display state to a string for attributes and Rich Text formatting
	static string GetActionDisplayStateAttribute(SCR_EActionDisplayState state)
	{
		return typename.EnumToString(SCR_EActionDisplayState, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get formatted percentage string
	static string FormatUnitPercentage(float percentage)
	{
		return WidgetManager.Translate(VALUE_UNIT_PERCENTAGE, percentage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get formatted +x string
	static string FormatUnitShortPlus(float value)
	{
		return WidgetManager.Translate(VALUE_UNIT_SHORT_PLUS, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get formatted x out of y string. Not condensed adds a space in some languages, ie: x / y instead of  x/y
	static string FormatValueOutOf(float value, float max, bool condensed = true)
	{
		string text = VALUE_OUT_OF;
		if (!condensed)
			text = VALUE_OUT_OF_SPACED;
		
		return WidgetManager.Translate(text, value, max);
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO: LOCALIZED STRING!
	static string FormatVersion(string version)
	{
		return string.Format("v. %1", version);
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetFavoriteLabel(bool isFavorite)
	{
		if (isFavorite)
			return FAVORITE_LABEL_REMOVE;
		else
			return FAVORITE_LABEL_ADD;
	}
	
	//------------------------------------------------------------------------------------------------
	// Depending on seconds given, returns formatted time display strings
	static string FormatSeconds(int totalSeconds)
	{
		int days;
		int hours;
		int minutes;
		int seconds;
		
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(totalSeconds, days, hours, minutes, seconds);

		if (days >= 1)
			return WidgetManager.Translate(TIME_DISPLAY_DAYS_HOURS_MINUTES_SECONDS, days, hours, minutes, seconds);
		
		if (hours >= 1)
			return WidgetManager.Translate(TIME_DISPLAY_HOURS_MINUTES_SECONDS, hours, minutes, seconds);
		
		if (minutes >= 1)
			return WidgetManager.Translate(TIME_DISPLAY_MINUTES_SECONDS, minutes, seconds);
		
		return WidgetManager.Translate(TIME_DISPLAY_SECONDS, seconds);
	}
}

// Input action display state for Rich text widgets
// States allow overriding icon colors in {2F14B8749FE911B4}Configs/WidgetLibrary/SCR_InputButton/SCR_InputButtonLayout.conf
enum SCR_EActionDisplayState
{
	DEFAULT,
	DISABLED,
	NON_INTERACTABLE_HINT,
	WARNING
}

enum SCR_EMouseButtons
{
	LEFT, //Keyboard Enter and Gamepad A are hardcoded to also return 0
	RIGHT,
	MIDDLE
}