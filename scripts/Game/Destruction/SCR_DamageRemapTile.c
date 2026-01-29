class SCR_DamageRemapTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		string slotName;
		ResourceName resourceName;
		source.Get("m_sMaterialSlotName", slotName);
		source.Get("m_sMaterialName", resourceName);
		if(resourceName)
		{
			array<string> strs = {};
			resourceName.Split("/", strs, true);
			resourceName = strs[strs.Count() - 1];
		}
		
		title = slotName + " | " + resourceName;
		return true;
	}
}