//------------------------------------------------------------------------------------------------
class WelcomeDialogUI: DialogUI
{
	protected string m_sHeaderName = "Title";
	protected string m_sTextName = "Text";
	protected string m_sSelectionHintName = "SelectionHint";
	protected string m_sPreviousName = "Previous";
	protected string m_sNextName = "Next";
	protected string m_sLinksName = "Links";

	protected TextWidget m_wText;
	protected Widget m_wHeader;
	protected Widget m_wLinks;
	protected ref array<ref Widget> m_wLinkButtons = new array<ref Widget>;
	protected Widget m_wLastFocusedLinkButton;
	protected SCR_SelectionHintComponent m_SelectionHint;
	protected SCR_InputButtonComponent m_Previous;
	protected SCR_InputButtonComponent m_Next;

	protected int m_iCurrentIndex;
	protected int m_iMaxIndex;
	protected ref array<string> m_aPageTexts = 
	{
		"#ar-welcomescreen_text_01",
		"#ar-welcomescreen_text_02",
		"#ar-welcomescreen_text_03",
		"#ar-welcomescreen_text_04",
		"#ar-welcomescreen_text_05",
		"#ar-welcomescreen_text_06",
		"#ar-welcomescreen_text_07"
	};

	protected ref array<string> m_aPageTextsPS = 
	{
		"#ar-welcomescreen_text_01",
		"#ar-welcomescreen_text_02",
		"#ar-welcomescreen_text_03",
		"#ar-welcomescreen_text_06"
	};
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		if (System.GetPlatform() == EPlatform.PS5 || System.GetPlatform() == EPlatform.PS4 || System.GetPlatform() == EPlatform.PS5_PRO)
			m_aPageTexts = m_aPageTextsPS;
		
		m_iMaxIndex = m_aPageTexts.Count() - 1;
		Widget w = GetRootWidget();
		m_wText = TextWidget.Cast(w.FindAnyWidget(m_sTextName));
		m_wHeader = w.FindAnyWidget(m_sHeaderName);
		
		m_wLinks = w.FindAnyWidget(m_sLinksName);
		if(m_wLinks)
			SCR_WidgetHelper.GetAllChildren(m_wLinks, m_wLinkButtons);
		
		if(!m_wLinkButtons.IsEmpty())
		{
			SCR_ButtonTextComponent buttonComp;
			foreach(Widget linkButton : m_wLinkButtons)
			{
				buttonComp = SCR_ButtonTextComponent.Cast(linkButton.FindHandler(SCR_ButtonTextComponent));
				if(!buttonComp)
					continue;
				
				buttonComp.m_OnFocus.Insert(OnLinkButtonFocus);
			}
		}
		
		m_Previous = SCR_InputButtonComponent.GetInputButtonComponent(m_sPreviousName, w);
		if (m_Previous)
			m_Previous.m_OnActivated.Insert(OnPrevious);

		m_Next = SCR_InputButtonComponent.GetInputButtonComponent(m_sNextName, w);
		if (m_Next)
			m_Next.m_OnActivated.Insert(OnNext);

		Widget hint = w.FindAnyWidget(m_sSelectionHintName);
		if (hint)
		{
			m_SelectionHint = SCR_SelectionHintComponent.Cast(hint.FindHandler(SCR_SelectionHintComponent));
			if (m_SelectionHint)
			{
				m_SelectionHint.SetItemCount(m_aPageTexts.Count(), false);
				m_SelectionHint.SetCurrentItem(m_iCurrentIndex, false);
			}
		}

		ShowIndex(0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		//! SCR_BrowserLinkComponent already reacts to OnClick events, meaning that the confirm button is only necessary as a visual indicator or if the player clicks it specifically
		if(!m_wLastFocusedLinkButton || GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
			return;
		
		SCR_BrowserLinkComponent linkComp =	SCR_BrowserLinkComponent.Cast(m_wLastFocusedLinkButton.FindHandler(SCR_BrowserLinkComponent));
		linkComp.OpenBrowser();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPrevious()
	{
		ShowIndex(m_iCurrentIndex - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNext()
	{
		ShowIndex(m_iCurrentIndex + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLinkButtonFocus(Widget linkButton)
	{
		SCR_BrowserLinkComponent linkComp =	SCR_BrowserLinkComponent.Cast(linkButton.FindHandler(SCR_BrowserLinkComponent));
		
		m_wLastFocusedLinkButton = linkButton;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowIndex(int index)
	{
		if (index < 0 || index > m_iMaxIndex)
			return;

		m_iCurrentIndex = index;

		if (m_wLinks)
			m_wLinks.SetVisible(index == m_iMaxIndex);
		
		if(index == m_iMaxIndex && !m_wLinkButtons.IsEmpty())
		{
			//! Focus one of the link buttons
			if(!m_wLastFocusedLinkButton)
				m_wLastFocusedLinkButton = m_wLinkButtons[0];
			GetGame().GetWorkspace().SetFocusedWidget(m_wLastFocusedLinkButton);
			
			/*
			if (m_Confirm)
				//m_Confirm.SetVisible(true);
			*/
		}
		else
		{
			/*
			if (m_Confirm)
				m_Confirm.SetVisible(false);
			*/
			
			//! Prevents SCR_BrowserLinkComponent from opening webpages while on another tab
			GetGame().GetWorkspace().SetFocusedWidget(null);
		}
		
		if (m_Previous)
			m_Previous.SetEnabled(index != 0);
		
		if (m_Next)
			m_Next.SetEnabled(index != m_iMaxIndex);
		
		if (m_wText)
			m_wText.SetTextFormat(m_aPageTexts[index]);
		
		if (m_SelectionHint)
			m_SelectionHint.SetCurrentItem(index);
	}
	
};


