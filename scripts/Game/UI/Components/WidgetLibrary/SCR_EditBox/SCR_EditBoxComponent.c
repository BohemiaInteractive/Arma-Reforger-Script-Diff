// Component handling interaction with text input fields (edit boxes)

// TODO:
// During interaction with edit boxes on pc, write mode does not disable itself if the dialog/menu is overlayed by another.
// This hasn't been a problem so far as there has never been an instance of a dialog/menu opening while an edit box interaction is allowed, but let's keep it in mind nonetheless

//------------------------------------------------------------------------------------------------
class SCR_EditBoxComponent : SCR_ChangeableComponentBase 
{
	[Attribute("0.1")]
	protected float m_fColorsAnimationTime;
	
	[Attribute("0")]
	protected bool m_bShowWriteIcon;
	
	[Attribute("1")]
	protected bool m_bShowInvalidInputWarning;
	
	[Attribute(UIConstants.ICONS_IMAGE_SET, UIWidgets.ResourcePickerThumbnail, "Image resource for icons")]
	protected ResourceName m_sIconImageSet;
	
	[Attribute("")]
	protected string m_sIconImageName;
	
	[Attribute("WriteIconButton")]
	protected string m_sWriteIconButtonName;
	
	[Attribute("0.1", "0 1")]
	protected float m_fOpacityDefault;
	
	[Attribute("0.4", "0 1")]
	protected float m_fOpacityFocused;
	
	[Attribute("0.022995 0.022995 0.022995 1.000000", UIWidgets.ColorPicker)]
	protected ref Color m_BackgroundDefault;
	
	[Attribute("0.108995 0.108995 0.108995 1.000000", UIWidgets.ColorPicker)]
	protected ref Color m_BackgroundInteracting;
	
	[Attribute("0.022995 0.022995 0.022995 1.000000", UIWidgets.ColorPicker)]
	protected ref Color m_BackgroundFocused;
	
	[Attribute("", UIWidgets.EditBox, "Text that should appear next to error warning on invalid input")]
	protected string m_sWarningText;
	
	Widget m_wEditBox;
	Widget m_wColorOverlay;
	Widget m_wEditBackground;
	Widget m_wWarningIcon;
	Widget m_wHorizontalLayout;
	
	Widget m_wWriteIconScale;
	ImageWidget m_wImgWriteIcon;
	
	protected SCR_ModularButtonComponent m_WriteIconButton;
	
	protected SCR_WidgetHintComponent m_Hint;
	
	protected ref Color COLOR_INVALID_INPUT = UIColors.WARNING;
	protected ref Color COLOR_VALID_INPUT 	= Color.White;
	
	// Arguments passed: SCR_EditBoxComponent, string (text)
	ref ScriptInvoker m_OnConfirm = new ScriptInvoker();
	ref ScriptInvoker m_OnFocusChangedEditBox = new ScriptInvoker();
	
	protected MultilineEditBoxWidget m_wMultilineEditBoxWidget;
	protected EditBoxWidget m_wEditBoxWidget;
	protected SCR_EventHandlerComponent m_EVHComponent;
	
	protected const int INTERACTION_STATE_UPDATE_FREQUENCY = 50;
	protected const int FOCUS_LOST_INTERACTION_STATE_UPDATE_DELAY = 1000 / UIConstants.FADE_RATE_DEFAULT;
	
	protected bool m_bIsFocused;
	protected bool m_bIsInWriteMode;
	protected bool m_bIsInWriteModePrevious;
	protected bool m_bValidInput = true;
	protected bool m_bIsTyping;
	protected ref Color m_BackgroundCurrent;
	
	// TODO: protect these
	ref ScriptInvokerVoid m_OnWriteModeEnter = new ScriptInvokerVoid();
	ref ScriptInvokerVoid m_OnCancel = new ScriptInvokerVoid();
	ref ScriptInvokerString m_OnWriteModeLeave = new ScriptInvokerString();
	ref ScriptInvokerString m_OnTextChange = new ScriptInvokerString();
	
	protected string m_sTextPrevious;
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancel()
	{
		m_OnCancel.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wColorOverlay = w.FindAnyWidget("OverlayPanel");
		
		m_wEditBox = w.FindAnyWidget("EditBox");
		m_wWarningIcon = w.FindAnyWidget("ErrorIcon");
		m_wEditBackground = w.FindAnyWidget("EditBackground");
		m_wHorizontalLayout = w.FindAnyWidget("HorizontalLayout");
		
		m_wWriteIconScale = w.FindAnyWidget("WriteIconScale");
		m_wImgWriteIcon = ImageWidget.Cast(w.FindAnyWidget("WriteIcon"));
		
		m_Hint = SCR_WidgetHintComponent.Cast(SCR_WidgetTools.FindHandlerOnWidget(w, "Hint", SCR_WidgetHintComponent));
		
		// Setup warning 
		if (GetGame().InPlayMode() && m_wWarningIcon)
		{
			if (m_bUseLabel)
				m_wWarningIcon.SetOpacity(0);
			else
				m_wWarningIcon.SetOpacity(0); // Without label, warning icon not shown at all
		}

		if (m_Hint)
		{
			m_Hint.SetMessage(m_sWarningText);
			m_Hint.SetVisible(0);
		}
		
		// Setup for label-less edit boxes
		m_wBackground.SetVisible(m_bUseLabel);
		
		// Convert to transition rate
		if (m_fColorsAnimationTime <= 0)
			m_fColorsAnimationTime = 1000;
		else
			m_fColorsAnimationTime = 1 / m_fColorsAnimationTime;
		
		if (!m_wEditBox)
			return;

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// This component needs to operate with two different classes that duplicate most of their functionality, but not all of it's functionalities actually support both! EditBoxFilterComponent	does not work on MultilineEditBoxWidget

		EditBoxFilterComponent filter = EditBoxFilterComponent.Cast(m_wEditBox.FindHandler(EditBoxFilterComponent));
		if (filter)
		{
			if (m_bShowInvalidInputWarning)
			{
				filter.m_OnInvalidInput.Insert(OnInvalidInput);
				filter.m_OnTextTooLong.Insert(OnInvalidInput);
			}

			filter.m_OnValidInput.Insert(OnValueChanged);
		}
		
		m_EVHComponent = SCR_EventHandlerComponent.Cast(m_wEditBox.FindHandler(SCR_EventHandlerComponent));
		if (m_EVHComponent)
		{
			m_EVHComponent.GetOnFocus().Insert(OnHandlerFocus);
			m_EVHComponent.GetOnFocusLost().Insert(OnHandlerFocusLost);
			m_EVHComponent.GetOnChangeFinal().Insert(OnConfirm);
		}
		
		if (m_wEditBackground)
		{
			m_wEditBackground.SetOpacity(m_fOpacityDefault);
			m_wEditBackground.SetColor(m_BackgroundDefault);
			m_BackgroundCurrent = m_BackgroundDefault;
		}
		
		// Image 
		if (m_wImgWriteIcon && m_wWriteIconScale)
		{
			bool hasResource = !m_sIconImageSet.IsEmpty() && !m_sIconImageName.IsEmpty();
			m_wWriteIconScale.SetVisible(hasResource);
			
			if (hasResource)
				m_wImgWriteIcon.LoadImageFromSet(0, m_sIconImageSet, m_sIconImageName);
			
			ShowWriteIcon(m_bShowWriteIcon && hasResource);
		}
		
		// Write Icon button
		Widget iconButton = m_wRoot.FindAnyWidget(m_sWriteIconButtonName);
		if (iconButton)
			m_WriteIconButton = SCR_ModularButtonComponent.FindComponent(iconButton);
		
		if (m_WriteIconButton)
			m_WriteIconButton.m_OnClicked.Insert(OnInternalButtonClicked);
			
		//Type of edit box. Why aren't these derived from a common parent :(
		m_wMultilineEditBoxWidget = MultilineEditBoxWidget.Cast(m_wEditBox);
		m_wEditBoxWidget = EditBoxWidget.Cast(m_wEditBox);

//---- REFACTOR NOTE END ----
		
		if (m_wMultilineEditBoxWidget)
			m_bIsInWriteMode = m_wMultilineEditBoxWidget.IsInWriteMode();
		else if (m_wEditBoxWidget)
			m_bIsInWriteMode = m_wEditBoxWidget.IsInWriteMode();
		
		m_bIsInWriteModePrevious = m_bIsInWriteMode;
		
		m_sTextPrevious = GetEditBoxText();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		ClearInteractionState();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		// Set focus to handler - super.OnFocus will be called only after the handler is focused!
		GetGame().GetWorkspace().SetFocusedWidget(m_wEditBox);
		m_bIsTyping = true;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnValueChanged()
	{
		if (m_Hint)
			m_Hint.SetVisible(false);
		
		m_OnChanged.Invoke(this, GetEditBoxText());

		if (m_bValidInput)
			return;
		
		AnimateWidget.Opacity(m_wWarningIcon, 0, m_fColorsAnimationTime, true);
		
		AnimateWidget.Color(m_wColorOverlay, COLOR_VALID_INPUT, m_fColorsAnimationTime);
		m_bValidInput = true;
		
	}
	
	//------------------------------------------------------------------------------------------------
	void OnHandlerFocus()
	{
		// Call focus event on parent class
		super.OnFocus(m_wRoot, 0, 0);

		m_bIsTyping = true;
		m_bIsFocused = true;
		
		// Make the widget unfocusable
		m_wRoot.SetFlags(WidgetFlags.NOFOCUS);
		m_OnFocusChangedEditBox.Invoke(this, m_wEditBox, true);
		
		UpdateBackgroundColor();
		
		//Update interaction state timer, because there are no delegates for write start and end :(
		if(m_wMultilineEditBoxWidget || m_wEditBoxWidget)
			GetGame().GetCallqueue().CallLater(UpdateInteractionState, INTERACTION_STATE_UPDATE_FREQUENCY, true, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnHandlerFocusLost()
	{
		// Call focusLost event on parent class
		super.OnFocusLost(m_wRoot, 0, 0);

		m_bIsTyping = false;
		m_bIsFocused = false;
		
		// Make focusable again
		if (m_wRoot)
			m_wRoot.ClearFlags(WidgetFlags.NOFOCUS);
		
		m_OnFocusChangedEditBox.Invoke(this, m_wEditBox, false);
		
		ClearInteractionState();
	}

	//------------------------------------------------------------------------------------------------
	void OnInvalidInput()
	{
		// Show simple icon next to text when no message 
		if (!m_Hint || m_Hint && m_Hint.GetMessage().IsEmpty())
			AnimateWidget.Opacity(m_wWarningIcon, 1, m_fColorsAnimationTime, true);
		else if (m_Hint && !m_Hint.GetMessage().IsEmpty()) 
			m_Hint.SetVisible(true);
			
		AnimateWidget.Color(m_wColorOverlay, COLOR_INVALID_INPUT, m_fColorsAnimationTime);
		m_bValidInput = false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearInvalidInput()
	{		
		AnimateWidget.Opacity(m_wWarningIcon, 0, m_fColorsAnimationTime, true);
		if (m_Hint) 
			m_Hint.SetVisible(false);
		
		AnimateWidget.Color(m_wColorOverlay, COLOR_VALID_INPUT, m_fColorsAnimationTime);
		m_bValidInput = false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeOverlayColor(Color color)
	{
		m_wColorOverlay.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetOverlayColor()
	{
		AnimateWidget.Color(m_wColorOverlay, COLOR_VALID_INPUT, m_fColorsAnimationTime);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnConfirm(Widget w)
	{
		if (m_bIsTyping)
			m_OnConfirm.Invoke(this, GetValue()); //TODO: this does not get called on gamepad!?
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInternalButtonClicked()
	{
		ActivateWriteMode(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetEditBoxText()
	{
		if (!m_wEditBox)
			return string.Empty;

		EditBoxWidget editBox = EditBoxWidget.Cast(m_wEditBox);
		if (editBox)
			return editBox.GetText();

		MultilineEditBoxWidget editMulti = MultilineEditBoxWidget.Cast(m_wEditBox);
		if (editMulti)
			return editMulti.GetText();

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEditBoxText(string text)
	{
		if (!m_wEditBox)
			return;

		EditBoxWidget editBox = EditBoxWidget.Cast(m_wEditBox);
		if (editBox)
		{
			editBox.SetText(text);
			return;
		}

		MultilineEditBoxWidget editMulti = MultilineEditBoxWidget.Cast(m_wEditBox);
		if (editMulti)
			editMulti.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetPlaceholderText()
	{
		EditBoxWidget editBox = EditBoxWidget.Cast(m_wEditBox);
		if (editBox)
			return editBox.GetPlaceholderText();
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlaceholderText(string str)
	{
		EditBoxWidget editBox = EditBoxWidget.Cast(m_wEditBox);
		if (editBox)
			editBox.SetPlaceholderText(str);
	}

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_EditBoxComponent GetEditBoxComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		if (!parent || name == string.Empty)
			return null;
		
		Widget w;
		if (searchAllChildren)
			w = parent.FindAnyWidget(name);
		else
			w = parent.FindWidget(name);
		
		if (!w)
			return null;
		
		SCR_EditBoxComponent comp = SCR_EditBoxComponent.Cast(w.FindHandler(SCR_EditBoxComponent));
		return comp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set write mode of editbox handler
	void ActivateWriteMode(bool refocus = false)
	{
		if (refocus)
		{
			Widget target = m_wMultilineEditBoxWidget;
			if (!m_wMultilineEditBoxWidget)
				target = m_wEditBoxWidget;
			
			GetGame().GetWorkspace().SetFocusedWidget(target);
		}
		
		if (m_wMultilineEditBoxWidget)
			m_wMultilineEditBoxWidget.ActivateWriteMode();

		if (m_wEditBoxWidget)
			m_wEditBoxWidget.ActivateWriteMode();
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowHint(bool show)
	{
		if (m_Hint)
			m_Hint.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowWriteIcon(bool show)
	{
		if (m_wWriteIconScale)
			m_wWriteIconScale.SetVisible(show);
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Callqueue workaround due to lack of proper events for write mode
	
	//------------------------------------------------------------------------------------------------
	void UpdateInteractionState(bool forceDisabled)
	{
		int interactionStateUpdateDelay;
		if (forceDisabled)
		{
			m_bIsInWriteMode = false;
			interactionStateUpdateDelay = FOCUS_LOST_INTERACTION_STATE_UPDATE_DELAY;
		}
		else
		{
			//These don't come from a common parent :(
			if (m_wMultilineEditBoxWidget)
				m_bIsInWriteMode = m_wMultilineEditBoxWidget.IsInWriteMode();
			
			else if (m_wEditBoxWidget)
				m_bIsInWriteMode = m_wEditBoxWidget.IsInWriteMode();
		}
		
		if (m_bIsInWriteMode != m_bIsInWriteModePrevious || forceDisabled)
		{
			SCR_MenuHelper.SetActiveWidgetInteractionState(m_bIsInWriteMode, interactionStateUpdateDelay);
			
			UpdateBackgroundColor();
			
			if(m_bIsInWriteMode)
			{
				GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.PRESSED, OnCancel);
				m_OnWriteModeEnter.Invoke();
			}
			else
			{
				GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.PRESSED, OnCancel);
				m_OnWriteModeLeave.Invoke(GetEditBoxText());
			}
		}
		
		m_bIsInWriteModePrevious = m_bIsInWriteMode;
		
		if (GetEditBoxText() != m_sTextPrevious || forceDisabled)
		{
			m_sTextPrevious = GetEditBoxText();
			m_OnTextChange.Invoke(m_sTextPrevious);
		}
	}
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	void ClearInteractionState()
	{
		GetGame().GetCallqueue().Remove(UpdateInteractionState);
		UpdateInteractionState(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateBackgroundColor()
	{
		if (!m_wEditBackground)
			return;
		
		Color idleColor = m_BackgroundDefault;
		if (m_bIsFocused)
			idleColor = m_BackgroundFocused;	
		
		if(m_bIsInWriteMode)
			m_BackgroundCurrent = m_BackgroundInteracting;
		else
			m_BackgroundCurrent = idleColor;
			
		AnimateWidget.Color(m_wEditBackground, m_BackgroundCurrent, m_fAnimationRate);
	}
	
	//------------------------------------------------------------------------------------------------
	//! User API
	void SetValue(string value) { SetEditBoxText(value); }
	string GetValue() { return GetEditBoxText(); }
	string GetPlaceHolderText() { return GetPlaceholderText(); }
	Widget GetEditBoxWidget() { return m_wEditBox; }
	bool IsValidInput() { return m_bValidInput; }
	SCR_WidgetHintComponent GetHint() { return m_Hint; }
	bool IsInWriteMode() { return m_bIsInWriteMode; }
	
	//------------------------------------------------------------------------------------------------
	SCR_EventHandlerComponent GetEVHComponent() 
	{ 
		return m_EVHComponent; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set string value to editbox and confirm
	void ConfirmValue(string value)
	{
		SetEditBoxText(value);
		OnConfirm(m_wRoot);
	}
	
};