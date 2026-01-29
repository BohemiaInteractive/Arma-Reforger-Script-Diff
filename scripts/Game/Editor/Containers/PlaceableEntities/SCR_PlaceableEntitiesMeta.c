[BaseContainerProps()]
/** @ingroup Editor_Containers_Root
*/

/*!
Meta header representing placeable entity
*/
class SCR_PlaceableEntitiesMeta
{
	[Attribute()]
	private LocalizedString m_sDisplayName;
	
	string GetDisplayName()
	{
		return m_sDisplayName;
	}
};