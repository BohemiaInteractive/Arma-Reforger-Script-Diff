// TODO: make the number of edit boxes procedural
class SCR_Login2FADialogUI : SCR_LoginProcessDialogUI
{
	protected string m_s2FAText;
	protected string m_sName;
	protected string m_sCode;

	protected ref array<SCR_EditBoxComponent> m_aEditBoxComponents = {};
	
	protected Widget m_wCodeWrapper;
	
	protected const int EDIT_BOX_INITIAL_FOCUS_DELAY = 100;
	protected const int EDIT_BOX_FLICKER_TIME = 400;

	//------------------------------------------------------------------------------------------------
	void SCR_Login2FADialogUI(string name, string code)
	{
		m_sName = name;
		m_sCode = code;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		// Find all edit boxes
		m_wCodeWrapper = m_wRoot.FindAnyWidget("CodeWrapper");
		if (m_wCodeWrapper)
		{
			array<ref Widget> editBoxes = {};
			SCR_WidgetHelper.GetAllChildren(m_wCodeWrapper, editBoxes);
			
			foreach (Widget w : editBoxes)
			{
				SCR_EditBoxComponent comp = SCR_EditBoxComponent.Cast(w.FindHandler(SCR_EditBoxComponent));
				if (!comp)
					continue;
				
				m_aEditBoxComponents.Insert(comp);
				BindEditBoxInputEvent(comp);
			}
		}

		//TODO: there is something resetting the focus to null somewhere, so we need to delay the activation. FIX!
		GetGame().GetCallqueue().CallLater(ActivateFirstEditBox, EDIT_BOX_INITIAL_FOCUS_DELAY);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		GetGame().GetCallqueue().Remove(OnFailDelayed);
		GetGame().GetCallqueue().Remove(ActivateFirstEditBox);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		if (m_bIsLoading)
			return;
		
		string code = m_s2FAText.Trim();
		
		if (!VerifyFormatting(code))
		{
			ShowWarningMessage(true);
			return;
		}
		BohemiaAccountApi.Link(m_Callback, m_sName, m_sCode, code);
		
		super.OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		super.OnTimeout();

		Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFailDelayed(BackendCallback callback)
	{
		super.OnFailDelayed(callback);

		Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// Code must be entirely made out of numbers
	override bool VerifyFormatting(string text)
	{
		if (!IsCodeComplete() || text.Contains(" "))
			return false;
		
		for (int i = 0; i < text.Length(); i++)
		{
			string char = text[i];
			int asciiCode = char.ToAscii();
			
			if (asciiCode < 48 || asciiCode > 57)
				return false;
		}

		return super.VerifyFormatting(text);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Overridden in console version
	protected void ActivateFirstEditBox()
	{
		if (!m_aEditBoxComponents.IsEmpty())
			m_aEditBoxComponents[0].ActivateWriteMode(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Overridden in console version
	protected void BindEditBoxInputEvent(SCR_EditBoxComponent comp)
	{
		comp.m_OnTextChange.Insert(OnTextChange);
	}
	
	//------------------------------------------------------------------------------------------------
	//! If number was inserted into current EditBox, add that number to the string and go to next EditBox. If all are filled, proceed to verification
	protected void OnTextChange(string text)
	{
		if (m_bIsLoading || text.IsEmpty())
			return;
		
		m_s2FAText = string.Empty;
		
		foreach (SCR_EditBoxComponent comp : m_aEditBoxComponents)
		{
			string value = comp.GetValue();
			if (value.IsEmpty())
			{
				comp.ActivateWriteMode(true);
				break;
			}

			m_s2FAText += value;
		}

		if (IsCodeComplete())
			OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	//! We have a number of single digit edit boxes. Overridden in console version
	protected bool IsCodeComplete()
	{
		return m_s2FAText.Length() >= m_aEditBoxComponents.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Clear()
	{
		m_s2FAText = string.Empty;
		
		foreach (SCR_EditBoxComponent comp : m_aEditBoxComponents)
		{
			comp.SetValue(string.Empty);
			
			// Flicker components
			GetGame().GetCallqueue().Remove(comp.ResetOverlayColor);
			
			comp.ChangeOverlayColor(Color.FromInt(UIColors.WARNING.PackToInt()));
			GetGame().GetCallqueue().CallLater(comp.ResetOverlayColor, EDIT_BOX_FLICKER_TIME);
		}
		
		ActivateFirstEditBox();
	}
}

//------------------------------------------------------------------------------------------------
class SCR_Login2FADialogConsoleUI : SCR_Login2FADialogUI
{
	//------------------------------------------------------------------------------------------------
	override void BindEditBoxInputEvent(SCR_EditBoxComponent comp)
	{
		comp.m_OnWriteModeLeave.Insert(OnTextChange);
	}
	
	//------------------------------------------------------------------------------------------------
	// We have a single edit box in which the whole code is inserted
	override bool IsCodeComplete()
	{
		return !m_s2FAText.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	// We don't want to activate the edit box automatically or the dialog will be obscured by the virtual console, so we just focus it
	override void ActivateFirstEditBox()
	{
		if (!m_aEditBoxComponents.IsEmpty())
			GetGame().GetWorkspace().SetFocusedWidget(m_aEditBoxComponents[0].GetRootWidget());
	}
}
