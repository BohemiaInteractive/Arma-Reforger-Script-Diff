[BaseContainerProps(), BaseContainerCustomDoubleCheckTitleField("m_bIsUnique", "m_sBio", "1", "UNIQUE - %1", "%1")]
class SCR_IdentityBioCharacterExtendedOverwrite: SCR_IdentityBioExtendedOverwrite
{
	//[Attribute(desc: "Overwrite Identity, leave null to not overwrite. Should only overwrite variables that are set and ignore those that are not set")]
	//protected ref Identity m_OverwriteIdentity;
	
	[Attribute(desc: "Name to overwrite, leave empty to auto assign" ,uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sOverwriteName;
	
	[Attribute(desc: "Alias to overwrite, leave empty to auto assign" ,uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sOverwriteAlias;
	
	[Attribute(desc: "Surname to overwrite, leave empty to auto assign" ,uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sOverwriteSurname;
	
	//~ TODO: Overwrite identity does not work with Identity Class and Identity class is fully code so no access to it
	/*
	Get overwrite identity. Either replaces full identity or replaces names only if Visual and Sound identity are not set
	\return Returns the Identity
	*/
	/*Identity GetOverwriteIdentity()
	{
		return m_OverwriteIdentity;
	}*/
	
	//------------------------------------------------------------------------------------------------
	/*
	//! Get overwrite names
	//! para[out] name Name to overwrite (Empty to ignore)
	//! param[out] alias Alias to overwrite (Empty to ignore)
	//! param[out] surname Surname to overwrite (Empty to ignore)
	//! \return Returns false if no names are set to overwrite
	*/
	bool GetOverwriteNames(out string name, out string alias, out string surname)
	{
		if (m_sOverwriteName.IsEmpty() && m_sOverwriteAlias.IsEmpty() && m_sOverwriteSurname.IsEmpty())
			return false;
		
		name = m_sOverwriteName;
		alias = m_sOverwriteAlias;
		surname = m_sOverwriteSurname;
		
		return true;
	}
}