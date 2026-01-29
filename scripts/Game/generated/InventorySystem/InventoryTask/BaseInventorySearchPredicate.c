/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem_InventoryTask
\{
*/

class BaseInventorySearchPredicate: Managed
{
	event protected array<typename> GetQueryComponents();
	event protected array<typename> GetQueryAttributes();
	/*
	will be called when serach query was satisfied, if you need to perform additional check on item - override this method
	queriedComponents and queriedAttributes arrays are guaranteed to contain provided types in Query arrays.
	eg. if  QueryComponentTypes[0] is YourComponent type then queriedComponents[0] guaranteed to be instance of YourComponent that belongs to item entity
	which means you can be sure that YourComponent.Cast(queriedComponents[0]) will always return a valid object
	same is applied to attributes from item's AttributeCollection
	*/
	event protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes) { return true; };
}

/*!
\}
*/
