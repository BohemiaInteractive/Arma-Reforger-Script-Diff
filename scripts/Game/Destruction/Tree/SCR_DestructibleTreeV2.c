[EntityEditorProps(category: "GameScripted/TreeDestructionV2", description: "A tree entity that can be destroyed.", color: "0 0 255 255", visible: false, dynamicBox: true)]
class SCR_DestructibleTreeV2Class : TreeClass
{
	[Attribute("200")]
	float m_fDamageThreshold;

	[Attribute("30000")]
	float m_fHealth;

	[Attribute("-1", desc: "Pick a prefab of the broken tree.")]
	ref SCR_TreePartHierarchyV2 m_TreePartHierarchy;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] prefab
	void SCR_DestructibleTreeV2Class(BaseContainer prefab)
	{
//		Print("CreatePrefabData");
//		Print(prefab.GetClassName());
//		Print(prefab.GetName());
//		Print(m_fDamageThreshold);
//		Print(m_fHealth);
	}
}

class SCR_TreeTraceInfo
{
	float impulse;
	float distance;
	vector position;
	vector direction;
	EDamageType type;
}

class SCR_DestroyedTreePartsData
{
	int treePartIdx;
	vector position;
	float rotation[4];
	bool isDynamic;
	bool isParented;
}

class SCR_ActiveTreeData
{
	ref array<SCR_TreePartV2> m_aTreeParts = null;
	ref array<ref SCR_TreeTraceInfo> m_aTreeTraceInfo = null;
	ref array<ref SCR_DestroyedTreePartsData> m_aCachedTreePartsToDestroy = null;
	ref array<int> m_aTreePartsToBreak = null;
	float m_fDamage = 0;
	float m_fTimeSinceLastSoundEvent = 0;
}

//! Encapsulates the functionality of a destructible tree entity in the world.
class SCR_DestructibleTreeV2 : Tree
{
#ifdef ENABLE_DESTRUCTION
	static const int INACTIVE_TREE_INDEX = -1;
	static const int DESTROYED_TREE_INDEX = -2;
	
	static bool s_bPrintTreeDamage = 0;
	static SCR_DestructibleTreesSynchManager synchManager = null;
	
	//THESE ARE RUNTIME
	//private ref SCR_ActiveTreeData treeData = null;
	private int m_iIndex = INACTIVE_TREE_INDEX;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsDestroyed()
	{
		return m_iIndex == DESTROYED_TREE_INDEX;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] sentRPC
	void SetToDestroy(bool sentRPC = false)
	{
		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE);
		
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		if (!sentRPC)
		{
			if (!SCR_DestructibleTreesSynchManager.instance)
				return;
			EntityID id = GetID();
			synchManager.SynchronizeSetToDestroy(id);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes the tree prefab into multiple tree part entities which are held together by joints.
	//! Also applies the impulse vector to the correct tree part.
	private void Destroy()
	{
		//Print("Destroy");
		if (!GetIsDestroyed())
		{
			Physics physics = GetPhysics();
			if (physics)
				physics.Destroy();
			SetObject(null, "");
			
			//Store new the newly spawned prefab in child variable.
			IEntity child = BuildHierarchy();
			
			if (!child)
				child = GetChildren();
		}
		
		if (m_iIndex < 0 || !synchManager)
			return;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		if (treeData && treeData.m_aTreePartsToBreak)
		{
			foreach (int i : treeData.m_aTreePartsToBreak)
			{
				SetToBreak(i);
				RemoveTreePartFromParent(i)
			}

			treeData.m_aTreePartsToBreak.Clear();
			treeData.m_aTreePartsToBreak = null;
		}
		
		m_iIndex = DESTROYED_TREE_INDEX;
		
		// TODO: Hotfix calling update to ensure colliders are traceable in this frame, remove once engine handles this.
		if (treeData && treeData.m_aTreeParts)
		{
			foreach (SCR_TreePartV2 treePart : treeData.m_aTreeParts)
			{
				treePart.Update();
			}
		}
		
		Trace();
		
		if (!treeData || !treeData.m_aCachedTreePartsToDestroy)
			return;
		
		int cachedTreePartsCount = treeData.m_aCachedTreePartsToDestroy.Count();
		for (int i = cachedTreePartsCount-1; i >= 0; i--)
		{
			SCR_TreePartV2 treePart = FindTreePart(treeData.m_aCachedTreePartsToDestroy[i].treePartIdx);
			//Print(treePart);
			if (treePart)
			{
				if (treeData.m_aCachedTreePartsToDestroy[i].isDynamic)
					treePart.SetPhysics(true);
				else
					treePart.SetPhysics(false);
				//Print(m_aCachedTreePartsToDestroy[i].isParented);
				treePart.CachePosition(treeData.m_aCachedTreePartsToDestroy[i].position);
				treePart.CacheRotation(treeData.m_aCachedTreePartsToDestroy[i].rotation);
//				Print("Caching data rotation: ");
//				Print(m_aCachedTreePartsToDestroy[i].rotation);
				if (!treeData.m_aCachedTreePartsToDestroy[i].isParented)
					treePart.ClientSetToBreakFromParent();
			}
			treeData.m_aCachedTreePartsToDestroy[i] = null;
			treeData.m_aCachedTreePartsToDestroy.Remove(i);
		}
		treeData.m_aCachedTreePartsToDestroy.Clear();
		treeData.m_aCachedTreePartsToDestroy = null;
	}
	
	//------------------------------------------------------------------------------------------------
	private IEntity BuildHierarchy()
	{
		SCR_DestructibleTreeV2Class prefabData = SCR_DestructibleTreeV2Class.Cast(GetPrefabData());
		if (!prefabData || !prefabData.m_TreePartHierarchy || prefabData.m_TreePartHierarchy.m_Prefab.GetPath() == "-1")
			return null;
		
		if (m_iIndex < 0 || !synchManager)
			return null;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		if (!treeData)
			treeData = new SCR_ActiveTreeData();
		
		if (!treeData.m_aTreeParts)
			treeData.m_aTreeParts = {};
		
		Resource resource = Resource.Load(prefabData.m_TreePartHierarchy.m_Prefab);		
		if (!resource.IsValid())
			return null;
		
		vector mat[4];
		GetTransform(mat);
		
		IEntity rootEnt = GetGame().SpawnEntityPrefab(resource);
		
		if (RplSession.Mode() == RplMode.Client)
			AddChild(rootEnt, -1, EAddChildFlags.AUTO_TRANSFORM);
		else
			AddChild(rootEnt, -1, EAddChildFlags.NONE);
		
		rootEnt.SetTransform(mat);
		SCR_TreePartV2 rootTreePart = SCR_TreePartV2.Cast(rootEnt);
		
		if (rootTreePart)
		{
			rootTreePart.m_vLockedOrigin = GetOrigin();
			treeData.m_aTreeParts.Insert(rootTreePart);
		}
		
		prefabData.m_TreePartHierarchy.SpawnAllChildren(rootEnt, treeData.m_aTreeParts);
		
		array<IEntity> children = {};
		SCR_Global.GetHierarchyEntityList(rootEnt, children);
		
		foreach (SCR_TreePartV2 treePart : treeData.m_aTreeParts)
		{
			treePart.m_ParentTree = this;
		}
		
		prefabData.m_TreePartHierarchy = null;
		
		return rootEnt;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] treePartIdx
	//! \param[in] quat
	void SetRotationOfTreePart(int treePartIdx, float quat[4])
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		if (!foundTreePart)
			return;
		
		foundTreePart.SetRotation(quat);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	//! \param[in] quat
	void CacheRotationOfTreePart(int treePartIdx, float quat[4])
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		
		if (!foundTreePart)
			return;
		
		foundTreePart.CacheRotation(quat);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	//! \param[in] toDynamic
	void CacheSwitchTreePartPhysics(int treePartIdx, bool toDynamic)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		
		if (!foundTreePart)
			return;
		
		foundTreePart.SetPhysics(toDynamic);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	//! \param[in] positionVector
	//! \param[in] impulseVector
	//! \param[in] damageType
	void ServerSetToBreak(int treePartIdx, vector positionVector, vector impulseVector, EDamageType damageType)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		if (!foundTreePart)
			return;
		
		foundTreePart.SetToBreak(treePartIdx, positionVector, impulseVector, damageType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] treePartIdx
	void SetToBreak(int treePartIdx)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		
		if (!foundTreePart)
		{
			if (m_iIndex < 0 || !synchManager)
				return;
			
			SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
			
			if (!treeData)
				treeData = new SCR_ActiveTreeData();

			if (!treeData.m_aTreePartsToBreak)
				treeData.m_aTreePartsToBreak = {};

			treeData.m_aTreePartsToBreak.Insert(treePartIdx);
			return;
		}
		
		foundTreePart.SetToBreak();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIndex
	void RemoveTreePartFromParent(int treePartIndex)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIndex);
		if (!foundTreePart)
			return;
		
		foundTreePart.RemoveFromParent();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	//! \param[in] position
	//! \param[in] quat
	//! \param[in] isDynamic
	//! \param[in] isParented
	void CacheDestroyedTreePart(int treePartIdx, vector position, float quat[4], bool isDynamic, bool isParented)
	{
		SCR_DestroyedTreePartsData data = new SCR_DestroyedTreePartsData();
		data.treePartIdx = treePartIdx;
		data.position[0] = position[0];
		data.position[1] = position[1];
		data.position[2] = position[2];
		Math3D.QuatCopy(quat, data.rotation);
		data.isDynamic = isDynamic;
		data.isParented = isParented;
		
		if (m_iIndex < 0 || !synchManager)
			return;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		if (!treeData)
			treeData = new SCR_ActiveTreeData();
		
		if (!treeData.m_aCachedTreePartsToDestroy)
			treeData.m_aCachedTreePartsToDestroy = {};

		treeData.m_aCachedTreePartsToDestroy.Insert(data);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	//! \param[in] pos
	//! \param[in] quat
	//! \param[in] velocityLinear
	//! \param[in] velocityAngular
	void UpdateTransformOfTreePart(int treePartIdx, vector pos, float quat[4], vector velocityLinear, vector velocityAngular)
	{
		SCR_TreePartV2 treePart = FindTreePart(treePartIdx);
		if (!treePart)
			return;
		
		treePart.UpdateTransform(pos, quat, velocityLinear, velocityAngular);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] treePartIdx
	//! \param[in] pos
	void SetPositionOfTreePart(int treePartIdx, vector pos)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		if (!foundTreePart)
			return;
		
		foundTreePart.SetPosition(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	//! \param[in] pos
	void CachePositionOfTreePart(int treePartIdx, vector pos)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		if (!foundTreePart)
			return;
		
		foundTreePart.CachePosition(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	//! \param[in] velocity
	void UpdateVelocityOfTreePart(int treePartIdx, vector velocity)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		if (!foundTreePart)
			return;
		
		foundTreePart.SetVelocity(velocity);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	//! \param[in] angularVelocity
	void UpdateAngularVelocityOfTreePart(int treePartIdx, vector angularVelocity)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		if (!foundTreePart)
			return;
		
		foundTreePart.SetAngularVelocity(angularVelocity);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] treePartIdx
	void ClientSwitchTreePartToStatic(int treePartIdx)
	{
		SCR_TreePartV2 foundTreePart = FindTreePart(treePartIdx);
		if (!foundTreePart)
			return;
		
		foundTreePart.SetPhysics(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] position
	//! \param[in] direction
	//! \param[in] impulse
	//! \param[in] distance
	//! \param[in] type
	//! \param[in] other
	void SetToTrace(vector position, vector direction, float impulse, float distance, EDamageType type, IEntity other)
	{
		SCR_TreeTraceInfo info = new SCR_TreeTraceInfo();
		info.position = position;
		info.direction = direction;
		info.impulse = impulse;
		info.distance = distance;
		info.type = type;
		
		if (m_iIndex < 0 || !synchManager)
			return;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		if (!treeData)
			treeData = new SCR_ActiveTreeData();

		if (!treeData.m_aTreeTraceInfo)
			treeData.m_aTreeTraceInfo = {};

		treeData.m_aTreeTraceInfo.Insert(info);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Trace()
	{
		if (m_iIndex < 0 || !synchManager)
			return;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		if (!treeData || !treeData.m_aTreeTraceInfo)
			return;
		
		int count = treeData.m_aTreeTraceInfo.Count();
		for (int i = count - 1; i >= 0; i--)
		{
			SCR_TreeTraceInfo info = treeData.m_aTreeTraceInfo[i];
			
			if (info.type == EDamageType.EXPLOSIVE)
			{
				foreach (SCR_TreePartV2 treePart : treeData.m_aTreeParts)
				{
					if (treePart.m_bDebugDamage)
						treePart.PrintDamageDebug(info.impulse, info.type);

					float reducedImpulse;
					if (treePart.WouldBreak(info.impulse, info.type, reducedImpulse))
						treePart.SetToBreak(positionVector: info.position, impulseVector: reducedImpulse * info.direction, damageType: info.type);
				}
			}
			else 
			{
				TraceSphere param = new TraceSphere();
				
				param.Radius = 0.1;
				param.Start = info.position;
				param.End = info.position + info.direction;
				param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
				param.LayerMask = EPhysicsLayerDefs.Camera; // is this a correct layer mask?
				
				SCR_Global.g_TraceFilterEnt = this;
				
				vector tracePos = GetWorld().TraceMove(param, SCR_Global.FilterCallback_IgnoreAllButEntityWithChildren) * (param.End - param.Start) + param.Start;
				
				IEntity traceEnt = param.TraceEnt;
				if (traceEnt)
				{
					SCR_TreePartV2 tracedTreePart = SCR_TreePartV2.Cast(traceEnt);
					if (tracedTreePart)
					{
						if (tracedTreePart.m_bDebugDamage)
							tracedTreePart.PrintDamageDebug(info.impulse, info.type);

						float reducedImpulse;
						if (tracedTreePart.WouldBreak(info.impulse, info.type, reducedImpulse))
							tracedTreePart.SetToBreak(positionVector: info.position, impulseVector: reducedImpulse * info.direction, damageType: info.type);
					}
				}
				
				treeData.m_aTreeTraceInfo[i] = null;
			}
		}

		treeData.m_aTreeTraceInfo.Clear();
		treeData.m_aTreeTraceInfo = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTreePartHitGround()
	{
		if (m_iIndex < 0 || !synchManager)
			return;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		if (treeData.m_fTimeSinceLastSoundEvent < 0.5)
			return;
		treeData.m_fTimeSinceLastSoundEvent = 0;
		
		if (synchManager)
		{
			BaseSoundComponent soundComponent = synchManager.GetSoundComponent();
			if (soundComponent)
				soundComponent.PlayStr(SCR_SoundEvent.SOUND_HIT_GROUND);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDamage(EDamageType type,
				  float damage,
				  HitZone pHitZone,
				  notnull Instigator instigator, 
				  inout vector hitTransform[3], 
				  float speed,
				  int colliderID, 
				  int nodeID)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		if (s_bPrintTreeDamage)
			Print(damage);
		
		SCR_DestructibleTreeV2Class prefabData = SCR_DestructibleTreeV2Class.Cast(GetPrefabData());
		
		if (damage < prefabData.m_fDamageThreshold)
			return;
		
		if (!synchManager)
			return;
		
		if (m_iIndex < 0)
			m_iIndex = synchManager.AddActiveTree();
		
		if (m_iIndex < 0)
			return;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		treeData.m_fDamage += damage;
		if (treeData.m_fDamage < prefabData.m_fHealth)
			return;
		
		float distance = 0;
		if (damageSource)
			distance = (damageSource.GetOrigin() - GetOrigin()).Length() * 1.1;
		
		//Calculate impulse vector from direction vector and damage
		vector positionVector = outMat[0];
		vector direction = outMat[1];
		
		if (type == EDamageType.EXPLOSIVE)
		{
			positionVector = "0 0 0";
			direction = (GetOrigin() - outMat[0]) * damage;
		}
		
		vector position = outMat[0] + (-outMat[1] * 0.5); // Position of hit + normal of the hit
		
		direction.Normalize();
		
		Vehicle vehicle = Vehicle.Cast(damageSource);
		if (vehicle)
			SetToTrace(position, direction, damage, distance, type, vehicle);
		else 
			SetToTrace(position, direction, damage, distance, type, null);

		SetToDestroy();
	}
	
	//------------------------------------------------------------------------------------------------
	private SCR_TreePartV2 FindTreePart(int treePartIdx)
	{
		if (m_iIndex < 0 || !synchManager)
			return null;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		if (!treeData || !treeData.m_aTreeParts)
			return null;
		
		foreach (SCR_TreePartV2 treePart : treeData.m_aTreeParts)
		{
			if (treePart.GetTreePartIndex() == treePartIdx)
				return treePart;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_iIndex < 0 || !synchManager)
			return;
		
		SCR_ActiveTreeData treeData = synchManager.GetActiveTreeData(m_iIndex);
		
		if (!GetIsDestroyed())
		{
			Destroy();
			
			treeData.m_fTimeSinceLastSoundEvent = 0;
		}
		
		treeData.m_fTimeSinceLastSoundEvent += timeSlice;
		
		if (treeData.m_fTimeSinceLastSoundEvent > 10)
		{
			treeData.m_fTimeSinceLastSoundEvent = 0;
			ClearEventMask(EntityEvent.FRAME);
			ClearFlags(EntityFlags.ACTIVE);
		}
	}
#endif
}
