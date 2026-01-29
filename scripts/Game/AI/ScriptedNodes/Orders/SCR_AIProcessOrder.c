class SCR_AIProcessOrder: AITaskScripted
{
	static const string ORDER_PORT = "OrderObjectIn";
    static const string ORDER_TYPE = "OrderTypeOut";
	static const string SCRIPTED_ORDER = "IsScriptedOrder";
    static const string ORDER_VALUE = "SciptedOrderValue";
    static const string ORDER_DEBUG_TEXT = "DebugText";
   
	
	[Attribute("0", UIWidgets.CheckBox, "Log Debug")]
	bool m_bDebugMe;
		
	AIOrder m_Order;
	string m_sDebugString;
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (!GetVariableIn(ORDER_PORT, m_Order))
			return ENodeResult.FAIL;
		if (!m_Order)
			return ENodeResult.FAIL;
		
		SetVariableOut(ORDER_DEBUG_TEXT, m_Order.GetText());
		
		SCR_AIOrderBase order = SCR_AIOrderBase.Cast(m_Order);
		if (!order && m_Order)
		{
			SetVariableOut(ORDER_TYPE, m_Order.GetOrderType());
			SetVariableOut(SCRIPTED_ORDER, false);
			ClearVariable(ORDER_VALUE);
		}
		else
		{
			order.GetOrderParameters(this);	
		#ifdef WORKBENCH
			if (m_bDebugMe)
				PrintFormat("%1 : %2",owner,m_sDebugString);
		#endif
		}
		
		#ifdef WORKBENCH
		SCR_AIOrderBase scriptOrder = SCR_AIOrderBase.Cast(m_Order);
		string dbgString;
		if (scriptOrder)
			dbgString = typename.EnumToString(EOrderType_Character,scriptOrder.m_eOrderType);
		else
			dbgString = m_Order.GetOrderTypeString();
		SCR_AIDebugVisualization.VisualizeMessage(owner.GetControlledEntity(), dbgString, EAIDebugCategory.ORDER, 3);
		#endif
		
		#ifdef AI_DEBUG
		AddDebugMessage(owner, string.Format("Process order: %1, from BT: %2", order, order.m_sSentFromBt));
		#endif
			
		return ENodeResult.SUCCESS;		
	}
	
	protected static ref TStringArray s_aVarsOut = {
		SCRIPTED_ORDER,
		ORDER_TYPE,
		ORDER_VALUE,
		ORDER_DEBUG_TEXT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }

	protected static ref TStringArray s_aVarsIn = {
		ORDER_PORT
	};	
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
		
	static override bool VisibleInPalette()
	{
		return true;
	}	
	
	static override string GetOnHoverDescription() 
	{ 
		return "Process order: reads order and gets type and data of order from it";
	};
	
	#ifdef AI_DEBUG
	protected void AddDebugMessage(AIAgent agent, string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(agent.FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(str, msgType: EAIDebugMsgType.MAILBOX);
	}
	#endif
};
