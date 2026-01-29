[ComponentEditorProps(category: "GameScripted/Editor", description: "Transmitting info about dedicated server. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_ServerInfoEditorComponentClass : SCR_BaseEditorComponentClass
{
};
class SCR_ServerInfoEditorComponent : SCR_BaseEditorComponent
{
	[Attribute("1", desc: "How often is info about serevr broadcasted.")]
	protected float m_fInterval;
	
	protected ref ScriptInvoker m_OnReceive = new ScriptInvoker();

	ScriptInvoker GetOnReceive()
	{
		return m_OnReceive;
	}
	
	protected void Transmit()
	{
		int fps = System.GetFPS();
		int memoryKB = System.MemoryAllocationKB();
		int tickCount = System.GetTickCount();
		
		Rpc(Receive, fps, memoryKB, tickCount);
	}	
	[RplRpc(RplChannel.Unreliable, RplRcver.Owner)]
	protected void Receive(int fps, int memoryKB, int tickCount)
	{
		m_OnReceive.Invoke(fps, memoryKB, tickCount);
	}
	
	override protected void EOnEditorActivateServer()
	{
		GetGame().GetCallqueue().CallLater(Transmit, m_fInterval * 1000, true);
	}
	override protected void EOnEditorDeactivateServer()
	{
		GetGame().GetCallqueue().Remove(Transmit);
	}
};
