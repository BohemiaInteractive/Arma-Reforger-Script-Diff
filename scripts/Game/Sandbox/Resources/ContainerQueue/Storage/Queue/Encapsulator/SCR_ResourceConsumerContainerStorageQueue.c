[BaseContainerProps()]
class SCR_ResourceEncapsulatorContainerStorageQueue : SCR_ResourceContainerStorageQueue<SCR_ResourceEncapsulator>
{
	//------------------------------------------------------------------------------------------------
	//! Registers a container into the queue.
	//! \param container The container to be registered into the queue.
	//! \return Returns the position of the container in the queue.
	override int RegisterContainer(notnull SCR_ResourceContainer container, notnull SCR_ResourceEncapsulator actor)
	{
		return InsertContainerAt(container, ComputePosition(container, actor));
	}
	
	//------------------------------------------------------------------------------------------------
	override int ComputePosition(SCR_ResourceContainer container, SCR_ResourceEncapsulator actor)
	{
		return GetContainerCount();
	}
}