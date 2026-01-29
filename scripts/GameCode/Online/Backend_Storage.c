

// events here!
class BackendCallback_Storage extends BackendCallback
{

};


class Backend_Test_Storage
{
	ref BackendCallback_Storage m_cbx1;
	
	void Backend_Test_Storage()
	{
	}
	
	void ~Backend_Test_Storage()
	{
		m_cbx1 = null;
	}

	void Init()
	{
		// callback + structure
		m_cbx1 = new BackendCallback_Storage;
	}

	void Read()
	{
		Print("BackendTest() -> Read Archive from StorageApi");
		WorkshopApi wa = GetGame().GetBackendApi().GetWorkshop();
		
		WorkshopItem s1 = wa.GetByName("Mod Prototype");
		if( s1 )
		{
			s1.Download(m_cbx1, null);
			
			Print(s1.GetStateFlags());
		}
			
		Print( "Page: " + wa.GetPage() ) ;
		Print( "Page Count: " + wa.GetPageCount() ) ;
		Print( "Page Items: " + wa.GetPageItemCount() ) ;
		Print( "Total Items: " + wa.GetTotalItemCount() ) ;
		
		ref array<WorkshopItem> aItems = new array<WorkshopItem>;
		/*int itemCount = */wa.GetPageItems(aItems);
		
		// iterate items
		Print( "--- Listing page items ---" ) ;
		for( int i = 0; i < aItems.Count(); i++ )
		{
			// name
			Print( "aItems[" + i +  "]" );
			Print( aItems[i].Name() );
			
			// print results
			if (aItems[i].Author())
				Print( "  AuthorName:" + aItems[i].Author().Name() );
			else
				Print( "  AuthorName: none (unpublished mod)" );
		}
		
		
	}
	
	void Update()
	{
		Print("BackendTest() -> Write Archive to StorageApi");
		WorkshopItem s1 = GetGame().GetBackendApi().GetWorkshop().GetByName("Mod Prototype");
		if( s1 )
		{
//			s1.Upload();
			s1.Download(m_cbx1, null);

			Print(s1.GetStateFlags());
		}
	}
	
};

