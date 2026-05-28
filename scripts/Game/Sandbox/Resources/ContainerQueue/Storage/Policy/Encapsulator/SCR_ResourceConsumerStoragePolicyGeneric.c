[BaseContainerProps()]
class SCR_ResourceEncapsulatorStoragePolicyGeneric : SCR_ResourceStoragePolicyBase
{
	//------------------------------------------------------------------------------------------------
	override bool IsStorageTypeValid(EResourceContainerStorageType storageType)
	{
		return true;
	}
}