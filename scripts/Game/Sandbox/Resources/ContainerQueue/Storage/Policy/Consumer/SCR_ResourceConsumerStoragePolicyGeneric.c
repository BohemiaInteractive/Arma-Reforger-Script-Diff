[BaseContainerProps()]
class SCR_ResourceConsumerStoragePolicyGeneric : SCR_ResourceStoragePolicyBase
{
	//------------------------------------------------------------------------------------------------
	override bool IsStorageTypeValid(EResourceContainerStorageType storageType)
	{
		return true;
	}
}