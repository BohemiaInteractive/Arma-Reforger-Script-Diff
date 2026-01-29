#ifdef WORKBENCH	// this config is only related to a Workbench plugin (SCR_BasicCodeFormatterPlugin)
[BaseContainerProps(configRoot: true)]
class SCR_BasicCodeFormatterSpellCheckConfig
{
	[Attribute(desc: "Commonly misspelt words and their potential fix", category: "SpellCheck and Forbidden Words")]
	ref array<ref SCR_BasicCodeFormatterSpellCheckConfig_ForbiddenWordEntry> m_aEntries;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleFields({ "m_sMistake", "m_sCorrection" }, "%1 → %2")]
class SCR_BasicCodeFormatterSpellCheckConfig_ForbiddenWordEntry
{
	[Attribute(defvalue: "1", desc: "Whether this check should be considered or not")]
	bool m_bEnabled;

	[Attribute(defvalue: "misspelled", desc: "Word that should not be present; case-insensitive, only alphanumerical characters are processed\n- word is exact word search\n- word* is begins with search\n- *word* is middle of word search\n- *word is ends with search")]
	string m_sMistake;

	[Attribute(defvalue: "misspelt", desc: "Word(s) that should replace the mistake; can be left empty")]
	string m_sCorrection;

	[Attribute(defvalue: "", desc: "Explanation, if any")]
	string m_sComment;
}
#endif
