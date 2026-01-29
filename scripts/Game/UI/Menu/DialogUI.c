class DialogUI : ChimeraMenuBase
{
	protected SCR_DialogDataComponent m_DialogData;

	protected Widget m_wCancelButton;
	protected Widget m_wConfirmButton;
	protected SCR_InputButtonComponent m_Cancel;
	protected SCR_InputButtonComponent m_Confirm;

	protected ImageWidget m_wImgTopLine;
	protected ImageWidget m_wImgTitleIcon;

	protected TextWidget m_wTitle;
	protected TextWidget m_wContent;
	ref ScriptInvoker m_OnConfirm = new ScriptInvoker();
	ref ScriptInvoker m_OnCancel = new ScriptInvoker();
	protected float m_fAnimationRate = UIConstants.FADE_RATE_FAST;

	protected EDialogType m_iDialogType;
	
	protected SCR_DynamicFooterComponent m_DynamicFooter;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		Widget w = GetRootWidget();

		// Cancel button
		m_wCancelButton = w.FindAnyWidget("Cancel");
		if(m_wCancelButton)
		{
			m_Cancel = SCR_InputButtonComponent.FindComponent(m_wCancelButton);
			if (m_Cancel)
				m_Cancel.m_OnActivated.Insert(OnCancel);
		}

		// Confirm button
		m_wConfirmButton = w.FindAnyWidget("Confirm");
		if(m_wConfirmButton)
		{
			m_Confirm = SCR_InputButtonComponent.FindComponent(m_wConfirmButton);
			if (m_Confirm)
				m_Confirm.m_OnActivated.Insert(OnConfirm);
		}

		// Images
		m_wImgTopLine = ImageWidget.Cast(w.FindAnyWidget("Separator"));
		m_wImgTitleIcon = ImageWidget.Cast(w.FindAnyWidget("ImgTitleIcon"));

		// Texts
		m_wTitle = TextWidget.Cast(w.FindAnyWidget("Title"));
		m_wContent = TextWidget.Cast(w.FindAnyWidget("Message"));

		// Find dialog component
		m_DialogData = SCR_DialogDataComponent.Cast( w.FindHandler(SCR_DialogDataComponent));

		// Set dialog type
		if (m_DialogData)
			SetDialogType(m_DialogData.m_iDialogType);
		
		SCR_MenuHelper.OnDialogOpen(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{
		super.OnMenuInit();
		
		m_DynamicFooter = SCR_DynamicFooterComponent.FindComponentInHierarchy(GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose() 
	{
		super.OnMenuClose();
		SCR_MenuHelper.OnDialogClose(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("DialogContext");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnConfirm()
	{
		m_OnConfirm.Invoke();
		//CloseAnimated();
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCancel()
	{
		m_OnCancel.Invoke();
		Close();
	}


	//------------------------------------------------------------------------------------------------
	//! Set color based on dialog type
	protected void SetDialogType(EDialogType type)
	{
		m_iDialogType = type;

		// Check widgets
		if (!m_wImgTopLine || !m_wImgTitleIcon)
			return;

		// Select color
		Color color = Color.FromInt(Color.WHITE);

		switch (m_iDialogType)
		{
			case EDialogType.ACTION:
				color = Color.FromInt(UIColors.CONTRAST_CLICKED_HOVERED.PackToInt());
				break;

			case EDialogType.WARNING:
				color = Color.FromInt(UIColors.WARNING.PackToInt());
				break;
			
			case EDialogType.POSITIVE:
				color = Color.FromInt(UIColors.CONFIRM.PackToInt());
				break;
			
			case EDialogType.ONLINE:
				color = Color.FromInt(UIColors.ONLINE.PackToInt());
				break;
		}

		// Set colors
		m_wImgTopLine.SetColor(color);
		m_wImgTitleIcon.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void SetTitle(string text)
	{
		if (m_wTitle)
			m_wTitle.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	string GetTitle()
	{
		if (!m_wTitle)
			return string.Empty;

		return m_wTitle.GetText();
	}

	//------------------------------------------------------------------------------------------------
	void SetMessage(string text)
	{
		if (m_wContent)
			m_wContent.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	string GetMessage()
	{
		if (!m_wContent)
			return string.Empty;

		return m_wContent.GetText();
	}

	//------------------------------------------------------------------------------------------------
	void SetConfirmText(string text)
	{
		if (m_Confirm)
			m_Confirm.SetLabel(text);
	}

	//------------------------------------------------------------------------------------------------
	void SetCancelText(string text)
	{
		if (m_Cancel)
			m_Cancel.SetLabel(text);
	}

	//------------------------------------------------------------------------------------------------
	void SetType(EDialogType type)
	{
		m_iDialogType = type;
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
	//! Set title icon by image name from image set in dialog data
	void SetTitleIcon(string imageName)
	{
		if (!m_DialogData)
			return;

		m_wImgTitleIcon.LoadImageFromSet(0, m_DialogData.m_sIconSet, imageName);
	}

	//------------------------------------------------------------------------------------------------
	//! animates dialog closure
	void CloseAnimated()
	{
		AnimateWidget.Opacity(GetRootWidget(), 0, m_fAnimationRate);
		int delay;
		if (m_fAnimationRate > 0)
			delay = 1 / m_fAnimationRate * 1000;
		GetGame().GetCallqueue().CallLater(Close, delay);
	}

	//------------------------------------------------------------------------------------------------
	SCR_DynamicFooterComponent GetFooterComponent()
	{
		return m_DynamicFooter;
	}
};

enum EDialogType
{
	MESSAGE,
	ACTION,
	WARNING,
	POSITIVE,
	ONLINE
};
