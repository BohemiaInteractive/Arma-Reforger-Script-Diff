class SCR_UserActionWithOccupancy: ScriptedUserAction
{
	protected IEntity m_Occupant;
	protected RplId m_OccupantRplId;
	//------------------------------------------------------------------------------------------------
	bool IsOccupied()
	{
		return m_Occupant != null;
	}
	
	//------------------------------------------------------------------------
	bool IsOccupiedBy(IEntity ent)
	{
		if (!ent)
			return false;
		
		if (m_Occupant)
			return m_Occupant == ent;
		
		return SCR_EntityHelper.EntityToRplId(ent) == m_OccupantRplId;
	}
	
	//------------------------------------------------------------------------
	void ReleaseOccupant()
	{
		m_Occupant = null;
		m_OccupantRplId = RplId.Invalid();
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (m_Occupant && m_Occupant != pUserEntity)
			return;
		
		if (!m_Occupant)
		{
			m_Occupant = pUserEntity;
			m_OccupantRplId = SCR_EntityHelper.EntityToRplId(pUserEntity);
			StartAction(pUserEntity);
		}
		else
		{
			ReleaseOccupant();
			StopAction(pUserEntity);
		}
	}
	//------------------------------------------------------------------------------------------------
	void StartAction(IEntity pUserEntity);
	//------------------------------------------------------------------------------------------------
	void StopAction(IEntity pUserEntity);
	//------------------------------------------------------------------------------------------------
	// Writes the occupant and returns true if a valid occupant was written
	protected bool SaveOccupant(ScriptBitWriter writer)
	{
		if (!m_Occupant)
		{
			writer.WriteBool(false);
			return false;
		}
		
		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(m_Occupant);
		if (!rplComp)
		{
			writer.WriteBool(false);
			return false;
		}

		writer.WriteBool(true);
		writer.WriteRplId(rplComp.Id());
		return true;
	}
	//------------------------------------------------------------------------------------------------
	// Reads the occupant and returns true if occupant data exists (but occupant may be null due to not being streamed in)
	protected bool LoadOccupant(ScriptBitReader reader)
	{
		ReleaseOccupant();
		bool occupantExists = false;
		
		reader.ReadBool(occupantExists);
		if (!occupantExists)
			return false;
		
		RplId rplId;
		reader.ReadRplId(rplId);
		m_OccupantRplId = rplId;
		m_Occupant = SCR_EntityHelper.RplIdToEntity(rplId);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnRplSave(ScriptBitWriter writer)
	{
		if (!super.OnRplSave(writer))
			return false;
		
		SaveOccupant(writer);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool OnRplLoad(ScriptBitReader reader)
	{
		if (!super.OnRplLoad(reader))
			return false;
		
		LoadOccupant(reader);
		return true;
	}
}