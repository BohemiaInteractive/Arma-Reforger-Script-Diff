[BaseContainerProps()]
class ResourceRequest
{	
	[Attribute( "", UIWidgets.CheckBox, "Should be cached only on the client side?" )]
	bool onlyClient;	
	
	ResourceName GetResource()
	{}	
};


[BaseContainerProps()]
class VehicleResourceRequest extends ResourceRequest
{
	[Attribute( "", UIWidgets.ResourceNamePicker, "entity prefab", "et" )]
	ResourceName resource;
	
	override ResourceName GetResource()
	{
		return resource;
	}
};

[BaseContainerProps()]
class ProjectileResourceRequest extends ResourceRequest
{
	[Attribute( "", UIWidgets.ResourceNamePicker, "entity prefab", "et" )]
	ResourceName resource;
	
	override ResourceName GetResource()
	{
		return resource;
	}
};

[BaseContainerProps()]
class ParticleResourceRequest extends ResourceRequest
{
	[Attribute( "", UIWidgets.ResourceNamePicker, "entity prefab", "ptc" )]
	ResourceName resource;
	
	override ResourceName GetResource()
	{
		return resource;
	}	
};

class Registrator<Class T>
{
	void RegisterPrefabArray(array<ref T> resources, PrefabList manager)
	{
		if (resources == null)
			return;
		int count = resources.Count();
		for( int i = 0; i < count; i++ )
		{
			T res = resources[i];
			if( !res.resource.IsEmpty() )
			{
				manager.RegisterPrefab(resources[i].resource, resources[i].onlyClient);
			}			
		}
	}
};

class BasicPrefabListClass: PrefabListClass
{
};

class BasicPrefabList : PrefabList
{
	[Attribute( "", UIWidgets.Object, "Vehicle" )]
	ref array<ref VehicleResourceRequest> vehicleResources;
	
	[Attribute( "", UIWidgets.Object, "Projectile")]
	ref array<ref ProjectileResourceRequest> projectileResources;
	
	[Attribute( "", UIWidgets.Object, "Particle")]
	ref array<ref ParticleResourceRequest> particleResources;
	
	override void OnPostInit(IEntity owner)
	{
		Registrator<VehicleResourceRequest> vehicleReg = new Registrator<VehicleResourceRequest>();
		
		vehicleReg.RegisterPrefabArray(vehicleResources, this)		
	}
	
};