[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_Label")]
class EditablePrefabsLabel_Faction : EditablePrefabsLabel_Base
{
	[Attribute()]
	protected FactionKey m_FactionToCheck;

	//------------------------------------------------------------------------------------------------
	override bool GetLabelValid(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource, string targetPath, EEditableEntityType entityType, notnull array<EEditableEntityLabel> authoredLabels, out EEditableEntityLabel label)
	{
		IEntityComponentSource factionComponentSource = SCR_BaseContainerTools.FindComponentSource(entitySource, FactionAffiliationComponent);
		IEntityComponentSource factionControlComponentSource = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_FactionAffiliationComponent);
		FactionKey factionKey = string.Empty;
		if (factionComponentSource)
			factionComponentSource.Get("faction affiliation", factionKey);
		else if (factionControlComponentSource)
			factionControlComponentSource.Get("m_DefaultFaction", factionKey);

		// Check if faction was set through authored labels, override
		bool factionLabelAuthored = authoredLabels.Contains(m_Label);
		if (factionLabelAuthored)
			factionKey = m_FactionToCheck;

		if (factionKey != string.Empty && factionKey == m_FactionToCheck)
		{
			// Set faction on Editable Entity UIInfo, used for a lot of... ui info
			array<ref ContainerIdPathEntry> UIInfoPath = { ContainerIdPathEntry(componentSource.GetClassName()), ContainerIdPathEntry("m_UIInfo") };
			api.SetVariableValue(entitySource, UIInfoPath, "m_sFaction", m_FactionToCheck);
			label = m_Label;

			// Only add the auto label if faction label was not authored
			return !factionLabelAuthored;
		}

		return false;
	}
}
