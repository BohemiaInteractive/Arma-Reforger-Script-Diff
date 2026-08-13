/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Replication
\{
*/

sealed class RplNode: pointer
{
	//! \deprecated Use RplNode.GetMainItemId() instead.
	[Obsolete("Use RplNode.GetMainItemId() instead.")]
	RplId GetId() { return this.GetMainItemId(); }

	proto external RplRole GetRole();
	proto external bool IsOwner();
	proto external bool IsLocked();
	proto external void SetParent(RplNode newParent, bool unusedAndDeprecated = true);
	proto external RplNode GetParent();
	proto external RplNode GetRootParent();
	proto external RplNode GetChildren();
	proto external RplNode GetSibling();
	proto external int GetItemCount();
	proto external RplId GetItemId(int idx);
	proto external RplId GetMainItemId();
	proto external Managed GetItem(int idx);
	proto external Managed GetMainItem();
	/*!
	\returns True if the item was added successfully, false otherwise.
	\warning Can be used only before the RplNode is inserted into replication.
	*/
	proto external bool AddItem(notnull Managed item);
}

/*!
\}
*/
