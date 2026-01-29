[BaseContainerProps(configRoot: true)]
class SCR_ResourceConsumerMilitaryBase : SCR_ResourceConsumer
{
	protected ref ScriptInvoker<SCR_ResourceConsumerMilitaryBase, SCR_ResourceContainer> m_OnBaseContainerRegistered;
	protected ref ScriptInvoker<SCR_ResourceConsumerMilitaryBase, SCR_ResourceContainer> m_OnBaseContainerUnregistered;
	
	//------------------------------------------------------------------------------------------------
	//! \return An invoker that gets called when a container gets registered on this consumer.
	ScriptInvoker<SCR_ResourceConsumerMilitaryBase, SCR_ResourceContainer> GetOnBaseContainerRegistered()
	{
		if (!m_OnBaseContainerRegistered)
			m_OnBaseContainerRegistered = new ScriptInvoker<SCR_ResourceConsumerMilitaryBase, SCR_ResourceContainer>();
		
		return m_OnBaseContainerRegistered;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return An invoker that gets called when a container gets unregistered on this consumer.
	ScriptInvoker<SCR_ResourceConsumerMilitaryBase, SCR_ResourceContainer> GetOnBaseContainerUnregistered()
	{
		if (!m_OnBaseContainerUnregistered)
			m_OnBaseContainerUnregistered = new ScriptInvoker<SCR_ResourceConsumerMilitaryBase, SCR_ResourceContainer>();
		
		return m_OnBaseContainerUnregistered;
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnContainerRegistered(notnull SCR_ResourceContainer container)
	{
		super.OnContainerRegistered(container);
		
		if (m_OnBaseContainerRegistered)
			m_OnBaseContainerRegistered.Invoke(this, container);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnContainerUnregistered(notnull SCR_ResourceContainer container)
	{
		super.OnContainerUnregistered(container);
		
		if (m_OnBaseContainerUnregistered)
			m_OnBaseContainerUnregistered.Invoke(this, container);
	}
}