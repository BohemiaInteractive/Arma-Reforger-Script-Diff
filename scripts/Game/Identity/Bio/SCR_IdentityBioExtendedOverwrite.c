[BaseContainerProps(), BaseContainerCustomDoubleCheckTitleField("m_bIsUnique", "m_sBio", "1", "UNIQUE - %1", "%1")]
class SCR_IdentityBioExtendedOverwrite: SCR_IdentityBio
{
	[Attribute(desc: "Any Extended Identity that this bio will overwrite. Leaving any varriables empty or -1 will make sure the specific extended identity var is not overwritten")]
	protected ref SCR_ExtendedIdentity m_OverwriteExtendedIdentity;
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns The overwrite identity
	SCR_ExtendedIdentity GetOverwriteExtendedIdentity()
	{
		return m_OverwriteExtendedIdentity;
	}
}