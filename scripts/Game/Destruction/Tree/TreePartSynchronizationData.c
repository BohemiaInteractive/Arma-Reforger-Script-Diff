class SCR_TreePartSynchronizationData
{
	int m_iTreePartIndex;
	float m_fQuaternion[4];
	vector m_vPosition;
	vector m_vVelocity;
	vector m_vAngularVelocity;
	EntityID m_TreeID;
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 64);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 64);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{	
		return lhs.CompareSnapshots(rhs, 64);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_TreePartSynchronizationData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return snapshot.Compare(prop.m_iTreePartIndex, 4)
			&& snapshot.Compare(prop.m_fQuaternion[0], 4)
			&& snapshot.Compare(prop.m_fQuaternion[1], 4)
			&& snapshot.Compare(prop.m_fQuaternion[2], 4)
			&& snapshot.Compare(prop.m_fQuaternion[3], 4)
			&& snapshot.Compare(prop.m_vPosition[0], 4)
			&& snapshot.Compare(prop.m_vPosition[1], 4)
			&& snapshot.Compare(prop.m_vPosition[2], 4)
			&& snapshot.Compare(prop.m_vVelocity[0], 4)
			&& snapshot.Compare(prop.m_vVelocity[1], 4)
			&& snapshot.Compare(prop.m_vVelocity[2], 4)
			&& snapshot.Compare(prop.m_vAngularVelocity[0], 4)
			&& snapshot.Compare(prop.m_vAngularVelocity[1], 4)
			&& snapshot.Compare(prop.m_vAngularVelocity[2], 4)
			&& snapshot.Compare(prop.m_TreeID, 8);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_TreePartSynchronizationData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(prop.m_iTreePartIndex, 4);
		snapshot.SerializeBytes(prop.m_fQuaternion[0], 4);
		snapshot.SerializeBytes(prop.m_fQuaternion[1], 4);
		snapshot.SerializeBytes(prop.m_fQuaternion[2], 4);
		snapshot.SerializeBytes(prop.m_fQuaternion[3], 4);
		snapshot.SerializeBytes(prop.m_vPosition[0], 4);
		snapshot.SerializeBytes(prop.m_vPosition[1], 4);
		snapshot.SerializeBytes(prop.m_vPosition[2], 4);
		snapshot.SerializeBytes(prop.m_vVelocity[0], 4);
		snapshot.SerializeBytes(prop.m_vVelocity[1], 4);
		snapshot.SerializeBytes(prop.m_vVelocity[2], 4);
		snapshot.SerializeBytes(prop.m_vAngularVelocity[0], 4);
		snapshot.SerializeBytes(prop.m_vAngularVelocity[1], 4);
		snapshot.SerializeBytes(prop.m_vAngularVelocity[2], 4);
		snapshot.SerializeBytes(prop.m_TreeID, 8);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_TreePartSynchronizationData prop) 
	{
		snapshot.SerializeBytes(prop.m_iTreePartIndex, 4);
		float temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_fQuaternion[0] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_fQuaternion[1] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_fQuaternion[2] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_fQuaternion[3] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vPosition[0] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vPosition[1] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vPosition[2] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vVelocity[0] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vVelocity[1] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vVelocity[2] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vAngularVelocity[0] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vAngularVelocity[1] = temp;
		snapshot.SerializeBytes(temp, 4);
		prop.m_vAngularVelocity[2] = temp;
		snapshot.SerializeBytes(prop.m_TreeID, 8);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_TreePartSynchronizationData()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TreePartSynchronizationData()
	{
		
	}
};