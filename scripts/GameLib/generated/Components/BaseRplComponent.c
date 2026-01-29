/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseRplComponentClass: GenericComponentClass
{
}

/*!
* BaseRplComponent
* ================
* A convenience wrapper for RplNode in the ECS. Its role is to serve as an
* accessor to the nodes data as well as collector of replicated items within
* the entity component hierarchy.
*
* During its initialization it traverses the hierarchy and looks for replicated
* items, collects them and registers into the replication pipeline.
*
* Also listens to changes within the entity component hierarchy and calls the
* appropriate replication methods to replicate the hierarchy changes to
* replicas.
*
* All of its features can be disabled, overridden or handled manually. For the
* most fine grained access to data its advisable to use the RplNode itself.
*
*
* Initialization
* --------------
* The RplNode provides a accurate role and ownership from the time of its
* construction.
*
* Then in EOnInit the InsertToReplication method is called, the hierarchy is
* traversed and replicated items are collected. The component looks for another
* BaseRplComponent instance in the direction upwards in the entity component
* hierarchy. If such instance is found it will become responsible for
* registration of this child instance.
*
* The BaseRplComponent has a low priority setup to make it possibly the last
* component to be initialized. If any of the components within the hierarchy
* would spawn children this way they would get registered into the pipeline
* too.
*
* WARNING: If anything gets spawned after the EOnInit is called on the
* BaseRplComponent instance it won't get registered at all.
*
* If the user chooses to disable the SELF_INSERT mechanism he will become
* ultimately responsible for calling the InsertToReplication method himself.
*
* Hierarchy changes
* -----------------
* The component handles the hierarchy changes automatically by default. Its a
* convenience function that should be turned off for more complex usage.
*
* WARNING: Not every replicated entity can change hierarchy at will. It has to
* be an owner of an RplNode (BaseRplComponent) as the Nodes are immutable
* collections in replication pipeline. There is no such mechanism that will
* prevent the user from doing so but it will cause a serious harm and
* malfunction.
*
* Destruction
* -----------
* Upon destruction the component will unregister its contents from replication
* pipeline automatically.
*/
class BaseRplComponent: GenericComponent
{
	proto external RplNode GetNode();
	/*!
	Transfers ownership of the hierarchy to given connection.
	\param newOwner Identity of new owner.
	*/
	proto external void Give(RplIdentity newOwner);
	/*!
	Returns replication id of the RplComponent.
	*/
	proto external RplId Id();
	/*!
	Returns replication id of the registered entity child.
	*/
	proto RplId ChildId(notnull Class item);
	/*!
	Returns current role of the component and its hierarchy within the
	replication.
	*/
	proto external RplRole Role();
	/*!
	If true we are the owner of the hierarchy
	*/
	proto external bool IsOwner();
	/*!
	If true we are a proxy
	*/
	proto external bool IsProxy();
	/*!
	If true we are both owner and authority (IsOwner() && !IsProxy())
	*/
	proto external bool IsMaster();
	/*!
	If true we are both owner and proxy (IsOwner() && IsProxy())
	*/
	proto external bool IsOwnerProxy();
	/*!
	If true we are a ghost item - not owned while being a proxy (!IsOwner() && IsProxy())
	*/
	proto external bool IsRemoteProxy();
	/*!
	Registers additional item into the set that will be inserted into replication
	when InsertToReplication is called.
	This can be used if you need to insert some custom items into your replicated node.

	\warning Can be used only before the RplNode is inserted into the replication
	*/
	proto external EItemInsertionResult InsertItem(Managed instance);
	/*!
	Inserts the component along with the entity and its children into the
	replication. Keep in mind that only items with replication code will be
	registered and receive the RplId.
	*/
	proto external bool InsertToReplication(RplSchedulerInsertionCtx ctx = NULL);
	/*!
	Returns the entity owning the component.
	*/
	proto external IEntity GetEntity();
	/*!
	Marks the item as released from replication.
	Items marked this way won't be physically deleted once removed from replication.
	When released, proxies will have to delete their entities manually after they're removed from replication.

	\warning Only has effect if called by the authority
	*/
	proto external void ReleaseFromRpl();
	/*!
	True if entity is marked as to-be-manually-deleted by proxies after removed from replication
	*/
	proto external bool IsReleasedFromRpl();
	/*
	Enable or disable spatial relevancy of a node.
	Spatial relevancy is not enforced in a hierarchy, meaning if a root node is not
	in spatial map, and a spatial node is attached to it, it will not be in spatial
	map.
	*/
	proto external void EnableSpatialRelevancy(bool enable);
	/*!
	Enable streaming for a specific entity.
	\param enable If true streaming is enabled.
	*/
	proto external void EnableStreaming(bool enable);
	/*!
	Returns true if the node is self-inserting (it inserts itself to replication in BaseRplComponent's EOnInit)
	*/
	proto external bool IsSelfInserted();
}

/*!
\}
*/
