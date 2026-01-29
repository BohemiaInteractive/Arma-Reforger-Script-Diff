class SCR_AIGetSmartActionComponent : AITaskScripted
{
	static const string WAYPOINT_PORT = "WaypointIn";
	static const string COMPONENT_PORT = "SmartActionComponent";	
	
	//------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		WAYPOINT_PORT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }

	//------------------------------------------------------------------------------------------	
	protected static ref TStringArray s_aVarsOut = {
		COMPONENT_PORT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
	}
	
	//------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}
		
		AIWaypoint wp;
		if(!GetVariableIn(WAYPOINT_PORT,wp))
			wp = group.GetCurrentWaypoint();
		if (!wp)
			return ENodeResult.FAIL;
		
		SCR_SmartActionWaypoint SAWP = SCR_SmartActionWaypoint.Cast(wp);
		
		if (!SAWP)
			 return ENodeResult.FAIL;
				
		IEntity SAEntity;
		string SATag;
				
		SAWP.GetSmartActionEntity(SAEntity, SATag);
		
		if (!SAEntity || SATag.IsEmpty())		
			return ENodeResult.FAIL;			
		
		if (SAEntity)
		{
			array<Managed> outComponents = {};
			SCR_AISmartActionComponent component;
			array<string> outTags = {};
			SAEntity.FindComponents(SCR_AISmartActionComponent, outComponents);
			// searching in hierarchy of SAEntity for SmartActionComponents
			IEntity child = SAEntity.GetChildren();
			while (child)
			{
				array<Managed> entityComponents = {};
				child.FindComponents(SCR_AISmartActionComponent,entityComponents);
				outComponents.InsertAll(entityComponents);
				child = child.GetSibling();
			}
			// searching SmartActionComponents of smartactions with correct TAG
			foreach (Managed outComponent : outComponents)
			{
				component = SCR_AISmartActionComponent.Cast(outComponent);
				if (!component)
					continue;
				component.GetTags(outTags);
				if (outTags.Contains(SATag) && component.IsActionAccessible())
				{
					SetVariableOut(COMPONENT_PORT, component);
					return ENodeResult.SUCCESS;
				}
			}
		}
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns SmartActionComponent set on waypoint, fails if waypoint is not set properly (entity or tag is missing).";
	}	
};
