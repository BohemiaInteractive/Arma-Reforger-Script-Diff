// Obsolete! Use SCR_AISendOrder_... instead!
class SCR_AISendOrder: SCR_AISendMessageGeneric
{
	static const string ORDER_TYPE = "OrderTypeIn";
	static const string ORDER_VALUE = "OrderValueIn";
	static const string ORDER_DEBUG_TEXT = "DebugText";	
	
	[Attribute("0", UIWidgets.ComboBox, "Order type", "", ParamEnumArray.FromEnum(EOrderType_Character))]
	private EOrderType_Character m_orderType;
	
	private int m_iValue // value of scripted order
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		Print("SCR_AISendOrder is obsolete!", LogLevel.WARNING);
		return ENodeResult.FAIL;
	}
	
	protected static ref TStringArray s_aVarsIn2 = {
		ORDER_TYPE,
		PORT_RECEIVER,
		ORDER_VALUE,
		ORDER_DEBUG_TEXT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn2;
    }
	
	static override string GetOnHoverDescription() 
	{ 
		return "Obsolete! Use SCR_AISendOrder_... instead!";	
	};
};
