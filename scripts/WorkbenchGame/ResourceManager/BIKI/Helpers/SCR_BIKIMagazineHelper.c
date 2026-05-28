class SCR_BIKIMagazineHelper
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] magazineContainer
	//! \return
	static ResourceName GetFirstBullet(notnull BaseContainer magazineContainer)
	{
		IEntityComponentSource magazineComponentSource = SCR_BaseContainerTools.FindComponentSource(magazineContainer, MagazineComponent);
		if (!magazineComponentSource)
			return ResourceName.Empty;

		ResourceName config;
		if (!magazineComponentSource.Get("AmmoConfig", config) || !config)
			return ResourceName.Empty;

		Resource resource = Resource.Load(config);
		if (!resource.IsValid())
			return ResourceName.Empty;

		array<int> indices;
		if (!magazineComponentSource.Get("AmmoMapping", indices) || indices.IsEmpty())
			return ResourceName.Empty;

		BaseContainer configContainer = resource.GetResource().ToBaseContainer();
		array<ResourceName> ammoResourceNames;
		if (!configContainer.Get("AmmoResourceArray", ammoResourceNames))
			return ResourceName.Empty;

		int index = indices[0];
		if (!ammoResourceNames.IsIndexValid(index))
			return ResourceName.Empty;

		return ammoResourceNames[index];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] magazines
	//! \return
	static map<typename, ref array<ResourceName>> GetMagazineWells(notnull array<ResourceName> magazines)
	{
		if (magazines.IsEmpty())
			return new map<typename, ref array<ResourceName>>();

		map<typename, ref array<ResourceName>> result = new map<typename, ref array<ResourceName>>();

		Resource resource;
		BaseContainer baseContainer;
		IEntityComponentSource magazineComponentSource;
		foreach (ResourceName magazine : magazines)
		{
			resource = Resource.Load(magazine);
			if (!resource.IsValid())
				continue;

			//baseContainer = resource.GetResource().ToBaseContainer();
			magazineComponentSource = SCR_BaseContainerTools.FindComponentSource(resource, MagazineComponent);
			if (!magazineComponentSource)
				continue;

			baseContainer = magazineComponentSource.GetObject("MagazineWell");
			if (!baseContainer)
				continue;

			typename typeName = baseContainer.GetClassName().ToType();
			if (!typeName)
				continue;

			if (result.Contains(typeName))
				result.Get(typeName).Insert(magazine);
			else
				result.Insert(typeName, { magazine });
		}

		return result;
	}
}
