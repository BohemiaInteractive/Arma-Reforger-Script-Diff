/*
File for various server browser callbacks to keep ServerBrowserMenuUI clean.
*/

//------------------------------------------------------------------------------------------------
enum EServerBrowserRequestResult
{
	SUCCESS 	= 0,
	ERROR 		= 1,
	TIMEOUT 	= 2,
};

//------------------------------------------------------------------------------------------------
//! Scripted room callback specific for single room 
class SCR_RoomCallback : BackendCallback
{
	protected Room m_Room;
	
	//------------------------------------------------------------------------------------------------
	void SetRoom(Room room)
	{
		m_Room = room;
	}
	
	//------------------------------------------------------------------------------------------------
	Room GetRoom()
	{
		return m_Room;
	}
}

//------------------------------------------------------------------------------------------------
//! Parameters for joining server
class RoomJoinData extends JsonApiStruct
{	
	string m_Password = "";
	
	// Ban related data
	string scope = "";
	string type = "";
	string reason = "";
	string issuer = "";
	int expiresAt = 0;
	int createdAt = 0;
	
	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{
		UnregV("scope");
		UnregV("type");
		UnregV("reason");
		UnregV("issuer");
		UnregV("expiresAt");
		UnregV("createdAt");
		
		if (!m_Password.IsEmpty())
			StoreString("password", m_Password);
	}
	
	override void OnExpand()
	{
		RegV("scope");
		RegV("type");
		RegV("reason");
		RegV("issuer");
		RegV("expiresAt");
		RegV("createdAt");
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPassword(string password) { m_Password = password; }
};