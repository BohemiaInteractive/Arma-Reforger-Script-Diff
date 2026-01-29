[BaseContainerProps()]
class SCR_ResourceGeneratorActionDropContainers : SCR_ResourceGeneratorActionBase
{
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, params: "inf inf 0 purpose=coords space=entity")]
	protected vector m_vSpawnOrigin;
	
	[Attribute(defvalue: "10.0", uiwidget: UIWidgets.SpinBox, params: "0.0 inf 1.0")]
	protected float m_fSpawnRadius;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Resource Container Prefab to be used as the dropped container.", "et")]
	protected ResourceName m_sResourceContainerPrefab;
	
	//------------------------------------------------------------------------------------------------
	//! \return The default prefab name for the container to spawn for the action.
	static ResourceName GetDefaultPrefab()
	{
		return "{A8A4609979D3385D}Prefabs/Props/Military/SupplyBox/SupplyStack/SupplyStack_Small_01/SupplyStack_Small_01_item.et";
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasConstrainedGeneration()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override float ComputeGeneratedResources(notnull SCR_ResourceGenerator generator, float resourceValue)
	{
		return resourceValue;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns a container with a given resource value. The resource value will be of course clamped
	//! to the limits of the container configuration that was selected.
	//!
	//! \param[in] generator The generator that caused the action.
	//! \param[inout] resourceValue The resource value that is expected for the container to have.
	//! \return The created container, if any otherwise null.
	protected SCR_ResourceContainer SpawnContainer(notnull SCR_ResourceGenerator generator, inout float resourceValue)
	{
		if (m_sResourceContainerPrefab.IsEmpty() && GetDefaultPrefab().IsEmpty())
		{
			Debug.Error2(ClassName() + " is misconfigured!.", "The generation action for dropping a container must have a Resource Container Prefab to be used as the dropped container.");
			
			return null;
		}

		vector position = generator.GetOwnerOrigin();
		SCR_ResourcePlayerControllerInventoryComponent.FindSuitablePosition(position, m_fSpawnRadius, m_fSpawnRadius);
		
		const EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = position;
		IEntity containerEntity;
		
		if (m_sResourceContainerPrefab.IsEmpty())
			containerEntity = GetGame().SpawnEntityPrefab(Resource.Load(GetDefaultPrefab()), GetGame().GetWorld(), spawnParams);
		else
			containerEntity = GetGame().SpawnEntityPrefab(Resource.Load(m_sResourceContainerPrefab), GetGame().GetWorld(), spawnParams);
		
		if (!containerEntity)
		{
			Debug.Error2(ClassName() + " is misconfigured!.", "The generation action for dropping a container must have a valid prefab to spawn.");
			
			return null;
		}
		
		const SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(containerEntity);
		
		if (!resourceComponent)
		{
			Debug.Error2(ClassName() + " is misconfigured!.", "The generation action for dropping a container must have prefab to spawn that has a Resource Component.");
			
			return null;
		}
		
		const SCR_ResourceContainer container = resourceComponent.GetContainer(generator.GetResourceType());
		
		if (!container)
		{
			Debug.Error2(ClassName() + " is misconfigured!.", "The generation action for dropping a container must have prefab to spawn that has a Resource Container of the same type as the Resource Generator.");
			
			return null;
		}
		
		return container;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Helper method to process a specific container of the list of container to perform the action on.
	//!
	//! \param[in] generator The container that the action is being performed on.
	//! \param[inout] resourceValue The resource value that is to be used with the action.
	protected void ProcessContainer(notnull SCR_ResourceContainer container, inout float resourceValue)
	{
		container.SetResourceValue(resourceValue);
		
		resourceValue -= container.GetResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(notnull SCR_ResourceGenerator generator, inout float resourceValue)
	{
		SCR_ResourceContainer container;
		
		while (resourceValue > 0)
		{
			container = SpawnContainer(generator, resourceValue);
			
			if (!container)
				break;
			
			ProcessContainer(container, resourceValue);
		}
	}
}