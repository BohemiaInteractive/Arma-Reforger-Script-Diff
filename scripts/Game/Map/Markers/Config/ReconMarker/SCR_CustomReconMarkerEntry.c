[BaseContainerProps(), SCR_CustomReconMarkerEntryTitle()]
class SCR_CustomReconMarkerEntry : SCR_BaseReconMarkerEntry
{
	[Attribute(desc: "Description in selection menu")]
	protected string m_sMenuDescription;

	[Attribute(desc: "Color name from Placed Marker Colors in MapMarkerConfig")]
	protected string m_sColorName;

	[Attribute(desc: "Imageset quad from Placed Marker Icons in MapMarkerConfig")]
	protected string m_sIconImagesetQuad;

	protected SCR_MapMarkerEntryPlaced m_PlacedMarkerConfig;

	//------------------------------------------------------------------------------------------------
	override void SetMenuEntry(notnull SCR_MapMarkerMenuEntry entry)
	{
		super.SetMenuEntry(entry);

		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (!markerConfig)
			return;

		m_PlacedMarkerConfig = SCR_MapMarkerEntryPlaced.Cast(markerConfig.GetMarkerEntryConfigByType(SCR_EMapMarkerType.PLACED_CUSTOM));
		if (!m_PlacedMarkerConfig)
			return;

		m_MenuEntry.SetMarkerType(SCR_EMapMarkerType.PLACED_CUSTOM);
		m_MenuEntry.SetName(m_sMenuDescription);

		int iconIndex = GetPlacedMarkerIconIndex();
		if (iconIndex < 0)
			return;

		string imageset, imagesetGlow, imageQuad;
		m_PlacedMarkerConfig.GetIconEntry(iconIndex, imageset, imagesetGlow, imageQuad);
		m_MenuEntry.SetIcon(imageset, imageQuad);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlacedMarkerIconIndex()
	{
		if (!m_PlacedMarkerConfig)
			return -1;

		array<ref SCR_MarkerIconEntry> iconsArr = m_PlacedMarkerConfig.GetIconEntries();
		if (!iconsArr)
			return -1;

		string imageset, imagesetGlow, imageQuad;
		foreach (int i, SCR_MarkerIconEntry iconEntry : iconsArr)
		{
			iconEntry.GetIconResource(imageset, imagesetGlow, imageQuad);
			if (imageQuad == m_sIconImagesetQuad)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override void OnEntryPerformed(SCR_SelectionMenuEntry entry)
	{
		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (!markerConfig)
			return;

		m_PlacedMarkerConfig = SCR_MapMarkerEntryPlaced.Cast(markerConfig.GetMarkerEntryConfigByType(SCR_EMapMarkerType.PLACED_CUSTOM));
		if (!m_PlacedMarkerConfig)
			return;

		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);

		int iconIndex = GetPlacedMarkerIconIndex();
		if (iconIndex != -1)
			marker.SetIconEntry(iconIndex);

		int colorIndex;
		foreach (int i, SCR_MarkerColorEntry colorEntry : m_PlacedMarkerConfig.GetColorEntries())
		{
			if (colorEntry.GetName() == m_sColorName)
			{
				colorIndex = i;
				break;
			}
		}

		marker.SetColorEntry(colorIndex);
		marker.SetCustomText(m_sMenuDescription);

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

		m_MarkerMgr.InsertStaticMarker(marker, false);
	}
}

class SCR_CustomReconMarkerEntryTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		string quad;
		source.Get("m_sIconImagesetQuad", quad);
		title = source.GetClassName() + ": "+ quad;

		return true;
	}
}
