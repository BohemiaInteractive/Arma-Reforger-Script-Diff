[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sDisplayName", "Config Entry: %1")]
class SCR_FieldManualPiece_ConfigEntry : SCR_FieldManualPiece
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected string m_sDisplayName;

	[Attribute(desc: "Config or Entity to parse", params: "conf et")]
	protected ResourceName m_ConfigPath;

	[Attribute(defvalue:"path/to/entry", desc: "in format \"level1/level2/level3/entryName\", entry name is CASE-SENSITIVE for an Entity (see SCR_ConfigHelper.GetChildBaseContainer for more information about the format)")]
	protected string m_sEntryPath;

	[Attribute(desc: "Use %1 to display the entry's value")]
	protected string m_sValueFormat;

	[Attribute(defvalue: "{671572B5C56766B3}UI/layouts/Menus/FieldManual/Pieces/FieldManual_Piece_ConfigEntry.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	protected ResourceName m_Layout;

	[Attribute(defvalue: "0", params: "-9 +9", desc: "move a number's decimal by # steps; -1 to the left, +1 to the right: -3 = divided by 1000, 6 = multiplied by one million, etc. Has no effects on other value types.")]
	protected int m_iDecimalMove;

	[Attribute(defvalue: "-1", params: "-1 5", desc: "decides a number's decimals. -1 = no decimals limit, 0 = rounded")]
	protected int m_iFixedDecimals;

	protected ref Resource m_Resource;
	protected BaseContainer m_BaseContainer;

	//------------------------------------------------------------------------------------------------
	protected void InitContainerAndPaths()
	{
		if (m_ConfigPath.IsEmpty() || m_sEntryPath.IsEmpty())
			return;

		m_Resource = Resource.Load(m_ConfigPath);
		if (!m_Resource.IsValid())
		{
			m_Resource = null;
			return;
		}

		m_BaseContainer = SCR_ConfigHelper.GetBaseContainerByPath(m_Resource, m_sEntryPath, true);

		array<string> paths = {};
		m_sEntryPath = SCR_ConfigHelper.SplitConfigPath(m_sEntryPath, paths, true); // note, m_sEntryPath gets recycled here
		if (!m_ConfigPath.EndsWith(".et")) // .et entries are case-sensitive -and- .conf ones need ToLower
			m_sEntryPath.ToLower();
	}

	//------------------------------------------------------------------------------------------------
	override void CreateWidget(notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_Layout, parent);
		if (!createdWidget)
			return;

		TextWidget descriptionWidget = TextWidget.Cast(createdWidget.FindAnyWidget("Description"));
		if (descriptionWidget)
			descriptionWidget.SetText(m_sDisplayName);

		TextWidget valueWidget = TextWidget.Cast(createdWidget.FindAnyWidget("Value"));
		if (valueWidget)
			SetConfigValue(valueWidget);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetConfigValue(notnull TextWidget valueWidget)
	{
		if (!m_Resource || !m_BaseContainer)
			InitContainerAndPaths();

		if (!m_Resource || !m_BaseContainer)
		{
			Print("Wrong config entry path | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			valueWidget.SetText("-wrong config entry path provided-");
			return;
		}

		int index = m_BaseContainer.GetVarIndex(m_sEntryPath);
		DataVarType dataVarType = m_BaseContainer.GetDataVarType(index);

		if (m_sValueFormat.Trim().IsEmpty())
			m_sValueFormat = "%1";

		switch (dataVarType)
		{
			case DataVarType.BOOLEAN:
				bool value;
				m_BaseContainer.Get(m_sEntryPath, value);
				valueWidget.SetTextFormat(m_sValueFormat, value);
				break;

			case DataVarType.INTEGER:
			case DataVarType.SCALAR:
				float value;
				m_BaseContainer.Get(m_sEntryPath, value);
				if (m_iDecimalMove)
					value /= Math.Pow(10, -m_iDecimalMove);

				valueWidget.SetTextFormat(m_sValueFormat, value.ToString(-1, m_iFixedDecimals));
				break;

			case DataVarType.STRING:
				string value;
				m_BaseContainer.Get(m_sEntryPath, value);
				valueWidget.SetTextFormat(m_sValueFormat, value);
				break;

			case DataVarType.VECTOR3:
				vector value;
				m_BaseContainer.Get(m_sEntryPath, value);
				valueWidget.SetTextFormat(m_sValueFormat, value);
				break;

			default:
				Print("Missing DataVarType type: " + dataVarType + " | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
				valueWidget.SetTextFormat(m_sValueFormat, "-");
				break;
		}

		if (m_sValueFormat == "%1")
			m_sValueFormat = string.Empty;
	}
}
