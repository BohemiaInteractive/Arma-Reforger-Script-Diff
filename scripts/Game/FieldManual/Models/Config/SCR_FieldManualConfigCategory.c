[BaseContainerProps(configRoot: true), SCR_BaseContainerLocalizedTitleField("m_sTitle", "Category: %1")]
class SCR_FieldManualConfigCategory
{
	[Attribute(defvalue: "1")]
	bool m_bEnabled;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	string m_sTitle;

	[Attribute(uiwidget: UIWidgets.Auto, desc: "Field Manual sub-categories")]
	ref array<ref SCR_FieldManualConfigCategory> m_aCategories;

	[Attribute()]
	ref array<ref SCR_FieldManualConfigEntry> m_aEntries;

	[Attribute(defvalue: "{0D3AF0CFA1D1F9A6}UI/layouts/Menus/FieldManual/MenuParts/FieldManual_MenuCategory.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	ResourceName m_CategoryLayout;

	[Attribute(defvalue: "{A316FF2475A5ADAB}UI/layouts/Menus/FieldManual/MenuParts/FieldManual_MenuSubCategory.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	ResourceName m_SubCategoryLayout;

	SCR_FieldManualConfigCategory m_Parent; // no strong ref: if the parent dies, he dies

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManualConfigCategory()
	{
		if (!m_aCategories) // can be config-provided
		{
			m_aCategories = {};
		}

		if (!m_aEntries) // can be config-provided
		{
			m_aEntries = {};
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] parent
	//! \return
	Widget CreateWidget(notnull Widget parent)
	{
		if (m_aCategories.Count() > 0)
		{
			return CreateMainCategory(parent);
		}
		else
		{
			return CreateSubCategory(parent);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected Widget CreateMainCategory(notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_CategoryLayout, parent);
		if (!createdWidget)
			return null;

		Widget widget = createdWidget.FindAnyWidget("Title");
		if (widget != null)
		{
			TextWidget titleWidget = TextWidget.Cast(widget);
			if (titleWidget)
				titleWidget.SetText(m_sTitle);
		}

		return createdWidget;
	}

	//------------------------------------------------------------------------------------------------
	protected Widget CreateSubCategory(notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_SubCategoryLayout, parent);
		if (!createdWidget)
			return null;

		SCR_ButtonTextComponent buttonTextComponent = SCR_ButtonTextComponent.Cast(createdWidget.FindHandler(SCR_ButtonTextComponent));
		if (buttonTextComponent != null)
			buttonTextComponent.SetText(m_sTitle);

		return createdWidget;
	}
};
