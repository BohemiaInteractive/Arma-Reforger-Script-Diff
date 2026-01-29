[EntityEditorProps(category: "GameScripted/Editor (Editables)", description: "")]
class SCR_EditableDecalComponentClass: SCR_EditableEntityComponentClass
{
}

//! @ingroup Editable_Entities

//! Component for managing editable decal entities.
//! It handles changing decal transformation.
//! Moving or rotating DecalEntity has no effect ont he decal itself, so the component deletes existing decal and creates a new one with the new transformation.
//!
//! How to configure prefab with this component:
//! 1. Create GenericEntity
//! 2. Add this component to it
//! 3. Add exactly one child DecalEntity from prefab (must be prefab, otherwise it won't work)
//! 4. Add a hierargy component to the parent and child
//! 5. Change child position / rotation / scale to arrange the decal correctly. Do not change any other attributes, they would be lost when the editable entity is modified in run-time.
class SCR_EditableDecalComponent : SCR_EditableEntityComponent
{
	protected DecalEntity m_Decal;
	protected vector m_vDecalTransform[4];
	protected float m_vDecalScale;
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	override protected void SetTransformBroadcast(vector transform[4])
	{
		super.SetTransformBroadcast(transform);
		
		if (m_Decal)
		{
			//--- Create a new decal and delete the previous one, because moving existing decals is not possible
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.Parent = GetOwner();
			spawnParams.TransformMode = ETransformMode.LOCAL;
			spawnParams.Transform = m_vDecalTransform;
			spawnParams.Scale = m_vDecalScale;
			
			EntityPrefabData decalPrefabData = m_Decal.GetPrefabData();
			ResourceName decalPrefab = SCR_BaseContainerTools.GetPrefabResourceName(decalPrefabData.GetPrefab());
			DecalEntity newDecal = DecalEntity.Cast(GetGame().SpawnEntityPrefab(Resource.Load(decalPrefab), GetOwner().GetWorld(), spawnParams));
			
			delete m_Decal;
			m_Decal = newDecal;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_Decal = DecalEntity.Cast(owner.GetChildren());
		if (m_Decal)
		{
			m_Decal.GetLocalTransform(m_vDecalTransform);
			m_vDecalScale = owner.GetWorldTransformAxis(0).Length() / m_Decal.GetScale();
		}
		else
		{
			Log("No child of type DecalEntity found!", true, LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.NO_LINK | EntityFlags.NO_TREE);
		owner.ClearFlags(EntityFlags.TRACEABLE);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		delete m_Decal;
	}
}
