[BaseContainerProps()]
class SCR_DataCollectorStoreDataModule : SCR_DataCollectorModule
{
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeTick)
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StorePlayer(int playerId)
	{
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerId, false);
		
		//DO STUFF
	}
};