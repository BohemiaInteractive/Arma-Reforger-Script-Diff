[BaseContainerProps()]
class SCR_ResourceGeneratorStoragePolicyGeneric : SCR_ResourceStoragePolicyBase<SCR_ResourceGenerator>
{
	//------------------------------------------------------------------------------------------------
	override bool IsStorageTypeValid(EResourceContainerStorageType storageType)
	{
		return true;
	}
}