//! Localisation rule controlled by LocParserManager.
//! When parsing files, each found string is evaluated towards all available rules.
[BaseContainerProps()]
class LocParserRule
{
	[Attribute(desc: "Newly created string table item will be created based on this template.\nSome variables will be overridden to reflect contextual data, e.g., ID or author.")]
	protected ref CustomStringTableItem m_ItemTemplate;

	protected LocParserManager m_Config;

	//------------------------------------------------------------------------------------------------
	//! Get intended ID of the string.
	//! To be overloaded by inherited classes.
	//! \param[in] fileName Name of the file (without path or extension) in which the string is
	//! \param[in] varName Name of the variable which holds the string
	//! \param[in] objects Hierarchy of script classes leading to the string, starting from the bottom-most one
	//! \param[out] index Item index (when in array)
	//! \return Localization ID
	string GetID(string fileName, string varName, array<BaseContainer> objects, array<int> indexes)
	{
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the string can be localized according to this rule.
	//! To be overloaded by inherited classes.
	//! \param[in] fileName Name of the file (without path or extension) in which the string is
	//! \param[in] varName Name of the variable which holds the string
	//! \param[in] objects Hierarchy of script classes leading to the string, starting from the bottom-most one
	//! \return True if the string can be localized
	bool Evaluate(string fileName, string varName, array<BaseContainer> objects)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the string can be localized according to this rule.
	//! To be overloaded by inherited classes.
	//! \param[in] fileName Name of the file (without path or extension) in which the string is
	//! \param[in] varName Name of the variable which holds the string
	//! \param[in] objects Hierarchy of script classes leading to the string, starting from the bottom-most one
	//! \param[in] id Intended localization ID
	//! \param[in] text Content of the string
	//! \return Newly added container (can be null if no new container was added, e.g., when only logging or when it already existed in the database)
	BaseContainer Localize(string fileName, string varName, array<BaseContainer> objects, string id, string text)
	{
		return m_Config.UpdateItem(id, text);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] config
	void InitRule(notnull LocParserManager config)
	{
		m_Config = config;
	}
}
