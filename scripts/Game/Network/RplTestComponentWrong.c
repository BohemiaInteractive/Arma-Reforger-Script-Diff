[ComponentEditorProps(category: "GameScripted/Test", description: "Test component showcasing doing the replication from script the WRONG way")]
class SCR_RplTestComponentWrongClass : ScriptComponentClass
{
}

class SCR_RplTestComponentWrong : ScriptComponent
{
	[RplProp(condition: RplCondition.NoOwner)]
	private int m_iTest = 0;
	private int m_iTestLast = 0;
	
	float m_fdelay = 0;
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void NetTestRpc(int testNum)
	{
		Print("RPC RECEIVE: " + testNum, LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (Debug.KeyState(KeyCode.KC_P))
		{
			Debug.ClearKey(KeyCode.KC_P);
			int rndRPCNum = Math.RandomIntInclusive(0, 100);
			// WRONG:
			// When performed on server, this is broadcast to everyone.
			// However when executed on client, this is only performed on the client.
			Rpc(NetTestRpc, rndRPCNum);
			Print("RPC SEND: " + rndRPCNum, LogLevel.NORMAL);
		}
		
		/*
			WRONG:
			Never never never (unless you really have to).
			If possible, never decide which RPC to call be checking for wheter you're
			a client or not.
			In general, if you need to write this condition it means your thought process is
			not the best one and you should think things through again.
			Instead, use proper attributes for the RPC. It takes a little bit of getting
			used to, however, you'll end up with code which is cleaner and easier to understand.
			This design actually enforces the right way of thinking on you.
		*/
		if (RplSession.Mode() == RplMode.Client) // Client
		{
			/*
				WRONG:
				Never never never.
				Always check for property changes on client using a callback.
				You won't need to remeber the previous value + you'll avoid
				cases where OnFrame isn't called for whatever reason (sometimes,
				simulation of certain entites might turn off EOnFrame for proxies).
				
				In this case, every time m_iTest changes on sever, OnTestChanged
				is called on client. The moment it's called, you know the value has
				already been updated to a new one. If you want to keep the old one (
				which you usually don't) you need to store it e.g. m_iTestLast or
				in an array depending on your needs.
			*/
			if (m_iTestLast != m_iTest)
				Print("RPLPROP CHANGED: " + m_iTest, LogLevel.NORMAL);

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
				Replication.BumpMe();
			}
						
			Print("RPLPROP CHANGE: " + m_iTest);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RplTestComponentWrong(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetEventMask(ent, EntityEvent.FRAME);
		ent.SetFlags(EntityFlags.ACTIVE, true);
	}
}
