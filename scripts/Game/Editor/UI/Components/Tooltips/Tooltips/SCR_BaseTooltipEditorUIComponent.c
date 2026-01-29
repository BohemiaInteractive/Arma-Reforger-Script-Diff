//! @ingroup Editor_UI Editor_UI_Components

class SCR_BaseTooltipEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute()]
	protected string m_sNameWidgetName;
	
	[Attribute()]
	protected string m_sDescriptionWidgetName;
	
	[Attribute()]
	protected string m_sIconWidgetName;	
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	void UpdateTooltip(Managed instance = null);

	//------------------------------------------------------------------------------------------------
	//! \param[in] info
	//! \param[in] instance
	//! \return
	bool SetTooltip(SCR_UIInfo info, Managed instance = null)
	{
		if (!info) return false;
		
		Widget widget = GetWidget();
		TextWidget nameWidget = TextWidget.Cast(widget.FindAnyWidget(m_sNameWidgetName));
		TextWidget descriptionWidget = TextWidget.Cast(widget.FindAnyWidget(m_sDescriptionWidgetName));
		
//		if (info.HasName() && !info.HasDescription())
//		{
//			//--- Only name is defined, use it as a desciption
//			if (nameWidget)
//			{
//				nameWidget.SetVisible(false);
//			}
//			if (descriptionWidget)
//			{
//				info.SetNameTo(descriptionWidget);
//			}
//		}
//		else
//		{
			//--- Name and possibly description defined
			if (nameWidget)
			{
				info.SetNameTo(nameWidget);
				nameWidget.SetVisible(info.HasName());
			}

			if (descriptionWidget)
			{
				info.SetDescriptionTo(descriptionWidget);
				descriptionWidget.SetVisible(info.HasDescription());
			}
//		}
		
		ImageWidget imageWidget = ImageWidget.Cast(widget.FindAnyWidget(m_sIconWidgetName));
		if (imageWidget && !info.SetIconTo(imageWidget))
			imageWidget.SetVisible(false);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsUnique()
	{
		return false;
	}
}
