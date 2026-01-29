class SCR_AISemaphoreIn: AITaskScripted
{
	[Attribute("", UIWidgets.EditBox, "Lock name")]
	protected string m_lockName;
	
	/*[Attribute("", UIWidgets.EditBox, "Debug node name")]
	protected string m_lockName2;*/
	
	[Attribute("1", UIWidgets.EditBox, "Max Allowed Holders")]
	protected int m_maxAllowedHolders;
	
	// Make scripted node visible or hidden in nodes palette
    static override bool VisibleInPalette()
    {
        return true;
    }

    // Sets up input variables, as array of strings
	protected static ref TStringArray s_aVarsIn = {
		"LockNameIn"
	};
    override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
    
	protected SCR_LockContextComponent m_lockComponent;
	
	override protected string GetNodeMiddleText()
	{
		return "Lock name : " + m_lockName;
	}
 
	override void OnEnter(AIAgent owner)
	{
		string lockName;
		GetVariableIn("LockNameIn",lockName);
		if ( lockName ) 
			m_lockName = lockName;		
		GenericEntity ent; 
		if (AIGroup.Cast(owner))
			ent = owner;
		else
			ent = GenericEntity.Cast(owner.GetControlledEntity());
		m_lockComponent = SCR_LockContextComponent.Cast(ent.FindComponent(SCR_LockContextComponent));			
	}
		
    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (m_lockComponent)
		{
			//Print("Jsem :" + m_lockName2); 
			bool tryOpenLock = m_lockComponent.JoinKey(m_lockName,m_maxAllowedHolders);
			if (!tryOpenLock) 
				return ENodeResult.RUNNING;			
			else
			{ 
				return ENodeResult.SUCCESS;
			}		
		}
		return ENodeResult.FAIL;
	}		
};




