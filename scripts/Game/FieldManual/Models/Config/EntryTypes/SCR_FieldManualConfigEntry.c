[BaseContainerProps(insertable: false), SCR_BaseContainerLocalizedTitleField("m_sTitle", "Do NOT use SCR_FieldManualConfigEntry class")]
class SCR_FieldManualConfigEntry
{
	[Attribute(defvalue: "1")]
	bool m_bEnabled;

	[Attribute(defvalue: SCR_Enum.GetDefault(EFieldManualEntryId.NONE), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EFieldManualEntryId))]
	EFieldManualEntryId m_eId;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	string m_sTitle;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds")]
	ResourceName m_Image; // m_s

	SCR_FieldManualConfigCategory m_Parent; // no strong ref: if the parent dies, he dies

	[Attribute()]
	ref array<ref SCR_FieldManualPiece> m_aContent;

	protected bool m_bCanRefresh;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManualConfigEntry()
	{
		if (!m_aContent) // can be config-provided
			m_aContent = {};
	}

	//------------------------------------------------------------------------------------------------
	//! To be overridden by child classes to call CreateWidgetFromLayout
	Widget CreateWidget(notnull Widget parent);

	//------------------------------------------------------------------------------------------------
	protected Widget CreateWidgetFromLayout(ResourceName layout, notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(layout, parent);
		if (!createdWidget)
		{
			Print(string.Format("could not create widget from layout \"%1\" | ", layout) + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return null;
		}

		if (!m_aContent)
		{
			Print("no content found (null array) | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return createdWidget;
		}

		Widget piecesLayout = SCR_WidgetHelper.GetWidgetOrChild(createdWidget, "piecesLayout");
		if (!piecesLayout || !m_aContent)
		{
			Print("no pieces layout found | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return createdWidget;
		}

		foreach (SCR_FieldManualPiece piece : m_aContent)
		{
			if (piece.CanCreateWidget())
			{
				piece.CreateWidget(piecesLayout);
				if (!m_bCanRefresh)
					m_bCanRefresh = piece.CanRefresh();
			}
		}

		return createdWidget;
	}

	//------------------------------------------------------------------------------------------------
	//! Used to check if the entry requires a refresh (on e.g KBM/pad input change)
	//! Valid only after CreateWidgetFromLayout has been called!
	//! \return true if there is anything to refresh, false otherwise
	bool CanRefresh()
	{
		return m_bCanRefresh;
	}
}
