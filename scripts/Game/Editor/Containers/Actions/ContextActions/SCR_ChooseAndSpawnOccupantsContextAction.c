[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_ChooseAndSpawnOccupantsContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute(desc: "Compartments Types to fill when chosing entities.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECompartmentType))]
	protected ref array<ECompartmentType> m_aCompartmentsTypes;

	[Attribute(desc: "Opens content browser and shows single characters only")]
	protected ref SCR_EditorContentBrowserDisplayConfig m_SingleCharacterDisplayConfig;
	
	[Attribute(desc: "Opens content browser and shows single characters and small groups")]
	protected ref SCR_EditorContentBrowserDisplayConfig m_SmallGroupsDisplayConfig;
	
	[Attribute(desc: "Opens content browser and shows single characters, small groups and medium groups")]
	protected ref SCR_EditorContentBrowserDisplayConfig m_MediumGroupsDisplayConfig;
	
	[Attribute(desc: "Opens content browser and shows single characters, small groups medium groups and large groups")]
	protected ref SCR_EditorContentBrowserDisplayConfig m_LargeGroupsDisplayConfig;
	
	[Attribute("5", desc: "This free compartment amount or higher need to be availible or more in order for 'large' filter label to be set in the content browser. Allowing those groups sizes (and lower) to be placed", params: "1 inf 1")]
	protected int m_LargeGroupSize;
	
	[Attribute("3", desc: "This free compartment amount or higher need to be availible or more in order for 'medium' filter label to be set in the content browser. Allowing those groups sizes (and lower) to be placed", params: "1 inf 1")]
	protected int m_MediumGroupSize;
	
	[Attribute("2", desc: "This free compartment amount or higher need to be availible or more in order for 'small' filter label to be set in the content browser. Allowing those groups sizes (and lower) to be placed. (Anything lower will be a single character)", params: "1 inf 1")]
	protected int m_SmallGroupSize;
	
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{		
		if (!hoveredEntity)
			return false;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(hoveredEntity.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return false;
		
		SCR_ContentBrowserEditorComponent contentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		return contentBrowserManager != null;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(hoveredEntity.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		
		return compartmentManager && compartmentManager.CanOccupy(m_aCompartmentsTypes, false, string.Empty, false, true);
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(hoveredEntity.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		
		array<BaseCompartmentSlot> freeCompartments = {};
		
		foreach(ECompartmentType compartmentType: m_aCompartmentsTypes)
		{
			compartmentManager.GetFreeCompartmentsOfType(freeCompartments, compartmentType);
		}
		
		if (freeCompartments.IsEmpty())
			return;

		int count = freeCompartments.Count();

		SCR_ContentBrowserEditorComponent contentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
	
		if (count >= m_LargeGroupSize)
			contentBrowserManager.OpenBrowserExtended(hoveredEntity, m_LargeGroupsDisplayConfig);
		else if (count >= m_MediumGroupSize)
			contentBrowserManager.OpenBrowserExtended(hoveredEntity, m_MediumGroupsDisplayConfig);
		else if (count >= m_SmallGroupSize)
			contentBrowserManager.OpenBrowserExtended(hoveredEntity, m_SmallGroupsDisplayConfig);
		else 
			contentBrowserManager.OpenBrowserExtended(hoveredEntity, m_SingleCharacterDisplayConfig);
	}
};