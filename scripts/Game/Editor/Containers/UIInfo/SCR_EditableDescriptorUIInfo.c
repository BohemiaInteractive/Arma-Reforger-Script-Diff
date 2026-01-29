[BaseContainerProps()]
class SCR_EditableDescriptorUIInfo: SCR_EditableEntityUIInfo
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sNameFormat;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sDescriptionFormat;
	
	protected LocalizedString m_sLocationName;
	protected string m_sNameEncoded;
	protected string m_sDescriptionEncoded;

	//------------------------------------------------------------------------------------------------
	//! Get location name.
	//! \return Location name
	LocalizedString GetLocationName()
	{
		return m_sLocationName;
	}

	//------------------------------------------------------------------------------------------------
	//! Set location name from existing name.
	//! \param locationName Name of the location
	void SetLocationName(LocalizedString locationName)
	{
		m_sLocationName = locationName;
		//EncodeName();
	}

	//------------------------------------------------------------------------------------------------
	//! Set location name from a position.
	//! \param pos Position
	void SetLocationName(vector pos)
	{
		m_sLocationName = SCR_MapEntity.GetGridLabel(pos);
		//EncodeName();
	}

	/*
	//------------------------------------------------------------------------------------------------
	protected void EncodeName()
	{
		m_sNameEncoded = SCR_LocalizationTools.EncodeFormat(m_sNameFormat, m_sLocationName);
		m_sDescriptionEncoded = SCR_LocalizationTools.EncodeFormat(m_sDescriptionFormat, m_sLocationName);
	}

	//------------------------------------------------------------------------------------------------
	override LocalizedString GetName()
	{
		if (m_sLocationName)
			return m_sNameEncoded;
		else
			return super.GetName();
	}

	//------------------------------------------------------------------------------------------------
	override LocalizedString GetDescription()
	{
		if (m_sLocationName)
			return m_sDescriptionEncoded;
		else
			return super.GetDescription();
	}
	*/

	//------------------------------------------------------------------------------------------------
	override bool SetNameTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		if (m_sLocationName.IsEmpty())
		{
			return super.SetNameTo(textWidget);
		}
		else
		{
			textWidget.SetTextFormat(m_sNameFormat, m_sLocationName);
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool SetDescriptionTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		if (m_sLocationName.IsEmpty())
		{
			return super.SetDescriptionTo(textWidget);
		}
		else
		{
			textWidget.SetTextFormat(m_sDescriptionFormat, m_sLocationName);
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	// override without 'protected' keyword
	override void CopyFrom(SCR_UIName source)
	{
		SCR_EditableDescriptorUIInfo descriptorSource = SCR_EditableDescriptorUIInfo.Cast(source);
		if (descriptorSource)
		{
			m_sNameFormat = descriptorSource.m_sNameFormat;
			m_sDescriptionFormat = descriptorSource.m_sDescriptionFormat;
		}
		else
		{
			m_sNameFormat = source.Name;
			
			SCR_UIDescription descriptionSource = SCR_UIDescription.Cast(source);
			if (descriptionSource)
				m_sDescriptionFormat = descriptionSource.Description;
		}
		
		super.CopyFrom(source);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_EditableDescriptorUIInfo()
	{
		if (!m_sNameFormat)
			m_sNameFormat = Name;
		
		if (!m_sDescriptionFormat)
			m_sDescriptionFormat = Description;
	}
}
