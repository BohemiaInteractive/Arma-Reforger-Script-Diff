class SCR_ComponentHelper
{	
	//------------------------------------------------------------------------------------------------
	//! Get component source from entity source.
	//! \param[in] entitySource Entity source - cannot be null
	//! \return Component source
	static IEntityComponentSource GetWeaponComponentSource(IEntitySource entitySource)
	{
		if (!entitySource)
			return null;
		
		int componentsCount = entitySource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(WeaponComponent))
				return componentSource;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get UI info from WeaponComponent source
	//! \param[in] componentSource Component source - cannot be null
	//! \return Weapon UI info
	static WeaponUIInfo GetWeaponComponentInfo(IEntityComponentSource componentSource)
	{		
		BaseContainer infoSource = componentSource.GetObject("UIInfo");
		if (!infoSource)
			return null;
		
		WeaponUIInfo info = WeaponUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(infoSource));
		return info;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get component source from entity source.
	//! \param[in] entitySource Entity source - cannot be null
	//! \return Component source
	static IEntityComponentSource GetInventoryItemComponentSource(IEntitySource entitySource)
	{
		if (!entitySource)
			return null;
		
		int componentsCount = entitySource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(InventoryItemComponent))
				return componentSource;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get UI info from InventoryItemComponent source
	//! \param[in] componentSource Component source - cannot be null
	//! \return Item Attribute Collection
	static SCR_ItemAttributeCollection GetInventoryItemInfo(IEntityComponentSource componentSource)
	{		
		BaseContainer infoSource = componentSource.GetObject("Attributes");
		if (!infoSource)
			return null;
		
		SCR_ItemAttributeCollection attributeCollection = SCR_ItemAttributeCollection.Cast(BaseContainerTools.CreateInstanceFromContainer(infoSource));
		return attributeCollection;
	}
};
