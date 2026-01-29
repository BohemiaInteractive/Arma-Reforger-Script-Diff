[BaseContainerProps(), SCR_ResourceStorageTypeTitle()]
class SCR_ResourceConsumerStoragePolicy : SCR_ResourceStoragePolicyBase<SCR_ResourceConsumer>
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
