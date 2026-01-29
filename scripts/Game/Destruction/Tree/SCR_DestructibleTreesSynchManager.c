//------------------------------------------------------------------------------------------------
class SCR_DestructibleTreesSynchManagerClass: ScriptComponentClass
{
};

class SCR_DestructibleTreesSynchManager : ScriptComponent
{
	[Attribute("0")]
	protected bool m_bPrintTreeDamage;
	
#ifdef ENABLE_DESTRUCTION
	static SCR_DestructibleTreesSynchManager instance = null;
	
	private IEntity m_OwnerEntity;
	private BaseSoundComponent m_SoundComponent;
	
	private ref array<ref SCR_DestroyedTreesData> m_aDestroyedTreesData = new array<ref SCR_DestroyedTreesData>();
	private ref array<ref SCR_ActiveTreeData> m_aActiveTreesData = new array<ref SCR_ActiveTreeData>();
	
	protected RplComponent m_RplComponent;
	
	//*********************//
	//PUBLIC MEMBER METHODS//
	//*********************//
	
	//------------------------------------------------------------------------------------------------
	SCR_ActiveTreeData GetActiveTreeData(int index)
	{
		if (!m_aActiveTreesData || m_aActiveTreesData.Count() <= index)
			return null;
		
		return m_aActiveTreesData[index];
	}
	
	//------------------------------------------------------------------------------------------------
	int AddActiveTree()
	{
		if (!m_aActiveTreesData)
			return -3;
		
		return m_aActiveTreesData.Insert(new SCR_ActiveTreeData());
	}
	
	//------------------------------------------------------------------------------------------------
	void SynchronizeTreePartTransform(int treePartIdx, float quat[4], vector position, vector velocity, vector angularVelocity, EntityID treeID)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		int treeIdx = FindTreeIndex(treeID);
		if (treeIdx != -1)
		{
			int treePartArrayIdx = FindTreePartIndex(treeIdx, treePartIdx);
			if (treePartArrayIdx != -1)
			{
				m_aDestroyedTreesData[treeIdx].m_aRotations[treePartArrayIdx*4] = quat[0];
				m_aDestroyedTreesData[treeIdx].m_aRotations[treePartArrayIdx*4 + 1] = quat[1];
				m_aDestroyedTreesData[treeIdx].m_aRotations[treePartArrayIdx*4 + 2] = quat[2];
				m_aDestroyedTreesData[treeIdx].m_aRotations[treePartArrayIdx*4 + 3] = quat[3];
				m_aDestroyedTreesData[treeIdx].m_aPositions[treePartArrayIdx] = position;
			}
		}
		
		SCR_TreePartSynchronizationData data = new SCR_TreePartSynchronizationData();
		data.m_iTreePartIndex = treePartIdx;
		Math3D.QuatCopy(quat, data.m_fQuaternion);
		data.m_vPosition = position;
		data.m_vVelocity = velocity;
		data.m_vAngularVelocity = angularVelocity;
		data.m_TreeID = treeID;
		
		Rpc(RPC_SynchronizeTreePart, data);
	}
	
	//------------------------------------------------------------------------------------------------
	void SynchronizeSetToBreak(int treePartIdx, vector positionVector, vector impulseVector, EDamageType damageType, EntityID treeID)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		SCR_DestructibleTreeV2 tree = FindTree(treeID);
		
		if (!tree)
			return;
		
		int treeIdx = FindTreeIndex(treeID);
		if (treeIdx != -1)
		{
			int treePartArrayIdx = FindTreePartIndex(treeIdx, treePartIdx);
			if (treePartArrayIdx == -1)
			{
				m_aDestroyedTreesData[treeIdx].m_aDestroyedPartsIndexes.Insert(treePartIdx);
				m_aDestroyedTreesData[treeIdx].m_aPositions.Insert("0 0 0");
				m_aDestroyedTreesData[treeIdx].m_aRotations.Insert(0);
				m_aDestroyedTreesData[treeIdx].m_aRotations.Insert(0);
				m_aDestroyedTreesData[treeIdx].m_aRotations.Insert(0);
				m_aDestroyedTreesData[treeIdx].m_aRotations.Insert(0);
				m_aDestroyedTreesData[treeIdx].m_aIsTreePartDynamic.Insert(false);
				m_aDestroyedTreesData[treeIdx].m_aIsParented.Insert(true);
			}
		}
		
		//tree.ServerSetToBreak(treePartIdx, positionVector, impulseVector, damageType);
		Rpc(RPC_SetToBreak, treePartIdx, treeID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SynchronizeRemoveFromParent(int treePartIdx, EntityID treeID)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		int treeIdx = FindTreeIndex(treeID);
		if (treeIdx != -1)
		{
			int treePartArrayIdx = FindTreePartIndex(treeIdx, treePartIdx);
			if (treePartArrayIdx != -1)
			{
				m_aDestroyedTreesData[treeIdx].m_aIsParented[treePartArrayIdx] = false;
			}
		}
		
		Rpc(RPC_RemoveFromParent, treePartIdx, treeID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SynchronizeSetToDestroy(EntityID treeID)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		SCR_DestructibleTreeV2 tree = FindTree(treeID);
		
		if (!tree)
			return;
		
		int treeIdx = FindTreeIndex(treeID);
		if (treeIdx == -1)
		{
			SCR_DestroyedTreesData data = new SCR_DestroyedTreesData();
			data.treeID = treeID;
			m_aDestroyedTreesData.Insert(data);
		}
		
		Rpc(RPC_SendSetToDestroy, treeID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SynchronizeSwitchTreePartToStatic(int treePartIdx, EntityID treeID)
	{
		if (IsProxy())
			return;
		
		int treeIdx = FindTreeIndex(treeID);
		if (treeIdx != -1)
		{
			int treePartArrayIdx = FindTreePartIndex(treeIdx, treePartIdx);
			if (treePartArrayIdx != -1)
			{
				m_aDestroyedTreesData[treeIdx].m_aIsTreePartDynamic[treePartArrayIdx] = false;
			}
		}
		
		Rpc(RPC_SendSwitchTreePartToStatic, treePartIdx, treeID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SynchronizeStaticPosition(int treePartIdx, vector position, EntityID treeID)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		int treeIdx = FindTreeIndex(treeID);
		if (treeIdx != -1)
		{
			int treePartArrayIdx = FindTreePartIndex(treeIdx, treePartIdx);
			if (treePartArrayIdx != -1)
			{
				m_aDestroyedTreesData[treeIdx].m_aPositions[treePartArrayIdx] = position;
			}
		}
		
		Rpc(RPC_SendStaticPosition, treePartIdx, position , treeID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SynchronizeStaticRotation(int treePartIdx, float quat[4], EntityID treeID)
	{
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		int treeIdx = FindTreeIndex(treeID);
		if (treeIdx != -1)
		{
			int treePartArrayIdx = FindTreePartIndex(treeIdx, treePartIdx);
			if (treePartArrayIdx != -1)
			{
				m_aDestroyedTreesData[treeIdx].m_aRotations[treePartArrayIdx*4] = quat[0];
				m_aDestroyedTreesData[treeIdx].m_aRotations[treePartArrayIdx*4 + 1] = quat[1];
				m_aDestroyedTreesData[treeIdx].m_aRotations[treePartArrayIdx*4 + 2] = quat[2];
				m_aDestroyedTreesData[treeIdx].m_aRotations[treePartArrayIdx*4 + 3] = quat[3];
			}
		}
		
		Rpc(RPC_SendStaticRotation, treePartIdx, quat[0], quat[1], quat[2], quat[3], treeID);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DestructibleTreeV2 FindTree(EntityID treeID)
	{
		IEntity treeEnt = m_OwnerEntity.GetWorld().FindEntityByID(treeID);
		if (treeEnt)
		{
			SCR_DestructibleTreeV2 tree = SCR_DestructibleTreeV2.Cast(treeEnt);
			return tree;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	int FindTreeIndex(EntityID treeID)
	{
		if (!treeID)
		{
//			Print("Tree ID is null!");
			return -1;
		}
		
		int count = m_aDestroyedTreesData.Count();
		for (int i = 0; i < count; i++)
		{
			if (m_aDestroyedTreesData[i].treeID == treeID)
				return i;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	int FindTreePartIndex(int treeIdx, int treePartIdx)
	{
		int count = m_aDestroyedTreesData[treeIdx].m_aDestroyedPartsIndexes.Count();
		
		for (int i = 0; i < count; i++)
		{
			if (m_aDestroyedTreesData[treeIdx].m_aDestroyedPartsIndexes[i] == treePartIdx)
				return i;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseSoundComponent GetSoundComponent()
	{
		return m_SoundComponent;
	}
	
	//****************************//
	//PROTECTED MEMBER RPC METHODS//
	//****************************//
	
	//------------------------------------------------------------------------------------------------
	//! Checks if this entity is locally owned
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_RemoveFromParent(int treePartIndex, EntityID treeID)
	{
		SCR_DestructibleTreeV2 tree = FindTree(treeID);
		
		if (!tree)
			return;
		
		tree.RemoveTreePartFromParent(treePartIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RPC_SynchronizeTreePart(SCR_TreePartSynchronizationData data)
	{
//		Print("------DATA RECEIVED------");
		SCR_DestructibleTreeV2 tree = FindTree(data.m_TreeID);
		if (!tree)
			return;
		
		tree.UpdateTransformOfTreePart(data.m_iTreePartIndex, data.m_vPosition, data.m_fQuaternion, data.m_vVelocity, data.m_vAngularVelocity);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RPC_SetToBreak(int treePartIdx, EntityID treeID)
	{
		SCR_DestructibleTreeV2 tree = FindTree(treeID);
		
		if (!tree)
			return;
		
		tree.SetToBreak(treePartIdx);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RPC_SendSetToDestroy(EntityID treeID)
	{
		SCR_DestructibleTreeV2 tree = FindTree(treeID);
//		Print("Client set to destroy " + treeID);
		
		if (!tree)
		{
//			Print("Tree not found");
			return;
		}
		
//		Print(treeID);
		tree.SetToDestroy();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SendSwitchTreePartToStatic(int treePartIdx, EntityID treeID)
	{
		SCR_DestructibleTreeV2 tree = FindTree(treeID);
		
		if (!tree)
			return;
		
		tree.ClientSwitchTreePartToStatic(treePartIdx);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SendStaticPosition(int treePartIdx, vector position, EntityID treeID)
	{
		SCR_DestructibleTreeV2 tree = FindTree(treeID);
		
		if (!tree)
			return;
		
		tree.SetPositionOfTreePart(treePartIdx, position);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SendStaticRotation(int treePartIdx, float q0, float q1, float q2, float q3, EntityID treeID)
	{
		float quat[4];
		quat[0] = q0;
		quat[1] = q1;
		quat[2] = q2;
		quat[3] = q3;
		
		SCR_DestructibleTreeV2 tree = FindTree(treeID);
		
		if (!tree)
			return;
		
		tree.SetRotationOfTreePart(treePartIdx, quat);
	}
	
	//***********************//
	//OVERRIDE MEMBER METHODS//
	//***********************//
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int count;
		reader.ReadInt(count);
//		Print(count);
		
		for (int i = 0; i < count; i++)
		{
			EntityID treeID;
			reader.ReadEntityId(treeID);
			SCR_DestructibleTreeV2 tree = FindTree(treeID);
			
//			Print(treeID);
			int treePartsCount;
			reader.ReadInt(treePartsCount);
			
			if (treePartsCount == 0)
				continue;
			
			tree.SetToDestroy();
			
//			Print(treePartsCount);
			
			for (int x = 0; x < treePartsCount; x++)
			{	
				int treePartIdx;
				reader.ReadInt(treePartIdx);
//				Print(treePartIdx);
				
				vector position;
				reader.ReadVector(position);
//				Print(position);
				
				float rotation[4];
				reader.ReadQuaternion(rotation);
//				Print(rotation);
				
				bool isDynamic;
				reader.ReadBool(isDynamic);
//				Print(isDynamic);
				
				bool isParented;
				reader.ReadBool(isParented);
//				Print(isParented);
				
				tree.CacheDestroyedTreePart(treePartIdx, position, rotation, isDynamic, isParented);
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		int count = m_aDestroyedTreesData.Count();
		writer.WriteInt(count);
//		Print(count);
		
		for (int i = 0; i < count; i++)
		{
			writer.WriteEntityId(m_aDestroyedTreesData[i].treeID);
//			Print(m_aDestroyedTreesData[i].treeID);
			
			int treePartsCount = m_aDestroyedTreesData[i].m_aDestroyedPartsIndexes.Count();
			writer.WriteInt(treePartsCount);
//			Print(treePartsCount);
			
			for (int x = 0; x < treePartsCount; x++)
			{
				writer.WriteInt(m_aDestroyedTreesData[i].m_aDestroyedPartsIndexes[x]);
//				Print(m_aDestroyedTreesData[i].destroyedTreeParts[x]);
				
				writer.WriteVector(m_aDestroyedTreesData[i].m_aPositions[x]);
//				Print(m_aDestroyedTreesData[i].positions[x][0]);
//				Print(m_aDestroyedTreesData[i].positions[x][1]);
//				Print(m_aDestroyedTreesData[i].positions[x][2]);
				
				writer.WriteQuaternion(m_aDestroyedTreesData[i].m_aRotations[x*4]);
//				Print(m_aDestroyedTreesData[i].rotations[x*4]);
//				Print(m_aDestroyedTreesData[i].rotations[x*4 + 1]);
//				Print(m_aDestroyedTreesData[i].rotations[x*4 + 2]);
//				Print(m_aDestroyedTreesData[i].rotations[x*4 + 3]);
				
				writer.WriteBool(m_aDestroyedTreesData[i].m_aIsTreePartDynamic[x]);
//				Print(m_aDestroyedTreesData[i].isTreePartDynamic[x]);
				
				writer.WriteBool(m_aDestroyedTreesData[i].m_aIsParented[x]);
//				Print(m_aDestroyedTreesData[i].isParented[x]);
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame() || !GetGame().InPlayMode())
			return;
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		SCR_DestructibleTreeV2.s_bPrintTreeDamage = m_bPrintTreeDamage;
		
		if (!instance)
		{
			instance = this;
			SCR_DestructibleTreeV2.synchManager = instance;
			SCR_TreePartV2.synchManager = instance;
		}
		
		m_OwnerEntity = owner;
		
		m_SoundComponent = BaseSoundComponent.Cast(owner.FindComponent(BaseSoundComponent));
	}
	
	//************************//
	//CONSTRUCTOR & DESTRUCTOR//
	//************************//
	
	//------------------------------------------------------------------------------------------------
	void SCR_DestructibleTreesSynchManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_DestructibleTreesSynchManager()
	{
		m_aDestroyedTreesData.Clear();
		m_aDestroyedTreesData = null;
	}
#endif
};
