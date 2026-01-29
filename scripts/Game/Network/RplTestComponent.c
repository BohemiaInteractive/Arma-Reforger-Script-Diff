[ComponentEditorProps(category: "GameScripted/Test", description: "Test component showcasing doing the replication from script the RIGHT way")]
class SCR_RplTestComponentClass : ScriptComponentClass
{
}

class SCR_RplTestComponent : ScriptComponent
{
	protected ChimeraCharacter m_CharacterOwner = null;
	private RplComponent m_RplComponent = null;
	
	// NoOwner - executed everywhere but on the owner. Note both client and server can be owners.
		// Thus, when the client is owner, this won't be called for it.
	// onRplName: "OnTestChanged" - if the target is right,
		// everytime the value changes OnTestChanged is executed
	[RplProp(condition: RplCondition.NoOwner, onRplName: "OnTestChanged")]
	private int m_iTest = 0;
	
	float m_fdelay = 0;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] testNum
	void Do_TestRpc(int testNum)
	{
		Print("RPC TestRpc EXECUTED: " + testNum, LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Send request to the server to print a message everywhere
	//! \param[in] testNum
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_TestRpc(int testNum)
	{
		Do_TestRpc(testNum);
		Rpc(RpcDo_TestRpc, testNum);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Broadcast.
	//! When called from server this is executed everywhere but on the server.
	//! When called on client, this is executed only on the client.
	//!
	//! NOTE:
	//! The caller of the RPC needs to provide exectly one integral argument "testNum".
	//! Note, max. 16 args are possible in script. This restriction applies universally not just
	//! to RPCs but for all script methods in general
	//! \param[in] testNum
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]	
	void RpcDo_TestRpc(int testNum)
	{
		Print("RPC NetTestRpc EXECUTED: " + testNum, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Server-only execution
	//! \param[in] timeSlice
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ChangePropValue(float timeSlice)
	{
		m_fdelay -= timeSlice;
		if (m_fdelay > 0)
			return;
		
		m_fdelay += 2;
		
		// Change the value server side
		// This will trigger OnTestChanged on clients
		m_iTest = Math.RandomIntInclusive(0, 100);
		Replication.BumpMe();
		Print("RPLPROP m_iTest CHANGED on SERVER: " + m_iTest, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	// Called on client everytime m_iTest changes value. No need to use m_iTestLast
	// unless you really need to know the previous value
	void OnTestChanged()
	{
		Print("RPLPROP m_iTest CHANGED on CLIENT: " + m_iTest, LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Do_TestFire()
	{
		Print("RPC TestFire EXECUTED", LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_TestFire()
	{
		// You might want to do some security checks here.
		// E.g., could the shot have been fired?
		// if (!...)
		// 	return;
		
		// Assuming the server is the authority here, we perform the action
		// right away.
		// If it weren't the case, we'd need to do the opposite condition we
		// did before calling the RPC which in our would result in:
		// if (m_RplComponent && !m_RplComponent.IsOwnerProxy())
		//   Do_TestFire();
		// Otherwise, Do_TestFire would be performed twice in some cases.
		// However, we design our code with security in mind and want the server
		// be the authority. Therefor, it's enough to have:
		Do_TestFire();
		
		Rpc(RpcDo_TestFire);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast, RplCondition.NoOwner)]
	void RpcDo_TestFire()
	{
		Do_TestFire();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.FRAME | EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_CharacterOwner = ChimeraCharacter.Cast(owner);
		if (m_CharacterOwner)
			m_RplComponent = RplComponent.Cast(m_CharacterOwner.FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (Debug.KeyState(KeyCode.KC_P))
		{
			Debug.ClearKey(KeyCode.KC_P);
			int rndRPCNum = Math.RandomIntInclusive(0, 100);
			// Print first
			Print("Requesting TestRpc: " + rndRPCNum, LogLevel.NORMAL);
			// Execute second! Otherwise, when calling this on server,
			// you'd first print you changed the value and only then
			// you'd print your requested a change.
			Rpc(RpcAsk_TestRpc, rndRPCNum);	
		}
		
		// Ask the server to update the property value
		Rpc(RpcAsk_ChangePropValue, timeSlice);
		
		// There might be times when you want some local effect be performed
		// right away. E.g., you start shooting and want to hide the network
		// latency (request + response could take 100+ ms and you would notice
		// this immediatelly).
		// In this case, if we assume the server has the authority (which should
		// always be the case to keep the game secure), I recommend doing:
		if (m_RplComponent && m_RplComponent.IsOwnerProxy())
			Do_TestFire(); // local effect right away
		// Ask the server to perform the action and if possible, broadcast it
		// to everybody but the owner
		Rpc(RpcAsk_TestFire);
		
		// IMPORTANT:
		// ONLY OWNERS CAN PERFORM RPCs! Calling RPCs from a non-owner equals
		// doing nothing.
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_RplTestComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{			
	}
}
