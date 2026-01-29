[BaseContainerProps()]
class SCR_ResourceConsumerStoragePolicyGeneric : SCR_ResourceStoragePolicyBase<SCR_ResourceConsumer>
{
	//------------------------------------------------------------------------------------------------
	override bool IsStorageTypeValid(EResourceContainerStorageType storageType)
	{
		return true;
	}
}