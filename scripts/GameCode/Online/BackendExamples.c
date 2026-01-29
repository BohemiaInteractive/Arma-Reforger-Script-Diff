
class BackendExamples
{

	void ExecuteTest( int no )
	{
		Print("// ----------------------------------");
		Print("...executing BACKEND TEST #" + no);
	

		// #7 Authentication + Static Configuration GET
		if( no == 7 )
		{
			// save it
			Print("SAVING");
			OnlineRecord_Parent p1;
			p1 = new OnlineRecord_Parent(); // we character OBJ as sample too
			p1.Initialize(5);
			p1.PackToFile("ArrayTest.json");
			p1.PrintThis();
			Print("PACKED");

			// load it
			Print("LOADING");
			OnlineRecord_Parent p2 = new OnlineRecord_Parent();
			p2.LoadFromFile("ArrayTest.json");
			p2.PrintThis();
			Print("LOADED");
		

		}
	}

	
};