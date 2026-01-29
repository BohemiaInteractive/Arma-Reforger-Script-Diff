class SCR_PersistentResource
{
	EResourceType	m_eResourceType;
	float			m_fValue;
}

class SCR_ResourceComponentSerializer : ScriptedComponentSerializer
{
	[Attribute(desc: "Filter for which resource types to save - or all if empty.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EResourceType))]
	protected ref array<EResourceType> m_aResourceTypeFilter;

	protected bool m_bUseTypeDescriminator = false;

	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_ResourceComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(component);

		const BaseContainer source = component.GetComponentSource(owner);
		auto containerSources = source.GetObjectArray("m_aContainers");
		auto containers = resource.GetContainers();
		if (!containers)
			return ESerializeResult.DEFAULT;

		if (containerSources.Count() != containers.Count())
			return ESerializeResult.ERROR;

		array<ref SCR_PersistentResource> resources();
		foreach (int idx, auto container : containers)
		{
			if (container.IsEncapsulated())
				continue;

			const EResourceType resourceType = container.GetResourceType();
			if (!m_aResourceTypeFilter.IsEmpty() && !m_aResourceTypeFilter.Contains(resourceType))
				continue;

			const float value = container.GetResourceValue();

			float defaultValue;
			const SCR_ResourceEncapsulator encapsulator = resource.GetEncapsulator(resourceType);
			if (encapsulator)
			{
				bool overrideByAction = false;
				auto actions = encapsulator.GetActions();
				foreach (auto action : actions)
				{
					auto changeDefaultAction = SCR_ResourceEncapsulatorActionChangeResourceValue.Cast(action);
					if (!changeDefaultAction)
						continue;

					overrideByAction = true;
					defaultValue = changeDefaultAction.GetValueCurrent();
					break;
				}
				if (!overrideByAction)
				{
					const SCR_ResourceContainerQueueBase queue = encapsulator.GetContainerQueue();
					for (int i = 0, count = queue.GetContainerCount(); i < count; ++i)
					{
						const SCR_ResourceContainer childContainer = queue.GetContainerAt(i);
						const SCR_ResourceComponent childComponent = childContainer.GetComponent();
						const int containerIdx = childComponent.GetContainers().Find(childContainer);
						auto childContainerSources = childComponent.GetComponentSource(childComponent.GetOwner()).GetObjectArray("m_aContainers");
						if (containerIdx == -1 || containerIdx >= childContainerSources.Count())
							return ESerializeResult.ERROR;

						float childDefaultValue;
						childContainerSources.Get(containerIdx).Get("m_fResourceValueCurrent", childDefaultValue);
						defaultValue += childDefaultValue;
					}
				}
			}
			else
			{
				containerSources.Get(idx).Get("m_fResourceValueCurrent", defaultValue);
			}

			if (!float.AlmostEqual(value, defaultValue))
			{
				SCR_PersistentResource entry();
				entry.m_eResourceType = resourceType;
				entry.m_fValue = value;
				resources.Insert(entry);
			}
		}

		array<EResourceType> disabledTypes();
		resource.GetDisabledResourceTypes(disabledTypes);
		if (!disabledTypes.IsEmpty())
		{
			array<EResourceType> disabledTypesDefault();
			source.Get("m_aDisabledResourceTypes", disabledTypesDefault);
			foreach (auto removeDefault : disabledTypesDefault)
			{
				disabledTypes.Remove(removeDefault);
			}
		}

		if (resources.IsEmpty() && disabledTypes.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		const bool prev = context.EnableTypeDiscriminator(m_bUseTypeDescriminator);
		context.Write(resources);
		context.EnableTypeDiscriminator(prev);
		
		if (!disabledTypes.IsEmpty() || !context.CanSeekMembers())
			context.Write(disabledTypes);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_ResourceComponent resource = SCR_ResourceComponent.Cast(component);

		int version;
		context.ReadValue("version", version);

		const bool prev = context.EnableTypeDiscriminator(m_bUseTypeDescriminator);
		array<ref SCR_PersistentResource> resources;
		const bool resourcesRead = context.Read(resources);
		context.EnableTypeDiscriminator(prev);
		if (resourcesRead)
		{
			resource.Initialize(owner);

			foreach (SCR_PersistentResource loadResource : resources)
			{
				SCR_ResourceContainer container = resource.GetContainer(loadResource.m_eResourceType);
				if (container)
					container.SetResourceValue(loadResource.m_fValue);
			}
		}

		array<EResourceType> disabledTypes;
		if (context.Read(disabledTypes))
		{
			foreach (auto disabledType : disabledTypes)
			{
				resource.SetResourceTypeEnabled(false, disabledType);
			}
		}

		return true;
	}
}
