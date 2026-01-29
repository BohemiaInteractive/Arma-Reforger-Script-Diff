//! Marker recon symbol entry - combination of military and custom symbols
[BaseContainerProps(), SCR_MapMarkerTitle()]
class SCR_MapMarkerEntryRecon : SCR_MapMarkerEntryConfig
{
	[Attribute("AR-MapMarker_Military", desc: "Description in selection menu")]
	protected string m_sMenuDescription;

	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Imageset resource", params: "imageset")]
	protected ResourceName m_sMenuImageset;

	[Attribute("unknown_faction", desc: "Imageset icon")]
	protected string m_sMenuIcon;

	[Attribute(desc:"Faction recon marker entries, for setting the recon markers for each faction individually")]
	protected ref array<ref SCR_FactionReconMarkerEntry> m_aFactionReconMarkerEntries;

	//------------------------------------------------------------------------------------------------
	override SCR_EMapMarkerType GetMarkerType()
	{
		return SCR_EMapMarkerType.PLACED_RECON;
	}

	//------------------------------------------------------------------------------------------------
	//! \return menu description
	string GetMenuDescription()
	{
		return m_sMenuDescription;
	}

	//------------------------------------------------------------------------------------------------
	//! \return menu imageset
	ResourceName GetMenuImageset()
	{
		return m_sMenuImageset;
	}

	//------------------------------------------------------------------------------------------------
	//! \return menu icon
	string GetMenuIcon()
	{
		return m_sMenuIcon;
	}

	//------------------------------------------------------------------------------------------------
	//! Create entries from config
	//! \param[in] radialUI
	//! \param[in] parent
	void CreateEntries(notnull SCR_MapRadialUI radialUI, notnull SCR_SelectionMenuCategoryEntry parent = null)
	{
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!faction)
			return;

		SCR_MapMarkerMenuEntry menuEntry;
		array<ref SCR_BaseReconMarkerEntry> reconMarkerEntries;
		foreach (SCR_FactionReconMarkerEntry factionReconMarkerEntry : m_aFactionReconMarkerEntries)
		{
			// if faction is not set in entry, markers are used for all factions
			if (!factionReconMarkerEntry.GetFactionKey().IsEmpty() && factionReconMarkerEntry.GetFactionKey() != faction.GetFactionKey())
				continue;

			reconMarkerEntries = factionReconMarkerEntry.GetReconMarkerEntries();
			if (!reconMarkerEntries)
				continue;

			foreach (SCR_BaseReconMarkerEntry entry : reconMarkerEntries)
			{
				if (!entry || !entry.IsEnabled())
					continue;

				menuEntry = new SCR_MapMarkerMenuEntry();
				menuEntry.GetOnPerform().Insert(OnEntryPerformed);

				radialUI.InsertCustomRadialEntry(menuEntry, parent);
				entry.SetMenuEntry(menuEntry);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntryPerformed(SCR_SelectionMenuEntry perfomedEntry)
	{
		array<ref SCR_BaseReconMarkerEntry> reconMarkerEntries;
		foreach (SCR_FactionReconMarkerEntry factionReconMarkerEntry : m_aFactionReconMarkerEntries)
		{
			reconMarkerEntries = factionReconMarkerEntry.GetReconMarkerEntries();
			if (!reconMarkerEntries)
				continue;

			foreach (SCR_BaseReconMarkerEntry entry : reconMarkerEntries)
			{
				if (entry.GetMenuEntry() == perfomedEntry)
				{
					entry.OnEntryPerformed(perfomedEntry);
					break;
				}
			}
		}
	}
}
