/*!
\defgroup EntityAPI Entity system
\{
*/

class EntityID: handle64
{
	static EntityID INVALID;
	static proto native EntityID FromInt(int hiBits, int loBits);
}

class NativeComponent: pointer
{
}

//! BoneMask == NULL means that all bits are set
class BoneMask: Managed
{
	int Mask[8]
}

//! Node global id is a hash of name of the node.
typedef int TNodeId;

typedef int TMeshIndex;

/*!
\}
*/
