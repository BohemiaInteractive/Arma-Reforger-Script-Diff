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
	proto external RplId GetId();
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
	proto external Managed GetItem(int idx);
	proto external void AddItem(notnull Managed item);
}

/*!
\}
*/
