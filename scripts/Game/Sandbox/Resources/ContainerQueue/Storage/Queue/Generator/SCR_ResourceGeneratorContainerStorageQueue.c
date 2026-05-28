[BaseContainerProps()]
class SCR_ResourceGeneratorContainerStorageQueue : SCR_ResourceContainerStorageQueue
{
	//------------------------------------------------------------------------------------------------
	override int ComputePosition(SCR_ResourceContainer container, SCR_ResourceInteractor actor)
	{
		return m_iContainerCount;
	}
}