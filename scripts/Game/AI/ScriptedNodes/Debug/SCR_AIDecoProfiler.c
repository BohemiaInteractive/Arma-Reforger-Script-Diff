class SCR_AIDecoProfiler : DecoratorScripted
{
	static override string GetOnHoverDescription() { return "Decorator used in external script profiler for profiling performance of trees."; }
	static override bool VisibleInPalette() { return true; }

	override bool TestFunction(AIAgent owner)	
	{
		return true;	
	}	
}