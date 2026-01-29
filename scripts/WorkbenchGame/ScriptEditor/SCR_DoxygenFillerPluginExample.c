#ifdef DEBUG
class SCR_DoxygenFillerPlugin_TestA
{
	// no return value, no comment, no separator
	void Method1();

	int Method2(); // comment

	//------------------------------------------------------------------------------------------------
	bool Method3(); // Not documented, with separator

	//! Already documented, without separator
	int GetNumberOfFingers();

	//------------------------------------------------------------------------------------------------
	//! Already documented, with separator
	int Method6();

	protected int Method7();		// protected
	private int Method8();			// private
	int Method9();					// override (in child)

	int GetMethod();				// getter
	void SetMethod(int value);		// setter

	void VoidMethodWithMultipleParams(bool ean, out int eger, inout float ingPoint, notnull array<string> test);
	int IntMethodWithMultipleParams(bool ean, out int eger, inout float ingPoint, notnull array<string> test);
	array<ref SCR_Faction> RefMethodWithMultipleParams(inout notnull array<ref SCR_Faction> test);

	void SCR_DoxygenFillerPlugin_TestA(); // constructor
	void ~SCR_DoxygenFillerPlugin_TestA(); // destructor
}

class SCR_DoxygenFillerPlugin_TestB : SCR_DoxygenFillerPlugin_TestA
{
	override int Method9();			// override
}

class SCR_DoxygenFillerPlugin_TestC
{
	array<ref SCR_Faction> Copy1(notnull array<ref SCR_Faction> factions, map<int, string> useless);
	map<int, ref SCR_Faction> Copy2(array<ref SCR_Faction> factions, map<int, string> useless);
	void SetIsPlayable1(bool isPlayable, bool killPlayersIfNotPlayable = false);

	//------------------------------------------------------------------------------------------------
	[Obsolete()]
	void AnObsoleteMethod()
	{
		Print("OK");
	}

	//------------------------------------------------------------------------------------------------
	// Called everywhere, used to generate initial data for this faction
	void InitializeFaction();

	int GetOrder();

	void ~SCR_DoxygenFillerPlugin_TestC();
	void SetIsPlayable2(bool isPlayable, bool killPlayersIfNotPlayable = false);
	void SCR_DoxygenFillerPlugin_TestC(int count);
}
#endif // DEBUG
