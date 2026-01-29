[BaseContainerProps()]
class SCR_TaskUIInfo : SCR_UIInfo
{
	[Attribute(desc: "Parameters for localized name. Up to 9 parameters can be passed")]
	protected ref array<LocalizedString> m_aNameParameters;

	[Attribute(desc: "Parameters for localized description. Up to 9 parameters can be passed")]
	protected ref array<LocalizedString> m_aDescriptionParameters;

	protected const int MAX_PARAM_COUNT = 9;

	protected int m_iDefaultFlags = 0;
	protected const int FLAG_CUSTOM_ICON 			= 1 << 0;
	protected const int FLAG_CUSTOM_NAME 			= 1 << 1;
	protected const int FLAG_CUSTOM_DESCRIPTION		= 1 << 2;

	//------------------------------------------------------------------------------------------------
	//! Sets icon path to given path
	void SetIconPath(ResourceName iconPath)
	{
		Icon = iconPath;
		m_iDefaultFlags |= FLAG_CUSTOM_ICON;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets icon path to given path
	void SetIconSetName(string iconSetName)
	{
		IconSetName = iconSetName;
		m_iDefaultFlags |= FLAG_CUSTOM_ICON;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns unformatted name and parameters
	//! \param[out] params
	//! \return
	LocalizedString GetUnformattedName(out array<LocalizedString> params = null)
	{
		if (m_aNameParameters && !m_aNameParameters.IsEmpty())
		{
			params = {};
			params.Copy(m_aNameParameters);
		}

		return Name;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets unformatted name with parameters
	void SetUnformattedName(LocalizedString name, array<LocalizedString> nameParams = null)
	{
		Name = name;
		m_iDefaultFlags |= FLAG_CUSTOM_NAME;

		if (!nameParams)
			return;

		m_aNameParameters = {};
		m_aNameParameters.Copy(nameParams);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns unformatted description and parameters
	//! \param[out] params
	//! \return
	LocalizedString GetUnformattedDescription(out array<LocalizedString> params = null)
	{
		if (m_aDescriptionParameters && !m_aDescriptionParameters.IsEmpty())
		{
			params = {};
			params.Copy(m_aDescriptionParameters);
		}

		return Description;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets unformatted description with parameters
	void SetUnformattedDescription(LocalizedString description, array<LocalizedString> descriptionParams = null)
	{
		Description = description;
		m_iDefaultFlags |= FLAG_CUSTOM_DESCRIPTION;

		if (!descriptionParams)
			return;

		m_aDescriptionParameters = {};
		m_aDescriptionParameters.Copy(descriptionParams);
	}

	//------------------------------------------------------------------------------------------------
	override bool SetNameTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;

		if (m_aNameParameters && !m_aNameParameters.IsEmpty())
			textWidget.SetTextFormat(Name, NameParam(0), NameParam(1), NameParam(2), NameParam(3), NameParam(4), NameParam(5), NameParam(6), NameParam(7), NameParam(8));
		else
			textWidget.SetText(Name);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SetDescriptionTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;

		if (m_aDescriptionParameters && !m_aDescriptionParameters.IsEmpty())
			textWidget.SetTextFormat(Description, DescriptionParam(0), DescriptionParam(1), DescriptionParam(2), DescriptionParam(3), DescriptionParam(4), DescriptionParam(5), DescriptionParam(6), DescriptionParam(7), DescriptionParam(8));
		else
			textWidget.SetText(Description);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	string GetTranslatedName()
	{
		if (m_aNameParameters && !m_aNameParameters.IsEmpty())
			return WidgetManager.Translate(Name, NameParam(0), NameParam(1), NameParam(2), NameParam(3), NameParam(4), NameParam(5), NameParam(6), NameParam(7), NameParam(8));
		else
			return WidgetManager.Translate(Name);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param index
	//! \return
	protected LocalizedString NameParam(int index)
	{
		if (index < m_aNameParameters.Count())
			return m_aNameParameters[index];
		else
			return LocalizedString.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param index
	//! \return
	protected LocalizedString DescriptionParam(int index)
	{
		if (index < m_aDescriptionParameters.Count())
			return m_aDescriptionParameters[index];
		else
			return LocalizedString.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Create SCR_TaskUIInfo from basic params.
	//! \param name
	//! \param description
	//! \param icon
	//! \param iconSetName
	//! \return SCR_TaskUIInfo class
	static SCR_TaskUIInfo CreateInfo(
		LocalizedString name,
		array<LocalizedString> nameParams,
		LocalizedString description,
		array<LocalizedString> descParams,
		ResourceName icon = ResourceName.Empty,
		string iconSetName = string.Empty)
	{
		SCR_TaskUIInfo info = new SCR_TaskUIInfo();
		info.Name = name;
		info.Description = description;
		info.Icon = icon;
		info.IconSetName = iconSetName;

		if (nameParams && !nameParams.IsEmpty())
		{
			info.m_aNameParameters = {};
			info.m_aNameParameters.Copy(nameParams);
		}

		if (descParams && !descParams.IsEmpty())
		{
			info.m_aDescriptionParameters = {};
			info.m_aDescriptionParameters.Copy(descParams);
		}

		return info;
	}

	//------------------------------------------------------------------------------------------------
	//--- Protected, to be overridden and/or made public by inherited classes
	override void CopyFrom(SCR_UIName source)
	{
		SCR_TaskUIInfo sourceTaskUIInfo = SCR_TaskUIInfo.Cast(source);
		if (sourceTaskUIInfo)
		{
			m_iDefaultFlags = sourceTaskUIInfo.m_iDefaultFlags;

			if (sourceTaskUIInfo.m_aNameParameters && !sourceTaskUIInfo.m_aNameParameters.IsEmpty())
			{
				m_aNameParameters = {};
				m_aNameParameters.Copy(sourceTaskUIInfo.m_aNameParameters);
			}

			if (sourceTaskUIInfo.m_aDescriptionParameters && !sourceTaskUIInfo.m_aDescriptionParameters.IsEmpty())
			{
				m_aDescriptionParameters = {};
				m_aDescriptionParameters.Copy(sourceTaskUIInfo.m_aDescriptionParameters);
			}
		}

		super.CopyFrom(source);
	}

	//------------------------------------------------------------------------------------------------
	bool HasCustomIcon()
	{
		return m_iDefaultFlags & FLAG_CUSTOM_ICON;
	}

	//------------------------------------------------------------------------------------------------
	bool HasCustomName()
	{
		return m_iDefaultFlags & FLAG_CUSTOM_NAME;
	}

	//------------------------------------------------------------------------------------------------
	bool HasCustomDescription()
	{
		return m_iDefaultFlags & FLAG_CUSTOM_DESCRIPTION;
	}
}
