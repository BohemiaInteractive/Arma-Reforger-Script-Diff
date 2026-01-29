[BaseContainerProps(configRoot: true)]
class SCR_BasePreviewConfig
{
	[Attribute()]
	ref array<ref SCR_BasePreviewEntry> m_aEntries;
	
	/*!
	Save this config to a file.
	Used for debugging purposes.
	\param resourceName File path
	\param fileName Alternative file path as string, not ResourceName
	\return True when saved
	*/
	bool SaveContainer(ResourceName resourceName, string fileName = string.Empty)
	{
		Resource resource = BaseContainerTools.CreateContainerFromInstance(this);
		if (!resource || !resource.IsValid())
			return false;

		return BaseContainerTools.SaveContainer(resource.GetResource().ToBaseContainer(), resourceName, fileName);
	}
};