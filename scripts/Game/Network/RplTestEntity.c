[EntityEditorProps(category: "GameScripted/Test", description: "Testing entity for scripted replication", color: "0 0 255 255")]
class SCR_RplTestEntityClass: GenericEntityClass
{
};

class RplTestPropType
{
	int iVal;
	float fVal;
	bool bVal;

	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx hint, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 9);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 9);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx hint) 
	{
		return lhs.CompareSnapshots(rhs, 9);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(RplTestPropType prop, SSnapSerializerBase snapshot, ScriptCtx hint) 
	{
		return snapshot.Compare(prop.iVal, 4) 
			&& snapshot.Compare(prop.fVal, 4) 
			&& snapshot.Compare(prop.bVal, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(RplTestPropType prop, ScriptCtx hint, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(prop.iVal, 4);
		snapshot.SerializeBytes(prop.fVal, 4);
		snapshot.SerializeBytes(prop.bVal, 1);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx hint, RplTestPropType prop) 
	{
		snapshot.SerializeBytes(prop.iVal, 4);
		snapshot.SerializeBytes(prop.fVal, 4);
		snapshot.SerializeBytes(prop.bVal, 1);

		return true;
	}
	//################################################################################################
		
};

//------------------------------------------------------------------------------------------------
class SCR_RplTestEntity : GenericEntity
{
	[RplProp(condition: RplCondition.NoOwner)]
	private int m_iTest = 0;
	private int m_iTestLast = 0;
	
	float m_fdelay = 0;
	
	[RplProp(onRplName: "OnRpl_CustomProp")]
	ref RplTestPropType customProp = new RplTestPropType();
	
	void OnRpl_CustomProp()
	{
		Print("CUSTOM RPLPROP CHANGE1: " + customProp.iVal);
		Print("CUSTOM RPLPROP CHANGE2: " + customProp.fVal);
		Print("CUSTOM RPLPROP CHANGE3: " + customProp.bVal);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void NetTestRpc(int testNum)
	{
		Print("RPC RECEIVE: " + testNum.ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (Debug.KeyState(KeyCode.KC_P))
		{
			Debug.ClearKey(KeyCode.KC_P);
			int rndRPCNum = Math.RandomIntInclusive(0, 100);
			Rpc(NetTestRpc, rndRPCNum);
			Print("RPC SEND: " + rndRPCNum.ToString());
		}
		
		if (RplSession.Mode() == RplMode.Client) // Client
		{
			if (m_iTestLast != m_iTest)
			{
				Print("RPLPROP CHANGED: " + m_iTest);
				Print("CUSTOM RPLPROP CHANGED1: " + customProp.iVal);
				Print("CUSTOM RPLPROP CHANGED2: " + customProp.fVal);
				Print("CUSTOM RPLPROP CHANGED3: " + customProp.bVal);
			}
			
			m_iTestLast = m_iTest;
		}
		else
		{
			m_fdelay -= timeSlice;
			if (m_fdelay > 0)
				return;
			
			m_fdelay += 2;
			
			// Change the value server side
			if (RplSession.Mode() != RplMode.Client)
			{
				m_iTest = Math.RandomIntInclusive(0, 100);
				customProp.iVal = Math.RandomIntInclusive(0, 100);
				customProp.fVal = Math.RandomFloatInclusive(0, 100);
				customProp.bVal = customProp.iVal > 50;
			}
			
			Print("RPLPROP CHANGE: " + m_iTest);
			Print("CUSTOM RPLPROP CHANGE1: " + customProp.iVal);
			Print("CUSTOM RPLPROP CHANGE2: " + customProp.fVal);
			Print("CUSTOM RPLPROP CHANGE3: " + customProp.bVal);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RplTestEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RplTestEntity()
	{
	}
};
