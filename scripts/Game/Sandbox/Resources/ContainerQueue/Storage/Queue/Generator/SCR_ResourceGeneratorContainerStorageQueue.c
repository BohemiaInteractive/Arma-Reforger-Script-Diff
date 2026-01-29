[BaseContainerProps()]
class SCR_ResourceGeneratorContainerStorageQueue : SCR_ResourceContainerStorageQueue<SCR_ResourceGenerator>
{
	//------------------------------------------------------------------------------------------------
	override int ComputePosition(SCR_ResourceContainer container, SCR_ResourceGenerator actor)
	{
		return m_iContainerCount;
	}
}