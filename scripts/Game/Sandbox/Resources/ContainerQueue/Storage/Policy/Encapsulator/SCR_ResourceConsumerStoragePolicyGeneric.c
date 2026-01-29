[BaseContainerProps()]
class SCR_ResourceEncapsulatorStoragePolicyGeneric : SCR_ResourceStoragePolicyBase<SCR_ResourceEncapsulator>
{
	//------------------------------------------------------------------------------------------------
	override bool IsStorageTypeValid(EResourceContainerStorageType storageType)
	{
		return true;
	}
}