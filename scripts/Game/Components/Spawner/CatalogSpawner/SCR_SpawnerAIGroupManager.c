class SCR_SpawnerAIGroupManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_SpawnerAIGroupManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute(defvalue: "1.5", params: "0.1 inf", desc: "Delay between spawning group members.")]
	protected float m_fGroupMemberSpawnDelay;

	protected ref array<ref SCR_SpawnerAIRequest> m_aAIQueue;
	protected float m_fCurrentGroupMemberSpawnDelay;

	//------------------------------------------------------------------------------------------------
	//! Live check against the active-AI budget - "at limit" means CanActivateGroup refuses a
	//! group-less query.
	bool IsAtAILimit()
	{
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (!aiWorld)
			return false;
		return !aiWorld.CanActivateGroup(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] spawner
	//! \param[in] resName
	//! \param[in] user
	//! \param[in] slotEntity
	//! \param[in] rallyPoint
	void QueueSpawn(notnull SCR_CatalogEntitySpawnerComponent spawner, ResourceName resName, notnull IEntity user, notnull IEntity slotEntity, notnull SCR_EntityLabelPointComponent rallyPoint)
	{
		if (!GetGameMode().IsMaster())
			return;
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(resName))
		{
			Print("'SCR_SpawnerAIGroupManagerComponent' resName is empty!", LogLevel.ERROR);
			return;
		}
			
		if (!m_aAIQueue)
			m_aAIQueue = new array<ref SCR_SpawnerAIRequest>;

		m_aAIQueue.Insert(new SCR_SpawnerAIRequest(spawner, resName, user, slotEntity, rallyPoint));
		
		if (m_aAIQueue.Count() == 1)
			SetEventMask(GetOwner(), EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fCurrentGroupMemberSpawnDelay += timeSlice;
		if (m_fCurrentGroupMemberSpawnDelay < m_fGroupMemberSpawnDelay)
			return;
		
		SCR_SpawnerAIRequest spawnerRequest = m_aAIQueue[0];
		
		spawnerRequest.m_Spawner.SpawnAIGroupMember(spawnerRequest.m_ResourceName, spawnerRequest.m_UserEntity, spawnerRequest.m_SlotEntity, spawnerRequest.m_RallyPoint);
		m_aAIQueue.RemoveOrdered(0);
		m_fCurrentGroupMemberSpawnDelay = 0;

		if (m_aAIQueue.IsEmpty())
			ClearEventMask(owner, EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
}

class SCR_SpawnerAIRequest
{
	SCR_CatalogEntitySpawnerComponent m_Spawner;
	ResourceName m_ResourceName;
	IEntity m_UserEntity;
	IEntity m_SlotEntity;
	SCR_EntityLabelPointComponent m_RallyPoint;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] spawner
	//! \param[in] resName
	//! \param[in] userEntity
	//! \param[in] slotEntity
	//! \param[in] labelComp
	void SCR_SpawnerAIRequest(SCR_CatalogEntitySpawnerComponent spawner, ResourceName resName, IEntity userEntity, IEntity slotEntity, SCR_EntityLabelPointComponent labelComp)
	{
		m_Spawner = spawner;
		m_ResourceName = resName;
		m_UserEntity = userEntity;
		m_SlotEntity = slotEntity;
		m_RallyPoint = labelComp;
	}
}
