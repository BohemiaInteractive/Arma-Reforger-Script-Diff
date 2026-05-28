[BaseContainerProps()]
class SCR_ResourceGeneratorStoragePolicyGeneric : SCR_ResourceStoragePolicyBase
{
	//------------------------------------------------------------------------------------------------
	override bool IsStorageTypeValid(EResourceContainerStorageType storageType)
	{
		return true;
	}
}