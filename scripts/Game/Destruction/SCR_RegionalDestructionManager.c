class SCR_RegionalDestructionManagerClass : RegionalDestructionManagerClass
{
}

class SCR_RegionalDestructionManager : RegionalDestructionManager
{
	ref array<ref SCR_DestructionInteriorBoundingBox> m_aInteriorBoxes = {};

	//------------------------------------------------------------------------------------------------
	override bool OnSave(notnull ScriptBitWriter writer)
	{
		writer.WriteInt(m_aInteriorBoxes.Count());
		foreach (SCR_DestructionInteriorBoundingBox boundingBox : m_aInteriorBoxes)
		{
			if (!boundingBox)
				continue;

			boundingBox.OnSave(writer);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnLoad(notnull ScriptBitReader reader)
	{
		m_aInteriorBoxes.Clear();

		int count;
		reader.ReadInt(count);
		
		SCR_DestructionManager manager = SCR_DestructionManager.GetDestructionManagerInstance();
		if (!manager)
			return true;
		
		ref array<ref vector> boxesPositions = {};
		bool exists = manager.m_RegionalManagerHandledBoxes.Find(GetRplID(), boxesPositions);

		if(!exists)
		{
			boxesPositions = new array<ref vector>;
			manager.m_RegionalManagerHandledBoxes.Insert(GetRplID(), boxesPositions);
		}
		
		for (int i = 0; i < count; i++)
		{
			SCR_DestructionInteriorBoundingBox boundingBox = new SCR_DestructionInteriorBoundingBox();
			boundingBox.OnLoad(reader);
			
			//check if box was handled already
			if (boxesPositions && boxesPositions.Contains(boundingBox.m_vMatrix[3] + boundingBox.m_center))
				continue;
			
			m_aInteriorBoxes.Insert(boundingBox);
			boxesPositions.Insert(boundingBox.m_vMatrix[3] + boundingBox.m_center);
		}

		ProcessInteriorBoundingBoxes();
		

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! CGoes through the interior boxes registered in regional manager to query entities in them for destruction
	void ProcessInteriorBoundingBoxes()
	{
		foreach (SCR_DestructionInteriorBoundingBox boundingBox : m_aInteriorBoxes)
		{
			QueryEntities(boundingBox.m_vMin, boundingBox.m_vMax, boundingBox.m_vMatrix);
		}

		SCR_BuildingInteriorDeletinator.QueueDeletionsThisFrame();
	}

	//------------------------------------------------------------------------------------------------
	void RegisterInteriorBoundingBox(vector m_vStartMatrix[4], vector mins, vector maxs, vector center)
	{
		SCR_DestructionInteriorBoundingBox boundingBox = new SCR_DestructionInteriorBoundingBox();
		boundingBox.m_vMatrix = m_vStartMatrix;

		boundingBox.SetBounds(mins, maxs);
		boundingBox.m_center = center;
		m_aInteriorBoxes.Insert(boundingBox);
	}

	//------------------------------------------------------------------------------------------------
	void QueryEntities(vector min, vector max, vector m_vMatrix[4])
	{
		EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.STATIC | EQueryEntitiesFlags.NO_PROXIES;
		BaseWorld world = GetGame().GetWorld();

		//TODO GET THIS OUT OF THIS CLASS PLZ
		world.QueryEntitiesByOBB(min, max, m_vMatrix, AddEntityCallback, QueryFilterCallback, queryFlags);
	}

	//------------------------------------------------------------------------------------------------
	//! Filters what entities should be handled
	protected bool AddEntityCallback(notnull IEntity e)
	{
		// There shouldn't really be any physical hierchies created on buildings and alike,
		// so more than anything, this check is just a precaution.
		// I wish it was just a precaution - Mour
		if (!e.GetPhysics() && !e.GetChildren())
			return true;

		// Exclude static entities such as trees, rocks, ruins etc. and other destructible buildings
		SCR_BuildingDestructionManagerComponent manager = GetGame().GetBuildingDestructionManager();
		foreach (typename typeName : manager.GetExcludedQueryTypes())
		{
			if (e.IsInherited(typeName) && e.Type() != SCR_PowerPole)
					return true;
		}

		SCR_EditorLinkComponent linkComp = SCR_EditorLinkComponent.Cast(e.FindComponent(SCR_EditorLinkComponent));
		if (linkComp)
			return true;

		SCR_BuildingInteriorDeletinator.InsertEntitiesToDelete(e);
		return true;
	}

	//! Used to filter out entities that are not meant to be handled in AddEntityCallback
	protected bool QueryFilterCallback(notnull IEntity entity)
	{
		RplComponent rplComp = RplComponent.Cast(entity.FindComponent(RplComponent));
		if (rplComp)
			return true;

		IEntity entityParent = entity.GetParent();

		// Exclude the owner && children of other objects
		if (entityParent)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static DestructionManager GetDestructionManager(BaseWorld world)
	{
		ChimeraWorld chimeraWorld = ChimeraWorld.CastFrom(world);
		if (chimeraWorld)
			return chimeraWorld.GetDestructionManager();

		return null;
	}
	
	RplId GetRplID()
	{
		BaseRplComponent component = BaseRplComponent.Cast(FindComponent(BaseRplComponent));
		
		//all regional managers should have rpl component, no need to double-check
		return component.Id();
	}
}

class SCR_DestructionInteriorBoundingBox
{
	bool m_bIsHandled;
	vector m_vMin;
	vector m_vMax;
	
	vector m_center;

	vector m_vMatrix[4];

	//------------------------------------------------------------------------------------------------
	void OnSave(notnull ScriptBitWriter writer)
	{
		writer.WriteVector(m_vMatrix[0]);
		writer.WriteVector(m_vMatrix[1]);
		writer.WriteVector(m_vMatrix[2]);
		writer.WriteVector(m_vMatrix[3]);

		writer.WriteVector(m_vMin);
		writer.WriteVector(m_vMax);
		writer.WriteVector(m_center);
	}

	//------------------------------------------------------------------------------------------------
	void OnLoad(notnull ScriptBitReader reader)
	{
		for (int i = 0; i < 4; i++)
		{
			vector value;
			reader.ReadVector(value);
			m_vMatrix[i] = value;
		}

		reader.ReadVector(m_vMin);
		reader.ReadVector(m_vMax);
		reader.ReadVector(m_center);
	}

	//------------------------------------------------------------------------------------------------`
	void SetBounds (vector min_, vector max_)
	{
		m_vMin = min_;
		m_vMax = max_;
	}
}

class SCR_BuildingInteriorDeletinator
{
	static int s_iDeletionsThisFrame = 0;
	static const int MAX_DELETIONS_PER_FRAME = 30;
	static bool s_bDeletionsQueued = false;

	static ref array<IEntity> s_aEntitiesToDelete = {};

	//------------------------------------------------------------------------------------------------
	static void InsertEntitiesToDelete(IEntity entityToDelete)
	{
		s_aEntitiesToDelete.Insert(entityToDelete);
	}

	//------------------------------------------------------------------------------------------------
	static void QueueDeletionsThisFrame()
	{
		if (s_bDeletionsQueued)
			return;

		s_bDeletionsQueued = true;
		GetGame().GetCallqueue().CallLater(SCR_BuildingInteriorDeletinator.DeleteInteriorEntities);
	}

	//------------------------------------------------------------------------------------------------
	// Handles deletion of the interior entities
	static void DeleteInteriorEntities()
	{
		IEntity entityToDelete;

		int lastElementIndex = s_aEntitiesToDelete.Count() -1;
		int previousSize = s_aEntitiesToDelete.Count();

		for (int index = lastElementIndex; s_iDeletionsThisFrame < MAX_DELETIONS_PER_FRAME; s_iDeletionsThisFrame++)
		{
			int indexToDelete = lastElementIndex - s_iDeletionsThisFrame;
			if (indexToDelete < 0)
				break;

			entityToDelete = s_aEntitiesToDelete[indexToDelete];

			if (!entityToDelete)
				continue;

			//Even if they don't have rpl component, DeleteRplEntity will delete the entity for this client
			RplComponent.DeleteRplEntity(entityToDelete, false);
		}

		if (s_aEntitiesToDelete.Count() != previousSize)
		{
			Print("SCR_BuildingInteriorDeletinator::While deleting building interiors new entities where added to the array of entities to delete, this is a HUGE issue!", LogLevel.ERROR);
			s_bDeletionsQueued = false;
			QueueDeletionsThisFrame();
			return;
		}

		s_aEntitiesToDelete.Resize(previousSize - s_iDeletionsThisFrame);

		s_iDeletionsThisFrame = 0;

		//deletions have already been done here
		s_bDeletionsQueued = false;

		if (!s_aEntitiesToDelete.IsEmpty())
			QueueDeletionsThisFrame();
	}
}
