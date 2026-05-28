//! Attribute for TranslationPluginConfigLocaleMatch
class TranslationPluginFields : BaseContainerCustomTitle
{
	protected string m_sFormat;
	protected ref array<string> m_aPropertyNames;

	protected static const int MAX_PROPERTIES_COUNT = 5;

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		if (!m_sFormat.Contains("%") || m_aPropertyNames.IsEmpty())
		{
			title = m_sFormat;
			return true;
		}

		foreach (int i, string propertyName : m_aPropertyNames)
		{
			if (i >= MAX_PROPERTIES_COUNT)
				break;

			if (!source.Get(propertyName, propertyName)) // variable reuse
				return false;

			m_aPropertyNames[i] = propertyName;
		}

		if (m_aPropertyNames.Count() != MAX_PROPERTIES_COUNT)
			m_aPropertyNames.Resize(MAX_PROPERTIES_COUNT);

		title = string.Format(m_sFormat, m_aPropertyNames[0], m_aPropertyNames[1], m_aPropertyNames[2], m_aPropertyNames[3], m_aPropertyNames[4]);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] format
	//! \param[in] propertyNames
	void TranslationPluginFields(string format, notnull array<string> propertyNames)
	{
		m_sFormat = format;
		m_aPropertyNames = propertyNames;
	}
}


[BaseContainerProps(configRoot: true)]
class TranslationPluginMatchConfig
{
	//
	//	Fields
	//

	[Attribute(defvalue: "Id", desc: "[Mandatory] Id string field name", category: "Fields")]
	string m_sIdField;

	[Attribute(desc: "Comment string field name", category: "Fields")]
	string m_sCommentField;

	[Attribute(desc: "Gender int field name (0 = none, 1 = male, 2 = female, 3 = other) see EBaseStringTableItemGender", category: "Fields")]
	string m_sGenderField;

	[Attribute(desc: "Max length int field name", category: "Fields")]
	string m_sMaxLengthField;

	[Attribute(desc: "Skip bool field name", category: "Fields")]
	string m_sSkipField;

	//
	//	Locales
	//

	[Attribute(defvalue: "Source", desc: "[Mandatory] Source field from which to translate (string table item full field name, prefix and suffix included if needed)", category: "Locales")]
	string m_sDefaultSource;

	[Attribute(desc: "Edited source, waiting for translation (string table item full field name, prefix and suffix included if needed)", category: "Locales")]
	string m_sDefaultSourceEdited;

	[Attribute(desc: "Field prefix, to remove from locale fields's name to obtain the locale (e.g Target_ for Target_en_us = en_us)", category: "Locales")]
	string m_sLocaleFieldsPrefix;

	[Attribute(desc: "Field suffix, to remove from locale fields's name to obtain the locale (e.g _final for en_us_final = en_us)", category: "Locales")]
	string m_sLocaleFieldsSuffix;

	[Attribute(desc: "File-Server locales matching - e.g \"english\" → \"en_gb\"", category: "Locales")]
	ref array<ref TranslationPluginConfigLocaleMatch> m_aLocalesDictionary;
}

[BaseContainerProps(), TranslationPluginFields("%1 → %2", { "m_sFileLocale", "m_sServerLocale" })]
class TranslationPluginConfigLocaleMatch
{
	[Attribute(defvalue: "english", desc: "Locale of the table - attribute name minus prefix/suffix (e.g Target_en_US_final - Target_ - _final = en_US)")]
	string m_sFileLocale;

	[Attribute(defvalue: "en_us", desc: "Locale expected by the server (format: en_gb, fr_ca, etc)")]
	string m_sServerLocale;
}
