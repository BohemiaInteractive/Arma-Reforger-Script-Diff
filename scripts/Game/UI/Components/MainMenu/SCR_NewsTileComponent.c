class SCR_NewsTileComponent : SCR_TileBaseComponent
{
	[Attribute("Title")]
	string m_sTitleName;

	[Attribute("Description")]
	string m_sDescriptionName;

	[Attribute("Image")]
	string m_sImageName;

	[Attribute("Author")]
	string m_sAuthorName;

	[Attribute("Date")]
	string m_sDateName;

	[Attribute("Footer")]
	string m_sFooterName;

	[Attribute("Unread")]
	string m_sUnreadImageName;

	[Attribute("Read")]
	string m_sReadButtonName;

	Widget m_wFooter;
	Widget m_wUnreadImage;
	TextWidget m_wTitle;
	TextWidget m_wAuthor;
	TextWidget m_wDate;
	TextWidget m_wDescription;
	ref SCR_NewsEntry m_Entry;

	SCR_InputButtonComponent m_Read;

	ref ScriptInvoker m_OnRead = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wUnreadImage = w.FindAnyWidget(m_sUnreadImageName);
		m_wTitle = TextWidget.Cast(w.FindAnyWidget(m_sTitleName));
		m_wAuthor = TextWidget.Cast(w.FindAnyWidget(m_sAuthorName));
		m_wDate = TextWidget.Cast(w.FindAnyWidget(m_sDateName));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget(m_sDescriptionName));
		m_wFooter = w.FindAnyWidget(m_sFooterName);

		if (!m_sReadButtonName.IsEmpty())
			m_Read = SCR_InputButtonComponent.GetInputButtonComponent(m_sReadButtonName, w);

		if (m_Read)
			m_Read.m_OnActivated.Insert(OnRead);

		if (!m_wFooter)
			return;

		m_wFooter.SetOpacity(0);
		m_wFooter.SetEnabled(false);
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		UpdateConnectionButtons();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_wFooter)
		{
			m_wFooter.SetEnabled(true);
			AnimateWidget.Opacity(m_wFooter, 1, m_fAnimationRate);
		}

		SetRead();

		m_OnFocused.Invoke(this);
		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_wFooter)
		{
			AnimateWidget.Opacity(m_wFooter, 0, m_fAnimationRate);
			m_wFooter.SetEnabled(false);
		}
		
		return super.OnFocusLost(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{	
		UpdateConnectionButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateConnectionButtons()
	{
		SCR_ConnectionUICommon.ForceConnectionButtonEnabled(m_Read, SCR_ServicesStatusHelper.IsBackendConnectionAvailable());
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRead()
	{
		if (!m_Entry || m_Entry.m_bRead)
			return;

		m_Entry.m_bRead = true;
		if (m_wUnreadImage)
			AnimateWidget.Opacity(m_wUnreadImage, 0, UIConstants.FADE_RATE_DEFAULT);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entry
	void ShowTile(SCR_NewsEntry entry)
	{
		m_wRoot.SetOpacity(entry != null);
		m_wRoot.SetEnabled(entry != null);

		m_Entry = entry;
		if (!entry)
			return;

		if (m_wTitle)
			m_wTitle.SetText(entry.m_Item.Title());

		if (m_wDescription)
			m_wDescription.SetText(entry.m_Item.Excerpt());

		if (m_wAuthor)
			m_wAuthor.SetText(entry.m_Item.Slug());

		if (m_wDate)
			m_wDate.SetText(entry.m_Item.Date());

		string imagePreview = entry.m_Item.Path();

		if (m_wImage && !imagePreview.IsEmpty())
		{
			// Try to load image
			bool loaded = m_wImage.LoadImageTexture(1, imagePreview, false, true);

			// Set image 0 - Placeholder if load image failed
			// Set image 1 - Downloaded image
			m_wImage.SetImage(loaded);

			// Refresh the size
			int sx, sy;
			m_wImage.GetImageSize(loaded, sx, sy);
			m_wImage.SetSize(sx, sy);
		}

		if (m_wUnreadImage)
			m_wUnreadImage.SetOpacity(!entry.m_bRead);
	}

	//------------------------------------------------------------------------------------------------
	void OnRead()
	{
		SetRead();
		m_OnRead.Invoke(this);
	}
}
