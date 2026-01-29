class SCR_AIAnimationWaypointClass: SCR_AIWaypointClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_AIAnimationParameters
{
	vector m_vAnimationLocalTransform[4];
	float m_fAnimationDuration;
	ref SCR_AIAnimation_Base m_oAnimationClass;
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AIAnimationWaypointParameters
{
	[Attribute("", UIWidgets.Object, "Animation position and angles - use PointInfo")]
	ref PointInfo m_oAnimationLocalTransform;
	[Attribute("-1", UIWidgets.EditBox, "Animation duration [s], -1 for indefinite")]
	float m_fAnimationDuration;
	[Attribute("", UIWidgets.Object, "Animation methods")]
	ref SCR_AIAnimation_Base m_oAnimationClass;
	
	private ref SCR_AIAnimationParameters m_Parameters;
	
	void SCR_AIAnimationWaypointParameters()
	{
		if (SCR_Global.IsEditMode())
			return;
		m_Parameters = new SCR_AIAnimationParameters;
		if (m_oAnimationLocalTransform)
			m_oAnimationLocalTransform.GetLocalTransform(m_Parameters.m_vAnimationLocalTransform);
		m_Parameters.m_fAnimationDuration = m_fAnimationDuration;
		m_Parameters.m_oAnimationClass = m_oAnimationClass;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIAnimationParameters GetParameters()
	{
		return m_Parameters;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParameters(vector animationTransform[4], float animationDuration, SCR_AIAnimation_Base animationClass)
	{
		m_Parameters.m_vAnimationLocalTransform = animationTransform;
		m_Parameters.m_fAnimationDuration = animationDuration;
		m_Parameters.m_oAnimationClass = animationClass;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AIAnimationScript
{
	[Attribute("", UIWidgets.EditBox, "Actor name", "")]
	string m_sAgentsName;
	
	[Attribute("", UIWidgets.Object, "Animation list to perform by the actor given by the index (of group member)")]
	protected ref array<ref SCR_AIAnimationWaypointParameters> m_aAgentAnimationParameters;
	
	//------------------------------------------------------------------------------------------------
	string GetAnimationActorName()
	{
		return m_sAgentsName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAnimationActorName(string actorName)
	{
		m_sAgentsName = actorName;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddAnimationWaypointParameter(notnull SCR_AIAnimationWaypointParameters animationWaypointParameter, int animationIndex)
	{
		if (!m_aAgentAnimationParameters)
			m_aAgentAnimationParameters = {};
		
		m_aAgentAnimationParameters.InsertAt(animationWaypointParameter, animationIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	bool RemoveAnimationWaypointParameter(SCR_AIAnimationWaypointParameters animationWaypointParameter)
	{
		int animationIndex = m_aAgentAnimationParameters.Find(animationWaypointParameter);
		if (animationIndex < 0) 
			return false;
		m_aAgentAnimationParameters.RemoveOrdered(animationIndex);
		return true;		
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIAnimationWaypointParameters GetParameters(int animationIndex)
	{
		return m_aAgentAnimationParameters[animationIndex];
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsAnimationIndexValid(int animationIndex)
	{
		return m_aAgentAnimationParameters.IsIndexValid(animationIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetAnimationPosition(IEntity rootEntity, int animationIndex)
	{
		if (!m_aAgentAnimationParameters.IsIndexValid(animationIndex))
			return vector.Zero;
		vector mat[4], outMat[4];
		rootEntity.GetWorldTransform(mat);
		Math3D.MatrixMultiply4(mat,m_aAgentAnimationParameters[animationIndex].GetParameters().m_vAnimationLocalTransform, outMat);
		return outMat[3];
	}
		
	//------------------------------------------------------------------------------------------------
	void GetAnimationWorldTransform(IEntity rootEntity, int animationIndex, out vector transform[4])
	{
		if (!m_aAgentAnimationParameters.IsIndexValid(animationIndex))
			return;
		vector origMat[4];
		rootEntity.GetWorldTransform(origMat);
		Math3D.MatrixMultiply4(origMat, m_aAgentAnimationParameters[animationIndex].GetParameters().m_vAnimationLocalTransform, transform);
	}
	
	//------------------------------------------------------------------------------------------------	
	float GetAnimationDuration(int animationIndex)
	{
		if (!m_aAgentAnimationParameters.IsIndexValid(animationIndex))
			return 0;
		return m_aAgentAnimationParameters[animationIndex].GetParameters().m_fAnimationDuration;
	}
	
	//------------------------------------------------------------------------------------------------	
	SCR_AIAnimation_Base GetAnimationClass(int animationIndex)
	{
		if (!m_aAgentAnimationParameters.IsIndexValid(animationIndex))
			return null;
		return m_aAgentAnimationParameters[animationIndex].GetParameters().m_oAnimationClass;
	}
};

//------------------------------------------------------------------------------------------------
void SCR_AIOnAnimationBehaviorAction(AIAgent agent, bool startingAnimation, int animationIndex);
typedef func SCR_AIOnAnimationBehaviorAction;

//------------------------------------------------------------------------------------------------
class SCR_AIAnimationWaypoint : SCR_AIWaypoint
{
	[Attribute("", UIWidgets.Object, "List of group members scripts - what to do on this waypoint, each member has array of animations on locations")]
	ref array<ref SCR_AIAnimationScript> m_aAnimationScripts;
	
	protected ref ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> m_OnAnimationBehaviorAction; // called if any of the waypoint's users (characters) start or stop animation
	
	//----------------------------------------------------------------------------------------
	void AddAnimationScript(notnull SCR_AIAnimationScript animationScript, int animationIndex = -1)
	{
		if (animationIndex < 0)
			m_aAnimationScripts.Insert(animationScript);
		else 
			m_aAnimationScripts.InsertAt(animationScript, animationIndex);
	}
	
	//----------------------------------------------------------------------------------------
	bool RemoveAnimationScript(SCR_AIAnimationScript animationScript)
	{
		int animationIndex = m_aAnimationScripts.Find(animationScript);
		if (animationIndex < 0)
			return false;
		m_aAnimationScripts.RemoveOrdered(animationIndex);
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	override SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_AIAnimationWaypointState(groupUtilityComp, this);
	}	
	
	//----------------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> GetOnAnimationBehaviorAction()
	{
		if (!m_OnAnimationBehaviorAction)
			m_OnAnimationBehaviorAction = new ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction>();
		return m_OnAnimationBehaviorAction; 
	}
};

//------------------------------------------------------------------------------------------------
class SCR_AIAnimationWaypointState : SCR_AIWaypointState
{
	//------------------------------------------------------------------------------------------------
	override void OnExecuteWaypointTree()
	{
		super.OnExecuteWaypointTree();
		SCR_AIAnimationWaypoint waypoint = SCR_AIAnimationWaypoint.Cast(m_Waypoint);
		if (!waypoint && m_Waypoint)
			m_Utility.m_Owner.CompleteWaypoint(m_Waypoint);
		
		auto activity = new SCR_AIAnimateActivity(m_Utility, waypoint, priorityLevel: waypoint.GetPriorityLevel());
		m_Utility.AddAction(activity);
	}
};
