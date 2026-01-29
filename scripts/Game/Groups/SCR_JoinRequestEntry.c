class SCR_JoinRequestEntry : SCR_SubMenuBase
{
	protected int playerID;
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerID(int value)
	{
		playerID = value;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		return playerID;
	}
}