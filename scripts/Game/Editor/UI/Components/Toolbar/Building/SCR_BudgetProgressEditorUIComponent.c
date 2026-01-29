class SCR_BudgetProgressEditorUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("")]
	protected string m_sProgressBar;

	[Attribute("")]
	protected string m_sProgressText;

	[Attribute("")]
	protected string m_sProgressAddText;

	[Attribute("")]
	protected string m_sIcon;

	protected SCR_WLibProgressBarComponent m_wProgressBar;
	protected TextWidget m_wProgressText;
	protected TextWidget m_wProgressAddText;
	protected ImageWidget m_wIcon;

	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wProgressBar = SCR_WLibProgressBarComponent.GetProgressBar(m_sProgressBar, w);
		m_wProgressText = TextWidget.Cast(w.FindAnyWidget(m_sProgressText));
		m_wProgressAddText = TextWidget.Cast(w.FindAnyWidget(m_sProgressAddText));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(m_sIcon));
	}

	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------

	//---- REFACTOR NOTE START: Not using localized unit values
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	void ShowBudget(float value)
	{
		m_wProgressBar.SetValue(value);

		float perc = Math.Floor(value * 100);
		m_wProgressText.SetText(perc.ToString() + "%");
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	//! \param[in] currentBudget
	void ShowBudgetChange(float value, int currentBudget)
	{
		m_wProgressAddText.SetVisible(true);
		m_wProgressBar.SetChange(value * 0.01);

		float perc = Math.Floor(value);

		if (value < 0)
			m_wProgressAddText.SetText(perc.ToString() + "%");
		else if (value > 1)
			m_wProgressAddText.SetText("+" + perc.ToString() + "%");
		else
			m_wProgressAddText.SetText("+<1%");

		// Colorize progress change
		if (currentBudget > 100)
		{
			m_wProgressAddText.SetColor(Color.FromInt(UIColors.WARNING.PackToInt()));
			m_wProgressBar.SetSliderChangeColor(Color.FromInt(UIColors.WARNING.PackToInt()));
		}
		else
		{
			m_wProgressAddText.SetColor(Color.FromInt(UIColors.CONFIRM.PackToInt()));
			m_wProgressBar.SetSliderChangeColor(Color.FromInt(UIColors.CONFIRM.PackToInt()));
		}
	}

	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//!
	void HideBudgetChange()
	{
		m_wProgressAddText.SetVisible(false);
		m_wProgressBar.SetChange(0);
	}

	//------------------------------------------------------------------------------------------------
	void SetIconTexture(ResourceName resource, string image = "")
	{
		if (resource.EndsWith("imageset"))
			m_wIcon.LoadImageFromSet(0, resource, image);
		else
			m_wIcon.LoadImageTexture(0, resource);
	}

	//------------------------------------------------------------------------------------------------
	//! Return ImageWidget of budget icon.
	ImageWidget GetIconWidhget()
	{
		return m_wIcon;
	}
}
