/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class DecoratorScripted: Decorator
{
	static protected override string GetOnHoverDescription() { return "Decorator Scripted: Allows creating scripted decorators"; }
	static protected override bool VisibleInPalette() { return true; }

	event protected bool TestFunction(AIAgent owner);
	event protected void OnInit(AIAgent owner);
	event protected string GetNodeMiddleText();
}

/*!
\}
*/
