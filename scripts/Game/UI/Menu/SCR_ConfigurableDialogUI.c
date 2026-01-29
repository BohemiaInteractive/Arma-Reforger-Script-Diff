//-------------------------------------------------------------------------------------------
//! Configurable dialog can be configured via .conf presets.
//! It has many options for button positions, title, message, colors, style, ...
//!
//! If you add buttons "confirm" or "cancel", they are automatically bound
//! to methods OnCancel and OnConfirm
#define DEBUG_CONFIGURABLE_DIALOGS

void ScriptInvokerConfigurableDialogMethod(SCR_ConfigurableDialogUi dialog);
typedef func ScriptInvokerConfigurableDialogMethod;
typedef ScriptInvokerBase<ScriptInvokerConfigurableDialogMethod> ScriptInvokerConfigurableDialog;

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Despite being a widget component, this is NOT meant to be placed manually in layouts: it is added by script, so it should not have attributes

class SCR_ConfigurableDialogUi: ScriptedWidgetComponent
{
	static const ResourceName NAVIGATION_BUTTON_LAYOUT = "{87037226B1A2064B}UI/layouts/WidgetLibrary/Buttons/WLib_NavigationButtonSuperSmall.layout";
	
	// Attributes
	[Attribute((1/UIConstants.FADE_RATE_FAST).ToString(), UIWidgets.Auto, "Duration of fade in and fade out animations")]
	protected float m_fFadeInTime;
	
	[Attribute(NAVIGATION_BUTTON_LAYOUT, UIWidgets.ResourceNamePicker, "Layout of the navigation button", params: "layout")]
	protected ResourceName m_sNavigationButtonLayout;

//---- REFACTOR NOTE END ----
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// A unified solution for footers should be used instead
	
	[Attribute("ButtonsLeft", UIWidgets.Auto, "Widget name of left buttons layout")]
	protected string m_sWidgetNameButtonsLeft;
	
	[Attribute("ButtonsRight", UIWidgets.Auto, "Widget name of right buttons layout")]
	protected string m_sWidgetNameButtonsRight;
	
	[Attribute("ButtonsCenter", UIWidgets.Auto, "Widget name of center buttons layout")]
	protected string m_sWidgetNameButtonsCenter;
	
//---- REFACTOR NOTE END ----
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Old untyped invokers
	
	// Script Invokers
	ref ScriptInvoker m_OnConfirm = new ScriptInvoker();			// (SCR_ConfigurableDialogUi dlg) - Called when Confirm button was clicked, if you don't override OnConfirm of this class
	ref ScriptInvoker m_OnCancel = new ScriptInvoker();				// (SCR_ConfigurableDialogUi dlg) - Called when Cancel button was clicked, if you don't override OnConfirm of this class
	ref ScriptInvoker m_OnClose = new ScriptInvoker();				// (SCR_ConfigurableDialogUi dlg) - Called when dialog is closed, AFTER the fade out animation is over
	ref ScriptInvoker m_OnCloseStart = new ScriptInvoker();			// (SCR_ConfigurableDialogUi dlg) - Called when dialog is closed, as soon as the fade out animation STARTS
	ref ScriptInvoker m_OnButtonPressed = new ScriptInvoker(); 		// (SCR_ConfigurableDialogUi dlg, string tag) - Generic delegate called by all the buttons, returning their EStage

//---- REFACTOR NOTE END ----
	
	// Widgets
	protected ImageWidget m_wImgTopLine;
	protected ImageWidget m_wImgTitleIcon;
	protected TextWidget m_wTitle;
	protected TextWidget m_wMessage;
	protected VerticalLayoutWidget m_wContentVerticalLayout;
	
	// Other
	// TODO: let the dynamic footer handle keeping track of buttons
	protected ref map<string, SCR_InputButtonComponent> m_aButtonComponents = new map<string, SCR_InputButtonComponent>;
	
	protected Widget m_wRoot;
	protected OverlayWidget m_wDialogBase;
	
	protected SCR_DynamicFooterComponent m_DynamicFooter;
	
	// Menu handler which performs menu-specific actions.
	protected MenuBase m_ProxyMenu;
	
	// Current dialog preset for data 
	protected ref SCR_ConfigurableDialogUiPreset m_DialogPreset;
	
	// Tag of the last button Button pressed
	protected string m_sLastPressedButtonTag;
	
	// Prevents Close() to be called multiple times
	protected bool m_bIsClosing;
	
	protected static SCR_ConfigurableDialogUi m_CurrentDialog;
	
	static const string BUTTON_CONFIRM = "confirm";
	static const string BUTTON_CANCEL = "cancel";

	
	//-------------------------------------------------------------------------------------------------------------------------
	//                                             P U B L I C   A P I 
	//-------------------------------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//! Creates a dialog from preset
	static SCR_ConfigurableDialogUi CreateFromPreset(ResourceName presetsResourceName, string tag, SCR_ConfigurableDialogUi customDialogObj = null)
	{
		// Create presets
		Resource rsc = BaseContainerTools.LoadContainer(presetsResourceName);
		BaseContainer container = rsc.GetResource().ToBaseContainer();
		SCR_ConfigurableDialogUiPresets presets = SCR_ConfigurableDialogUiPresets.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		// Find preset
		SCR_ConfigurableDialogUiPreset preset = presets.FindPreset(tag);
		
		// Bail if preset is not found
		if (!preset)
		{
			Print(string.Format("[SCR_ConfigurableDialogUi] Preset was not found: %1, %2", presets, tag), LogLevel.ERROR);
			return null;
		}
		
		SCR_ConfigurableDialogUi dialog = CreateByPreset(preset, customDialogObj);

		#ifdef DEBUG_CONFIGURABLE_DIALOGS
			Print(string.Format("[SCR_ConfigurableDialogUi] presetsResourceName: %1, tag: %2, customDialogObj: %3", presetsResourceName, tag, customDialogObj));
		#endif
		
		return dialog;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi GetCurrentDialog()
	{
		return m_CurrentDialog;
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetCurrentDialogTag()
	{
		if (m_CurrentDialog)
			return m_CurrentDialog.GetDialogPreset().m_sTag;

		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsPresetValid(ResourceName presetsResourceName, string tag)
	{
		// Create presets
		Resource rsc = BaseContainerTools.LoadContainer(presetsResourceName);
		BaseContainer container = rsc.GetResource().ToBaseContainer();
		SCR_ConfigurableDialogUiPresets presets = SCR_ConfigurableDialogUiPresets.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		
		// Find preset
		return presets.FindPreset(tag);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static SCR_ConfigurableDialogUi CreateByPreset(SCR_ConfigurableDialogUiPreset preset, SCR_ConfigurableDialogUi customDialogObj = null)
	{
		// Open the proxy dialog
		SCR_ConfigurableDialogUiProxy proxyComp = SCR_ConfigurableDialogUiProxy.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ConfigurableDialog));
		
		// Create the base configurable dialog layout inside proxy
		Widget internalWidget = GetGame().GetWorkspace().CreateWidgets(preset.m_sLayout, proxyComp.GetRootWidget());
		
		if (!internalWidget)
		{
			Print(string.Format("[SCR_ConfigurableDialogUi] internalWidget wans't created"), LogLevel.ERROR);
			return null;
		}
		
		// Create the content layout inside the content area of the base layout, if there should be one
		if(!preset.m_sContentLayout.IsEmpty())
		{
			Widget contentContainer = GetContentWidget(internalWidget);
			
			if(!contentContainer)
			{
				Print(string.Format("[SCR_ConfigurableDialogUi] CreateByPreset() contentContainer wasn't found"), LogLevel.ERROR);
				return null;
			}
			
			Widget contentWidget = GetGame().GetWorkspace().CreateWidgets(preset.m_sContentLayout, contentContainer);
			
			AlignableSlot.SetHorizontalAlign(contentWidget, LayoutHorizontalAlign.Stretch);
		}
		
		SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.Cast(internalWidget.FindHandler(SCR_ConfigurableDialogUi));
		
		// Create a new dialog object, or apply the provided one, if the dialog obj was not found in the layout.
		if (!dialog)
		{
			if (customDialogObj)
				dialog = customDialogObj;
			else
				dialog = new SCR_ConfigurableDialogUi();
			dialog.InitAttributedVariables();
			internalWidget.AddHandler(dialog);
		}
		
		dialog.Init(internalWidget, preset, proxyComp);
		proxyComp.Init(dialog);
		
		// Set action context
		if (!preset.m_sActionContext.IsEmpty())
			proxyComp.SetActionContext(preset.m_sActionContext);
		
		// Call dialog's events manually
		dialog.OnMenuOpen(preset);
		
		m_CurrentDialog = dialog;
		return dialog;
	}
	
	//----------------------------------------------------------------------------------------
	//! Returns the container in which to place the content layout. Container must be called ContentLayoutContainer
	protected static Widget GetContentWidget(Widget baseWidget)
	{
		if(!baseWidget)
		{
			Print(string.Format("[SCR_ConfigurableDialogUi] GetContentWidet(): invalid base Widget"), LogLevel.ERROR);
			return null;
		}	
		
		Widget contentContainer = baseWidget.FindAnyWidget("ContentLayoutContainer");
		
		if(!contentContainer)
		{
			Print(string.Format("[SCR_ConfigurableDialogUi] GetContentWidet(): couldn't find ContentLayoutContainer widget in base layout"), LogLevel.ERROR);
			return null;
		}
		
		return contentContainer;
	}
	
	//----------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//----------------------------------------------------------------------------------------
	//! Returns the base dialog overlay (the rectangle covered by the background). Useful for dialog wide darkening (e.g. by SCR_LoadingOverlay)
	OverlayWidget GetDialogBaseOverlay()
	{
		return m_wDialogBase;
	}
	
	//----------------------------------------------------------------------------------------
	//! Returns the root of the content layout
	Widget GetContentLayoutRoot()
	{	
		return GetContentWidget(GetRootWidget()).GetChildren();
	}
	
	//----------------------------------------------------------------------------------------
	SCR_ConfigurableDialogUiPreset GetDialogPreset()
	{
		return m_DialogPreset;
	}
	
	//------------------------------------------------------------------------------------------------
	// Closes the dialogue with a fade-out animation
	void Close()
	{
		if (m_bIsClosing)
			return;
		
		m_bIsClosing = true;
		m_OnCloseStart.Invoke(this);
		Internal_Close();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SetTitle(string text)
	{
		if (m_wTitle)
			m_wTitle.SetText(text);
	}
	

	//------------------------------------------------------------------------------------------------
	void SetMessage(string text)
	{
		if (m_wMessage)
		{
			m_wMessage.SetVisible(true);
			m_wMessage.SetText(text);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMessageColor(Color color)
	{
		if (m_wMessage)
			m_wMessage.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	TextWidget GetMessageWidget()
	{
		return m_wMessage;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetMessageStr()
	{
		if (m_wMessage)
			return m_wMessage.GetText();
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set title icons with custom image 
	void SetTitleIcon(ResourceName image, string imageName)
	{
		if (!m_wImgTitleIcon)
			return;
		
		//  Set image by input 
		if (image.EndsWith("imageset"))
			m_wImgTitleIcon.LoadImageFromSet(0, image, imageName);
		else
			m_wImgTitleIcon.LoadImageTexture(0, image);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set title icons with custom image 
	void SetIconColor(Color color)
	{
		if (!m_wImgTitleIcon)
			return;
		
		m_wImgTitleIcon.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets colors based on style
	void SetStyle(EConfigurableDialogStyle type)
	{	
		// Check widgets 
		if (!m_wImgTopLine || !m_wImgTitleIcon)
			return;
		
		// Select color
		Color color = Color.FromInt(Color.WHITE);
		
		switch (type)
		{
			case EConfigurableDialogStyle.ACTION:
				color = Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt());
				break;
			
			case EConfigurableDialogStyle.WARNING:
				color = Color.FromInt(UIColors.WARNING.PackToInt());
				break;
			
			case EConfigurableDialogStyle.POSITIVE:
				color = Color.FromInt(UIColors.CONFIRM.PackToInt());
				break;
			
			case EConfigurableDialogStyle.ONLINE:
				color = Color.FromInt(UIColors.ONLINE.PackToInt());
				break;
		}
		
		// Set colors 
		m_wImgTopLine.SetColor(color);
		m_wImgTitleIcon.SetColor(color);
	}
	
	//----------------------------------------------------------------------------------------
	//! Allows to register a custom button to call OnConfirm (i.e. one not in the conf files but placed by hand in the layout)
	void BindButtonConfirm(SCR_InputButtonComponent button)
	{
		if (!button)
			return;
		
		button.m_OnActivated.Insert(OnConfirm);
		
		//! Cache
		if(m_aButtonComponents.Contains(BUTTON_CONFIRM))
			m_aButtonComponents.Set(BUTTON_CONFIRM, button);
		else
			m_aButtonComponents.Insert(BUTTON_CONFIRM, button);
	}
	
	//----------------------------------------------------------------------------------------
	//! Allows to register a custom button to call OnCancel (i.e. one not in the conf files but placed by hand in the layout)
	void BindButtonCancel(SCR_InputButtonComponent button)
	{
		if (!button)
			return;
		
		button.m_OnActivated.Insert(OnCancel);
		
		//! Cache
		if(m_aButtonComponents.Contains(BUTTON_CANCEL))
			m_aButtonComponents.Set(BUTTON_CANCEL, button);
		else
			m_aButtonComponents.Insert(BUTTON_CANCEL, button);
	}
	
	//----------------------------------------------------------------------------------------
	//! Allows to register a custom button to call OnButtonPressed (i.e. one not in the conf files but placed by hand in the layout). An empty tag will result in the widget's name being used instead
	void BindButtonGeneric(SCR_InputButtonComponent button, string tag = string.Empty)
	{
		if (!button)
			return;
		
		button.m_OnActivated.Insert(OnButtonPressed);
		
		//! Cache
		Widget root = button.GetRootWidget();
		string buttonTag = tag;
		if (root && buttonTag.IsEmpty())
			buttonTag = root.GetName();
		
		if(m_aButtonComponents.Contains(buttonTag))
			m_aButtonComponents.Set(buttonTag, button);
		else
			m_aButtonComponents.Insert(buttonTag, button);
	}
	
	//----------------------------------------------------------------------------------------
	//! Returns a button with given tag
	SCR_InputButtonComponent FindButton(string tag)
	{
		return m_aButtonComponents.Get(tag);
	}
	
	//----------------------------------------------------------------------------------------
	//! Returns a button's tag
	string GetButtonTag(SCR_InputButtonComponent button)
	{
		if(!button)
			return string.Empty;
		
		return SCR_MapHelper<string, SCR_InputButtonComponent>.GetKeyByValue(m_aButtonComponents, button);
	}
	
	// --- Protected ---
	//----------------------------------------------------------------------------------------
	protected void OnConfirm()
	{
		m_OnConfirm.Invoke(this);
		Close();
	}
	
	
	//----------------------------------------------------------------------------------------
	protected void OnCancel()
	{
		m_OnCancel.Invoke(this);
		Close();
	}
	
	
	//----------------------------------------------------------------------------------------
	protected void OnButtonPressed(SCR_InputButtonComponent button)
	{
		m_sLastPressedButtonTag = GetButtonTag(button);
		m_OnButtonPressed.Invoke(this, m_sLastPressedButtonTag);
	}
	
	//----------------------------------------------------------------------------------------
	//! Called last of all, after all the initialization of main element done.
	//! Here you can perform custom initialization.
	protected void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset);
	
	// Menu events
	void OnMenuUpdate(float tDelta);
	void OnMenuFocusGained();
	void OnMenuFocusLost();
	void OnMenuShow();
	void OnMenuHide();
	void OnMenuClose();
	
	//----------------------------------------------------------------------------------------
	protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		m_ProxyMenu = proxyMenu;
		m_DialogPreset = preset;
		m_wRoot = root;
		
		InitWidgets();
		
		// Set title
		SetTitle(preset.m_sTitle);
		
		// Set message
		if (!preset.m_sMessage.IsEmpty())
		{
			SetMessage(preset.m_sMessage);
		}
		
		// Set style
		SetStyle(preset.m_eVisualStyle);
		
		// Set title image
		if(m_wImgTitleIcon)
			m_wImgTitleIcon.SetVisible(preset.m_bShowIcon);
		
		if (!preset.m_sTitleIconTexture.IsEmpty())
		{
			SetTitleIcon(preset.m_sTitleIconTexture, preset.m_sTitleIconImageName);
		}
		
		// Footer
		m_DynamicFooter = SCR_DynamicFooterComponent.FindComponentInHierarchy(GetRootWidget());
		
		// Create buttons
		HorizontalLayoutWidget wLeft =		HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsLeft));
		HorizontalLayoutWidget wRight =		HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsRight));
		HorizontalLayoutWidget wCenter =	HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsCenter));
		
		foreach (SCR_ConfigurableDialogUiButtonPreset buttonPreset : preset.m_aButtons)
		{	
			CreateButton(buttonPreset);
		}
		
		// Bind actions to default buttons (if they were created)
		SCR_InputButtonComponent buttonConfirm = FindButton(BUTTON_CONFIRM);
		BindButtonConfirm(buttonConfirm);
		
		SCR_InputButtonComponent buttonCancel = FindButton(BUTTON_CANCEL);
		BindButtonCancel(buttonCancel);
	}
	
	//----------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		Widget w = m_wRoot;
		
		// Images 
		m_wImgTopLine = ImageWidget.Cast(w.FindAnyWidget("Separator"));
		m_wImgTitleIcon = ImageWidget.Cast(w.FindAnyWidget("ImgTitleIcon"));

		// Texts 
		m_wTitle = TextWidget.Cast(w.FindAnyWidget("Title"));
		m_wMessage = TextWidget.Cast(w.FindAnyWidget("Message"));
		
		// Verical layout widget
		m_wContentVerticalLayout = VerticalLayoutWidget.Cast(w.FindAnyWidget("ContentVerticalLayout"));
		
		// Base Overlay
		m_wDialogBase = OverlayWidget.Cast(w.FindAnyWidget("DialogBase"));
	}
	
	//----------------------------------------------------------------------------------------
	protected SCR_InputButtonComponent CreateButton(SCR_ConfigurableDialogUiButtonPreset buttonPreset)
	{
		HorizontalLayoutWidget wLayout;
		float padding, paddingLeft, paddingRight;
		switch (buttonPreset.m_eAlign)
		{
			case EConfigurableDialogUiButtonAlign.LEFT:
				wLayout = HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsLeft));
				break;
			
			case EConfigurableDialogUiButtonAlign.RIGHT:
				wLayout = HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsRight));
				break;
			
			default:
				wLayout = HorizontalLayoutWidget.Cast(GetRootWidget().FindAnyWidget(m_sWidgetNameButtonsCenter)); break;
		}
		
		Widget wButton = GetGame().GetWorkspace().CreateWidgets(m_sNavigationButtonLayout, wLayout);
		wButton.SetName(buttonPreset.m_sTag);
		
		//! Handle padding
		// TODO: let the dynamic footer handle the padding entirely
		float left, top, bottom;
		AlignableSlot.GetPadding(wButton, left, top, padding, bottom);
		switch (buttonPreset.m_eAlign)
		{
			case EConfigurableDialogUiButtonAlign.LEFT:
				paddingRight = padding;
				break;
			
			case EConfigurableDialogUiButtonAlign.RIGHT:
				paddingLeft = padding;
				break;
			
			default: break;
		}
		
		AlignableSlot.SetPadding(wButton, paddingLeft, 0.0, paddingRight, 0.0);
		
		// Button setup
		SCR_InputButtonComponent comp = SCR_InputButtonComponent.Cast(wButton.FindHandler(SCR_InputButtonComponent));
			
		if (!comp)
			return null;
		
		comp.SetVisible(buttonPreset.m_bShowButton, false);
		comp.SetLabel(buttonPreset.m_sLabel);
		comp.SetAction(buttonPreset.m_sActionName);
		
		comp.SetHoverSound(buttonPreset.m_sSoundHovered);
		comp.SetClickedSound(buttonPreset.m_sSoundClicked);
		
		comp.m_OnActivated.Insert(OnButtonPressed);
		
		// Cache
		m_aButtonComponents.Insert(buttonPreset.m_sTag, comp);
		
		if (m_DynamicFooter)
			m_DynamicFooter.RegisterButton(comp);
		
		return comp;
	}
	
	//----------------------------------------------------------------------------------------
	protected void Internal_Close()
	{
		if (!m_ProxyMenu)
			return;
		
		m_ProxyMenu.Close();
		m_OnClose.Invoke(this);
	}
	
	//----------------------------------------------------------------------------------------
	//! Verifies that all attributed variables are set up.
	//! When we create this object with 'new' keyword, the game doesn't assign default values to attributed variables.
	protected void InitAttributedVariables()
	{	
		if (m_sNavigationButtonLayout.IsEmpty())
			m_sNavigationButtonLayout = NAVIGATION_BUTTON_LAYOUT;
		
		if (m_sWidgetNameButtonsLeft.IsEmpty())
			m_sWidgetNameButtonsLeft = "ButtonsLeft";
		
		if (m_sWidgetNameButtonsRight.IsEmpty())
			m_sWidgetNameButtonsRight = "ButtonsRight";
		
		if (m_sWidgetNameButtonsCenter.IsEmpty())
			m_sWidgetNameButtonsCenter = "ButtonsCenter";
		
		if (m_fFadeInTime == 0)
			m_fFadeInTime = 1/UIConstants.FADE_RATE_FAST;
	}
}

//-------------------------------------------------------------------------------------------
//! It is here to expose the Menu API to the configurable dialog instance.
class SCR_ConfigurableDialogUiProxy : DialogUI
{
	protected SCR_ConfigurableDialogUi m_Dlg;
	
	void Init(SCR_ConfigurableDialogUi dlg)
	{
		m_Dlg = dlg;
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		m_Dlg.OnMenuUpdate(tDelta);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		
		if (m_Dlg)
			m_Dlg.OnMenuFocusGained();
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		
		if (m_Dlg)
			m_Dlg.OnMenuFocusLost();
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		if (m_Dlg)
			m_Dlg.OnMenuShow();
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		
		if (m_Dlg)
			m_Dlg.OnMenuHide();
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		if (m_Dlg)
			m_Dlg.OnMenuClose();
	}
};


//-------------------------------------------------------------------------------------------
//! Configuration for button alignment
enum EConfigurableDialogUiButtonAlign
{
	LEFT = 0,
	RIGHT,
	CENTER
};


//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Duplicate of EDialogType

//------------------------------------------------------------------------------------------------
//! Style of the dialog
enum EConfigurableDialogStyle
{
	NEUTRAL,
	ACTION,
	WARNING,
	POSITIVE,
	ONLINE
};

//---- REFACTOR NOTE END ----

//-------------------------------------------------------------------------------------------
//		C O N F I G U R A T I O N   C L A S S E S 
//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------
//! Configuration for a button
[BaseContainerProps(configRoot : true), SCR_BaseContainerCustomTitleField("m_sTag")]
class SCR_ConfigurableDialogUiButtonPreset
{
	[Attribute("", UIWidgets.Auto, "Custom tag, used for finding this button at run time")]
	string m_sTag;
	
	[Attribute("", UIWidgets.Auto, "Action name the button will listen to")]
	string m_sActionName;
	
	[Attribute("", UIWidgets.Auto, "Label of the button")]
	string m_sLabel;
	
	[Attribute("0", UIWidgets.ComboBox, "Alignment of the button", "", ParamEnumArray.FromEnum(EConfigurableDialogUiButtonAlign))]
	EConfigurableDialogUiButtonAlign m_eAlign;
	
	[Attribute(SCR_SoundEvent.SOUND_FE_BUTTON_HOVER, UIWidgets.EditBox, "")]
	string m_sSoundHovered;

	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox, "")]
	string m_sSoundClicked;
	
	[Attribute("1", UIWidgets.CheckBox)]
	bool m_bShowButton;
};

//-------------------------------------------------------------------------------------------
//! Configuration for a dialog
[BaseContainerProps(configRoot : true), SCR_BaseContainerCustomTitleField("m_sTag")]
class SCR_ConfigurableDialogUiPreset
{
	[Attribute("{E6B607B27BCC1477}UI/layouts/Menus/Dialogs/ConfigurableDialog.layout", UIWidgets.ResourceNamePicker, ".layout for the base of the dialog", params: "layout")]
	ResourceName m_sLayout;
	
	[Attribute("", UIWidgets.ResourceNamePicker, ".layout for the content of the dialog", params: "layout")]
	ResourceName m_sContentLayout;
	
	[Attribute("", UIWidgets.Auto, "Custom tag, used for finding this preset at run time.")]
	string m_sTag;
	
	[Attribute("0", UIWidgets.ComboBox, "Visual style. Affects color of elements.", "", ParamEnumArray.FromEnum(EDialogType))]
	EDialogType m_eVisualStyle;
	
	[Attribute(UIConstants.ICONS_IMAGE_SET, UIWidgets.ResourcePickerThumbnail, "Texture or imageset for title icon", "edds imageset")]
	ResourceName m_sTitleIconTexture;
	
	[Attribute("1")]
	bool m_bShowIcon;
	
	[Attribute("misc", UIWidgets.Auto)]
	string m_sTitleIconImageName;
	
	[Attribute("", UIWidgets.Auto, "Message to be displayed inside, if messaage widget is present.")]
	string m_sMessage;
	
	[Attribute("", UIWidgets.Auto, "Title of the dialog")]
	string m_sTitle;
	
	[Attribute()]
	ref array<ref SCR_ConfigurableDialogUiButtonPreset> m_aButtons;
	
	[Attribute(string.Empty, UIWidgets.EditBox, "Action context of the menu. If empty, MenuContext is used, same as in standard menu manager.")]
	string m_sActionContext;
};


//-------------------------------------------------------------------------------------------
//! Class for a .conf file with multiple presets.
[BaseContainerProps(configRoot : true)]
class SCR_ConfigurableDialogUiPresets
{
	[Attribute()]
	ref array<ref SCR_ConfigurableDialogUiPreset> m_aPresets;
	
	
	//-------------------------------------------------------------------------------------------
	//! Finds a preset by tag
	SCR_ConfigurableDialogUiPreset FindPreset(string tag)
	{
		foreach (auto i : m_aPresets)
		{
			if (i.m_sTag == tag)
				return i;
		}
		
		return null;
	}
};