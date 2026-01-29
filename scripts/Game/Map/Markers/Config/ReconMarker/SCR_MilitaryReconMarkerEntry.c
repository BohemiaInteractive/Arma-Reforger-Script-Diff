[BaseContainerProps()]
class SCR_MilitaryReconMarkerEntry : SCR_BaseReconMarkerEntry
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Military dimension defines", enumType: EMilitarySymbolDimension)]
	protected EMilitarySymbolDimension m_eDimension;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Military Type A can be set e.g. to Light and can be combined with Military Type B", enumType: EMilitarySymbolIcon)]
	protected EMilitarySymbolIcon m_eTypeA;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Military Type B can be set e.g. to Infantry and can be combined with Military Type A", enumType: EMilitarySymbolIcon)]
	protected EMilitarySymbolIcon m_eTypeB;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Predefined marker faction", enumType: EMilitarySymbolIdentity)]
	protected EMilitarySymbolIdentity m_eMilitarySymbolIdentity;

	[Attribute("1", desc: "Show timestamp when the marker was created")]
	protected bool m_bShowTimestamp;

	protected SCR_MapMarkerEntryMilitary m_MilitaryMarkerConfig;

	//------------------------------------------------------------------------------------------------
	protected SCR_MapMarkerEntryMilitary GetMilitaryMarkerConfig()
	{
		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (!markerConfig)
			return null;

		return SCR_MapMarkerEntryMilitary.Cast(markerConfig.GetMarkerEntryConfigByType(SCR_EMapMarkerType.PLACED_MILITARY));
	}

	//------------------------------------------------------------------------------------------------
	override void SetMenuEntry(notnull SCR_MapMarkerMenuEntry entry)
	{
		super.SetMenuEntry(entry);

		if (!m_MilitaryMarkerConfig)
			m_MilitaryMarkerConfig = GetMilitaryMarkerConfig();

		if (!m_MilitaryMarkerConfig)
			return;

		entry.SetMarkerType(SCR_EMapMarkerType.PLACED_MILITARY);
		entry.SetLayout();

		// set military symbol identity by setting from config
		int factionID = m_MilitaryMarkerConfig.GetFactionEntryID(m_eMilitarySymbolIdentity);
		if (factionID < 0)
		{
			PrintFormat("EMilitarySymbolIdentity:%1, set in SCR_MilitaryReconMarkerEntry is not supported.", m_eMilitarySymbolIdentity, level: LogLevel.WARNING);
			return;
		}

		SCR_MarkerMilitaryFactionEntry factionEntry = m_MilitaryMarkerConfig.GetFactionEntry(factionID);
		if (factionEntry)
			entry.SetSymbolProps(m_eMilitarySymbolIdentity, factionEntry.GetColor(), m_eDimension, m_eTypeA | m_eTypeB, EMilitarySymbolAmplifier.NONE);

		int dimensionID = m_MilitaryMarkerConfig.GetDimensionEntryID(m_eDimension);
		if (dimensionID < 0)
		{
			PrintFormat("EMilitarySymbolDimension:%1, set in SCR_MilitaryReconMarkerEntry is not supported.", m_eDimension, level: LogLevel.WARNING);
			return;
		}

		string entryName = string.Empty;
		SCR_MarkerMilitaryDimension dimensionData = m_MilitaryMarkerConfig.GetDimensionEntry(dimensionID);
		if (dimensionData)
			entryName = dimensionData.GetTranslation();

		array<ref SCR_MarkerMilitaryType> milTypes = m_MilitaryMarkerConfig.GetMilitaryTypes();
		if (!milTypes)
			return;

		// create entry name from dimension and military type
		foreach (SCR_MarkerMilitaryType type : milTypes)
		{
			if (!(m_eTypeA & type.GetType()) && !(m_eTypeB & type.GetType()))
				continue;

			if (!entryName.IsEmpty())
				entryName += " ";

			entryName += type.GetTranslation();
		}

		entry.SetName(entryName);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEntryPerformed(SCR_SelectionMenuEntry entry)
	{
		super.OnEntryPerformed(entry);

		if (!m_MilitaryMarkerConfig)
			m_MilitaryMarkerConfig = GetMilitaryMarkerConfig();

		if (!m_MilitaryMarkerConfig)
			return;

		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(SCR_EMapMarkerType.PLACED_MILITARY);
		marker.SetFlags(m_eTypeA | m_eTypeB);
		int dimensionID = m_MilitaryMarkerConfig.GetDimensionEntryID(m_eDimension);
		if (dimensionID < 0)
		{
			PrintFormat("EMilitarySymbolDimension:%1, set in SCR_MilitaryReconMarkerEntry is not supported.", m_eDimension, level: LogLevel.WARNING);
			return;
		}

		// set military symbol identity by setting in config
		EMilitarySymbolIdentity militarySymbolIdentity = m_eMilitarySymbolIdentity;
		int factionID = m_MilitaryMarkerConfig.GetFactionEntryID(militarySymbolIdentity);
		if (factionID < 0)
		{
			PrintFormat("EMilitarySymbolIdentity:%1, set in SCR_MilitaryReconMarkerEntry is not supported.", m_eMilitarySymbolIdentity, level: LogLevel.WARNING);
			return;
		}

		marker.SetMarkerConfigID(dimensionID * 100 + factionID); // combination of faction and dimension id

		float wX, wY;
		m_MapEntity.GetMapCenterWorldPosition(wX, wY);
		marker.SetWorldPos(wX, wY);

		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			Faction markerOwnerFaction = SCR_FactionManager.SGetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());
			if (markerOwnerFaction)
				marker.AddMarkerFactionFlags(factionManager.GetFactionIndex(markerOwnerFaction));
		}

		marker.SetTimestampVisibility(m_bShowTimestamp);

		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			marker.SetTimestamp(world.GetServerTimestamp());

		m_MarkerMgr.InsertStaticMarker(marker, false);
	}
}
