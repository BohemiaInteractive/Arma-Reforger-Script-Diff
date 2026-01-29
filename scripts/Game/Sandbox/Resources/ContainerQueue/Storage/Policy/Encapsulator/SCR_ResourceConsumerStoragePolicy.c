[BaseContainerProps(), SCR_ResourceStorageTypeTitle()]
class SCR_ResourceEncapsulatorStoragePolicy : SCR_ResourceStoragePolicyBase<SCR_ResourceEncapsulator>
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Resource consumption source order.", enums: ParamEnumArray.FromEnum(EResourceContainerStorageType))]
	protected EResourceContainerStorageType m_eStorageType;
	
	//------------------------------------------------------------------------------------------------
	EResourceContainerStorageType GetStorageType()
	{
		return m_eStorageType;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsStorageTypeValid(EResourceContainerStorageType storageType)
	{
		return m_eStorageType == storageType;
	}
};
