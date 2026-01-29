// Here is define which layout prefab belongs to given editable entity and how long it takes to build up the composition. 
// The Value is kind of abstract as it's not a cost of the composition or the time needed. 
// It depends on other setup like how much of this value is added by one usage of building tool or how many players at the same time are using their tool to build the composition.

[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleResourceName("", true)]
class SCR_CampaignBuildingCompositionOutlineManager
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Composition layout fits to a small (flat) slot size.", "et")]
	protected ResourceName m_sSlotFlatSmallLayout;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Composition layout fits to a medium (flat) slot size.", "et")]
	protected ResourceName m_sSlotFlatMediumLayout;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Composition layout fits to a large (flat) slot size.", "et")]
	protected ResourceName m_sSlotFlatLargeLayout;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Composition layout fits to a small (road) slot size.", "et")]
	protected ResourceName m_sSlotRoadSmallLayout;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Composition layout fits to a medium (road) slot size.", "et")]
	protected ResourceName m_sSlotRoadMediumLayout;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Composition layout fits to a large (road) slot size.", "et")]
	protected ResourceName m_sSlotRoadLargeLayout;
	
	[Attribute()]
	protected ref array<ref SCR_CampaignBuildingCompositionOutline> m_aCompositionLayouts;
	
	static const int DEFAULT_BUILDING_VALUE = 50;
	
	//------------------------------------------------------------------------------------------------
	//! Search all SCR_CampaignBuildingCompositionLayoutManager entry and try to find a matching composition. If none is found, the default one based on the slot size is returned.
	ResourceName GetCompositionOutline(notnull SCR_EditableEntityComponent entity)
	{
		IEntity composition = entity.GetOwner();
		if (!composition)	
			return ResourceName.Empty;
		
		ResourceName compositionResourceName = composition.GetPrefabData().GetPrefabName();
		if (compositionResourceName.IsEmpty())
			return ResourceName.Empty;
		
		foreach (SCR_CampaignBuildingCompositionOutline compositionLayout : m_aCompositionLayouts)
		{
			if (compositionLayout.GetEditableEntity() == compositionResourceName)
				return compositionLayout.GetCompositionLayout();
		}
		
		// If there wasn't found a specific layout for a given composition, load the one set for a specific slot.
		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(entity.GetInfo());
		if (!editableEntityUIInfo)
			return ResourceName.Empty;
		
		if (editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.SLOT_FLAT_SMALL))
			return m_sSlotFlatSmallLayout;
		else if (editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.SLOT_FLAT_MEDIUM))
			return m_sSlotFlatMediumLayout;
		else if (editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.SLOT_FLAT_LARGE))
			return m_sSlotFlatLargeLayout;
		else if (editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.SLOT_ROAD_SMALL))
			return m_sSlotRoadSmallLayout;
		else if (editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.SLOT_ROAD_MEDIUM))
			return m_sSlotRoadMediumLayout;
		else if (editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.SLOT_ROAD_LARGE))
			return m_sSlotRoadLargeLayout;
		// if the slot size wasn't set, use fail safe.
		return m_sSlotFlatSmallLayout;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Search all SCR_CampaignBuildingCompositionLayoutManager entry and try to find a building value for given composition.
	int GetCompositionBuildingValue(ResourceName originalComposition)
	{
		foreach (SCR_CampaignBuildingCompositionOutline compositionLayout : m_aCompositionLayouts)
		{
			if (compositionLayout.GetEditableEntity() == originalComposition)
				return compositionLayout.GetBuildingValue();
		}

		return DEFAULT_BUILDING_VALUE;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("", true)]
class SCR_CampaignBuildingCompositionOutline
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Editable entity prefab", "et")]
	protected ResourceName m_sEditableEntity;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Layout prefab", "et")]
	protected ResourceName m_sCompositionLayout;

	[Attribute("50", "Defines how long it takes to build up the composition.", "")]
	protected int m_iBuildingValue;

	//------------------------------------------------------------------------------------------------
	ResourceName GetEditableEntity()
	{
		return m_sEditableEntity;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetCompositionLayout()
	{
		return m_sCompositionLayout;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBuildingValue()
	{
		return m_iBuildingValue;
	}
}