/*!
\addtogroup Replication
\{
*/

class RplSessionCallbacks
{
	void EOnBegan();
	void EOnEnded();
	void EOnFailed(string msg);
	void EOnConnected(RplIdentity identity);
	void EOnDisconnected(RplIdentity identity);
}

//! \deprecated Use RplStateOverride instead.
enum ERplStateOverride
{
	None = RplStateOverride.None,
	Static = RplStateOverride.None,
	Dynamic = RplStateOverride.Runtime,
}

/*!
\}
*/
