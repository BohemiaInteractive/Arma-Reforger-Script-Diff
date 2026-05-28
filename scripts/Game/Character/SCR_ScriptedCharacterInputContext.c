class SCR_ScriptedCharacterInputContext
{
	int m_iLoiteringType = -1;
	bool m_bLoiteringShouldHolsterWeapon;
	bool m_bLoiteringShouldAlignCharacter;
	bool m_bLoiteringRootMotion;
	bool m_bLoiteringDisablePlayerInput;
	vector m_mLoiteringPosition[4];
	IEntity m_loiterEntity;
	RplId m_loiterEntityRplId = RplId.Invalid();
	ref SCR_LoiterCustomAnimData m_CustomAnimData;
	
	//------------------------------------------------------------------------
	
	IEntity GetLoiterEntity()
	{
		if (m_loiterEntity)
			return m_loiterEntity;
		
		return SCR_EntityHelper.RplIdToEntity(m_loiterEntityRplId);
	}
	
	//------------------------------------------------------------------------
	void SetLoiteringEntity(IEntity ent)
	{
		m_loiterEntity = ent;
		m_loiterEntityRplId = SCR_EntityHelper.EntityToRplId(m_loiterEntity);
	}
	
	//------------------------------------------------------------------------
	void SetLoiteringEntityId(RplId id)
	{
		m_loiterEntityRplId = id;
		m_loiterEntity = SCR_EntityHelper.RplIdToEntity(id);
	}
	
	//------------------------------------------------------------------------
	void OnRplLoad(ScriptBitReader r)
	{
		r.ReadInt(m_iLoiteringType);
		r.ReadBool(m_bLoiteringShouldHolsterWeapon);
		r.ReadBool(m_bLoiteringShouldAlignCharacter);
		r.ReadBool(m_bLoiteringRootMotion);
		r.ReadBool(m_bLoiteringDisablePlayerInput);
		
		vector position;
		r.ReadVector(position);
		
		vector angles;
		r.ReadVector(angles);
		
		vector rotation[3];
		Math3D.AnglesToMatrix(angles, rotation);
		m_mLoiteringPosition[3] = position;
		m_mLoiteringPosition[0] = rotation[0];
		m_mLoiteringPosition[1] = rotation[1];
		m_mLoiteringPosition[2] = rotation[2];
		
		RplId rplId;
		r.ReadRplId(rplId);
		
		SetLoiteringEntityId(rplId);
		
		bool hasCustomData;
		r.ReadBool(hasCustomData);
		if (hasCustomData)
		{
			m_CustomAnimData = new SCR_LoiterCustomAnimData();
			m_CustomAnimData.OnRplLoad(r);
		}
		else
		{
			m_CustomAnimData = SCR_LoiterCustomAnimData.Default;
		}
	}
	//------------------------------------------------------------------------
	
	void OnRplSave(ScriptBitWriter w)
	{
		w.WriteInt(m_iLoiteringType);
		w.WriteBool(m_bLoiteringShouldHolsterWeapon);
		w.WriteBool(m_bLoiteringShouldAlignCharacter);
		w.WriteBool(m_bLoiteringRootMotion);
		w.WriteBool(m_bLoiteringDisablePlayerInput);
		w.WriteVector(m_mLoiteringPosition[3]);
		w.WriteVector(Math3D.MatrixToAngles(m_mLoiteringPosition));
		w.WriteRplId(SCR_EntityHelper.EntityToRplId(m_loiterEntity));
		
		if (m_CustomAnimData && !m_CustomAnimData.IsDefault())
		{
			w.WriteBool(true);
			m_CustomAnimData.OnRplSave(w);
		}
		else
		{
			w.WriteBool(false);
		}
		
	}
}
