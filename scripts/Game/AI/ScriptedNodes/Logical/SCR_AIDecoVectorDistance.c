class SCR_AIDecoVectorDistance : DecoratorScripted
{
	protected static const string PORT_VECTOR_0 = "Vector0";
	protected static const string PORT_VECTOR_1 = "Vector1";
	protected static const string PORT_DISTANCE_THRESHOLD = "Threshold";
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_VECTOR_0,
		PORT_VECTOR_1,
		PORT_DISTANCE_THRESHOLD
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	[Attribute("0", UIWidgets.EditBox)]
	protected float m_fDistanceThreshold;
	
	static override string GetOnHoverDescription() { return "Returns true when distance between two vectors is below distance threshold."; }
	
	override bool TestFunction(AIAgent owner)
	{
		vector v0;
		vector v1;
		float t;
		
		if (!GetVariableIn(PORT_VECTOR_0, v0) ||
			!GetVariableIn(PORT_VECTOR_1, v1))
			return false;
		
		if (!GetVariableIn(PORT_DISTANCE_THRESHOLD, t))
			t = m_fDistanceThreshold;
		
		return vector.Distance(v0, v1) < t;
	}
}