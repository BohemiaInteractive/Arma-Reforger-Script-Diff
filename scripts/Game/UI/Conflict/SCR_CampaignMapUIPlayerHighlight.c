//------------------------------------------------------------------------------
class SCR_CampaignMapUIPlayerHighlight : SCR_MapUIElement
{
	protected vector position;
	
	override vector GetPos()
	{
		return position;
	}
	
	void SetPos(vector v)
	{
		position = v;
	}
};
