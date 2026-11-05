class SCR_AIChooseIdleGesture : AITaskScripted
{
	protected ref array<float> m_aGestureWeights = {120, 10, 0.5, 5};
	protected ref array<int> m_aGestureEnumValue = {-1, SCR_ECharacterGestureType.SIGN, SCR_ECharacterGestureType.BOO, SCR_ECharacterGestureType.CHEER};
	protected float m_fSumOfWeights;
	protected const static string PACKED_INT = "PackedIntOut";
	protected const static string JUST_IDLE = "bJustIdle";
	protected static ref TStringArray _s_aVarsOut = {JUST_IDLE, PACKED_INT};
	
	//----------------------------------------------------------------------------------------------------
	override event void OnInit(AIAgent owner)
	{
		m_fSumOfWeights = 0;
		
		for (int i, count = m_aGestureWeights.Count(); i < count; i++)
		{
			float oldSum = m_fSumOfWeights;
			m_fSumOfWeights += m_aGestureWeights[i];
			m_aGestureWeights[i] = oldSum + m_aGestureWeights[i];
		}		
	}
		
	//----------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt) 
	{ 
		float rand = s_AIRandomGenerator.RandFloatXY(0, m_fSumOfWeights);
		SCR_ECharacterGestureType gestureType;
		
		if (rand < m_aGestureWeights[0])
		{
			SetVariableOut(JUST_IDLE, true);
			return ENodeResult.SUCCESS;
		}
			
		for (int i = 1, max = m_aGestureWeights.Count(); i < max; i++)
		{
			if (rand < m_aGestureWeights[i])
			{
				gestureType = m_aGestureEnumValue[i];
				break;
			}	
		}
		
		int mood = Math.RandomInt(0, 100);
		
		SetVariableOut(PACKED_INT, SCR_AIIntegerOperations.EncodeTwo(gestureType, mood));
		ClearVariable(JUST_IDLE);
		
		return ENodeResult.SUCCESS; 
	};
		
	//----------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut() { return _s_aVarsOut; }	
	
	//----------------------------------------------------------------------------------------------------
	override static string GetOnHoverDescription()
	{
		return "SCR_AIChooseIdleGesture: selects one of available gestures by set distribution and returns correct values for SendOrder_Gesture node.";
	}
	
	//----------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
}