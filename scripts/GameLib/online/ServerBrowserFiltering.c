//! base classes for filtering in server browser



class GetRoomsIds extends JsonApiStruct
{
	ref array<string> ids;
	
	void GetRoomsIds()
	{
		ids = new array<string>;
		RegV("ids");
	}
}

class RoomFilterBase : JsonApiStruct
{
	bool includePing = false;
	bool ownedOnly = false;
}