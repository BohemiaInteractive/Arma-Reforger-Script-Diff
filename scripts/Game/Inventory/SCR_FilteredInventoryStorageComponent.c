[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_FilteredInventoryStorageComponentClass : SCR_UniversalInventoryStorageComponentClass
{
	[Attribute(uiwidget: UIWidgets.ComboBox, desc: "List of common item types which can be stored in this container", enums: ParamEnumArray.FromEnum(ECommonItemType))]
	protected ref array<string> m_aAllowedItemTypes;

	protected ref array<ECommonItemType> m_aAllowedItemTypesEnum;

	//------------------------------------------------------------------------------------------------
	//! Initalization of shared part of the memory for this prefab
	void Init()
	{
		if (!m_aAllowedItemTypes || m_aAllowedItemTypes.IsEmpty())
			return;

		if (m_aAllowedItemTypesEnum && m_aAllowedItemTypesEnum.Count() == m_aAllowedItemTypes.IsEmpty())
			return;

		m_aAllowedItemTypesEnum = {};
		foreach (string enumName : m_aAllowedItemTypes)
		{
			m_aAllowedItemTypesEnum.Insert(typename.StringToEnum(ECommonItemType, enumName));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ECommonItemType> GetAllowedItemTypes()
	{
		return m_aAllowedItemTypesEnum;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNumberOfAllowedTypes()
	{
		if (!m_aAllowedItemTypesEnum)
			return 0;

		return m_aAllowedItemTypesEnum.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	//! \return
	ECommonItemType GetAllowedItemType(int id)
	{
		if (!m_aAllowedItemTypesEnum || !m_aAllowedItemTypesEnum.IsIndexValid(id))
			return 0;

		return m_aAllowedItemTypesEnum[id];
	}
}

class SCR_FilteredInventoryStorageComponent : SCR_UniversalInventoryStorageComponent
{
	protected const string VAR_NAME_ATTRIBUTE = "Attributes";
	protected const string VAR_NAME_COMMON_TYPE = "CommonItemType";

	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		SCR_FilteredInventoryStorageComponentClass data = SCR_FilteredInventoryStorageComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data || data.GetNumberOfAllowedTypes() < 1)
			return true;//no filter so everything is allowed

		if (!item)
			return false;

		InventoryItemComponent inventoryItemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItemComp)
			return false;

		ItemAttributeCollection attributes = inventoryItemComp.GetAttributes();
		if (!attributes)
			return false;

		foreach (ECommonItemType allowedType : data.GetAllowedItemTypes())
		{
			if (attributes.GetCommonType() == allowedType)
				return super.CanStoreItem(item, slotID);
		}

		return false;
	}


	//------------------------------------------------------------------------------------------------
	override bool CanStoreResource(ResourceName resourceName, int slotID)
	{
		SCR_FilteredInventoryStorageComponentClass data = SCR_FilteredInventoryStorageComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data || data.GetNumberOfAllowedTypes() < 1)
			return true;//no filter so everything is allowed

		Resource entityResource = Resource.Load(resourceName);
		if (!entityResource.IsValid())
			return false;

		BaseResourceObject resourceObj = entityResource.GetResource();
		if (!resourceObj)
			return false;

		IEntitySource entitySrc = resourceObj.ToEntitySource();
		if (!entitySrc)
			return false;

		IEntityComponentSource componentSrc;
		BaseContainer attributes;
		string commonTypeName;
		ECommonItemType commonType;
		for (int i, componentsCount = entitySrc.GetComponentCount(); i < componentsCount; i++)
		{
			componentSrc = entitySrc.GetComponent(i);
			if (!componentSrc)
				continue;

			if (!componentSrc.GetClassName().ToType().IsInherited(InventoryItemComponent))
				continue;

			attributes = componentSrc.GetObject(VAR_NAME_ATTRIBUTE);
			if (!attributes)
				return false;

			attributes.Get(VAR_NAME_COMMON_TYPE, commonTypeName);
			if (commonTypeName.IsEmpty())
				return false;

			commonType = typename.StringToEnum(ECommonItemType, commonTypeName);
			foreach (ECommonItemType allowedType : data.GetAllowedItemTypes())
			{
				if (commonType == allowedType)
					return super.CanStoreResource(resourceName, slotID);
			}

			return false;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_FilteredInventoryStorageComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SCR_FilteredInventoryStorageComponentClass data = SCR_FilteredInventoryStorageComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return;

		data.Init();
	}
}
