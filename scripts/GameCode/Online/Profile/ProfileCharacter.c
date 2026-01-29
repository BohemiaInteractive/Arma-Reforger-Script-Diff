

// Array Item
class OnlineRecord_Item extends JsonApiStruct
{
	string m_String;
	float m_Float;
	
	void OnlineRecord_Item()
	{
		// register variables for auto pack/ expand
		RegV("m_String");
		RegV("m_Float")
	}
	
	void Initialize( int count )
	{
		m_String = "something";
		m_Float = 1 + count;
	}
	
	void PrintThis()
	{
		Print("m_String=" + m_String);
		Print("m_Float=" + m_Float);
	}
};

// Parent Object
class OnlineRecord_Parent extends JsonApiStruct
{
	string m_Message;
	float m_AnotherFloat;
	
	protected ref OnlineRecord_Item m_SingleItem; // object
	
	protected ref array<ref OnlineRecord_Item> m_aItems; // arrays of objects
	
	void OnlineRecord_Parent()
	{
		// register variables for auto pack/ expand
		RegV("m_Message");
		RegV("m_AnotherFloat");
		RegV("m_SingleItem");
		RegV("m_aItems");
	}

	void Initialize( int count )
	{
		m_Message = "The Ring has awoken, it�s heard its master�s call.";
		m_AnotherFloat = 256;
		
		m_SingleItem = new OnlineRecord_Item();
		m_SingleItem.Initialize(64);
	
		m_aItems = new array<ref OnlineRecord_Item>();
		for( int i = 0; i < count; i++ )
		{
			ref OnlineRecord_Item rn = new OnlineRecord_Item();
			rn.Initialize(i);		
			m_aItems.Insert(rn);
		}
	}

	void PrintThis()
	{
		Print(" -- MASTER OBJECT -- ");
		Print("m_Message=" + m_Message);
		Print("m_AnotherFloat=" + m_AnotherFloat);

		m_SingleItem.PrintThis();

		if( m_aItems )
		{
			for( int i = 0; i < m_aItems.Count(); i++ )
				m_aItems[i].PrintThis();
		}
		
		Print(" -- END -- ");
	}
	

	/**
	\brief Event called when pending store operation is finished - callback from JsonApiHandle before handle release
	*/
	override void OnSuccess( int errorCode )
	{
	}	

};

