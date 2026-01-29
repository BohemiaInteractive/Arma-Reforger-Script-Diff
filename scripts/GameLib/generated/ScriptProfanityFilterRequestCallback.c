/*
===========================================
Do not modify, this script is generated
===========================================
*/

class ScriptProfanityFilterRequestCallback: ProfanityFilterRequestCallback
{
	event void OnFilteredResult() { array<string> outTexts = new array<string>(); GetTexts(outTexts); Print(outTexts); };
}
