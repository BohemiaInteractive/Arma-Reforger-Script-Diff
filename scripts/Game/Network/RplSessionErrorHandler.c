//-----------------------------------------------------------------------------
class RplSessionErrorHandler : RplSessionCallbacks
{
	protected const string ERROR_GROUP = "REPLICATION";
	
	//-----------------------------------------------------------------------------
	void RplSessionErrorHandler()
	{
		RplSession.RegisterCallbacks(this);
	}
	
	//-----------------------------------------------------------------------------
	override void EOnFailed(string msg)
	{
		SCR_KickDialogs.CreateKickErrorDialog(msg, ERROR_GROUP);
	}

};
