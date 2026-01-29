class SCR_RadioCoverageMapDescriptorComponentClass : SCR_MapDescriptorComponentClass
{
	[Attribute("{C35F29E48086221A}Configs/Campaign/CampaignGraphLinesConfig.conf", category: "Lines")]
	protected ref SCR_GraphLinesData m_GraphLinesData;

	//------------------------------------------------------------------------------------------------
	SCR_GraphLinesData GetGraphLinesData()
	{
		return m_GraphLinesData;
	}
}

class SCR_RadioCoverageMapDescriptorComponent : SCR_MapDescriptorComponent
{
	SCR_CoverageRadioComponent m_Radio;

	//------------------------------------------------------------------------------------------------
	protected SCR_GraphLinesData GetGraphLinesData()
	{
		IEntity owner = Item().Entity();

		if (!owner)
			return null;

		SCR_RadioCoverageMapDescriptorComponentClass componentData = SCR_RadioCoverageMapDescriptorComponentClass.Cast(GetComponentData(owner));

		if (!componentData)
			return null;

		return componentData.GetGraphLinesData();
	}

	//------------------------------------------------------------------------------------------------
	void SetParentRadio(SCR_CoverageRadioComponent radio)
	{
		m_Radio = radio;

		if (m_Radio)
		{
			SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
			SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
		}
		else
		{
			DeleteLinks();
			SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
			SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_CoverageRadioComponent GetParentRadio()
	{
		return m_Radio;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapOpen(MapConfiguration config)
	{
		CreateLinks();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapClose(MapConfiguration config)
	{
		DeleteLinks();
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateLinks()
	{
		MapItem mapItem = Item();

		if (!mapItem)
			return;

		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!playerFaction)
			return;

		string playerFactionEncryption = playerFaction.GetFactionRadioEncryptionKey();

		if (playerFactionEncryption.IsEmpty())
			return;

		string radioEncryption = m_Radio.GetEncryptionKey();
		SCR_ERadioCoverageStatus coverage =  m_Radio.GetCoverageByEncryption(radioEncryption);

		if (radioEncryption != playerFactionEncryption || coverage == SCR_ERadioCoverageStatus.NONE || coverage == SCR_ERadioCoverageStatus.SEND)
			return;

		array<SCR_CoverageRadioComponent> radios = {};
		m_Radio.GetRadiosInRange(radios);
		
		SCR_RadioCoverageMapDescriptorComponent target;
		MapItem targetItem;
		array<MapLink> targetLinks = {};
		MapLink link;

		foreach (SCR_CoverageRadioComponent radio : radios)
		{
			target = SCR_RadioCoverageMapDescriptorComponent.Cast(radio.GetOwner().FindComponent(SCR_RadioCoverageMapDescriptorComponent));

			if (!target)
				continue;

			targetItem = target.Item();

			if (!targetItem || !targetItem.IsVisible())
				continue;

			bool linked;
			targetLinks.Clear();
			targetItem.GetLinks(targetLinks);

			// Don't create two links between the same bases
			foreach (MapLink targetLink : targetLinks)
			{
				if (targetLink.Target() == mapItem)
				{
					linked = true;
					break;
				}
			}

			if (linked)
				continue;

			link = mapItem.LinkTo(targetItem);
			ColorMapLink(link, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteLinks()
	{
		MapItem mapItem = Item();

		if (!mapItem)
			return;

		mapItem.ClearLinks();
	}

	//------------------------------------------------------------------------------------------------
	void UpdateLinks()
	{
		DeleteLinks();
		CreateLinks();
	}

	//------------------------------------------------------------------------------------------------
	void OnIconHovered(bool hovering)
	{
		MapItem mapItem = Item();

		if (!mapItem)
			return;

		array<MapLink> links = {};
		mapItem.GetLinks(links);

		// Add links saved in neighboring components so we can potentially highlight them as well
		array<SCR_CoverageRadioComponent> radiosInRange = {};
		m_Radio.GetRadiosInRange(radiosInRange);
		
		SCR_RadioCoverageMapDescriptorComponent comp;
		MapItem otherMapItem;
		array<MapLink> otherLinks = {};

		foreach (SCR_CoverageRadioComponent radioInRange : radiosInRange)
		{
			comp = SCR_RadioCoverageMapDescriptorComponent.Cast(radioInRange.GetOwner().FindComponent(SCR_RadioCoverageMapDescriptorComponent));

			if (!comp)
				continue;

			otherMapItem = comp.Item();

			otherLinks.Clear();
			otherMapItem.GetLinks(otherLinks);

			foreach (MapLink otherLink : otherLinks)
			{
				if (otherLink.Target() == mapItem)
					links.Insert(otherLink);
			}
		}

		foreach (MapLink link : links)
		{
			if (!link)
				continue;

			ColorMapLink(link, hovering);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ColorMapLink(notnull MapLink link, bool isHovering)
	{
		MapLinkProps props = link.GetMapLinkProps();

		if (!props)
			return;

		SCR_GraphLinesData linesData = GetGraphLinesData();

		if (!linesData)
			return;

		props.SetLineWidth(linesData.GetLineWidth());

		Color c = Color.FromInt(props.GetLineColor().PackToInt());

		if (!c)
			return;

		if (isHovering)
			c.SetA(linesData.GetHighlightedAlpha());
		else
			c.SetA(linesData.GetDefaultAlpha());

		props.SetLineColor(c);
	}
}
