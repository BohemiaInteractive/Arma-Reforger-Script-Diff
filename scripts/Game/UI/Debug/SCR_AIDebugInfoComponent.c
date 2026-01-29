class SCR_AIDebugInfoComponent : ScriptedWidgetComponent
{
	Widget m_wRoot;
	TextWidget m_wBehavior;
	TextWidget m_wCombat;
	TextWidget m_wOrder;
	TextWidget m_wTarget;
	TextWidget m_wThreat;	
	TextWidget m_wTable;
	TextWidget m_wAIInfo;
	
	CameraBase m_Camera;
	IEntity m_TargetAI; 
	IEntity m_FixedAI; 
	SCR_AIUtilityComponent m_UtilityComponent;
	SCR_AICombatComponent m_CombatComponent;
	SCR_AIInfoComponent m_InfoComponent;
	CameraManager m_CameraManager;
	
	const float OFFSET_X = 60;
	const float OFFSET_Y = 0;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		Widget parent = w.FindAnyWidget("Behavior");
		if (parent)
			m_wBehavior = TextWidget.Cast(parent.FindAnyWidget("Value"));
		
		parent = w.FindAnyWidget("Order");
		if (parent)
			m_wOrder = TextWidget.Cast(parent.FindAnyWidget("Value"));
		
		parent = w.FindAnyWidget("Target");
		if (parent)
			m_wTarget = TextWidget.Cast(parent.FindAnyWidget("Value"));		
		
		parent = w.FindAnyWidget("Threat");
		if (parent)
			m_wThreat = TextWidget.Cast(parent.FindAnyWidget("Value"));
		
		parent = w.FindAnyWidget("BehaviorTable");
		if (parent)
			m_wTable = TextWidget.Cast(parent.FindAnyWidget("Value"));
		
		parent = w.FindAnyWidget("CombatComponent");
		if (parent)
			m_wCombat = TextWidget.Cast(parent.FindAnyWidget("Value"));
		
		parent = w.FindAnyWidget("AIInfoComponent");
		if (parent)
			m_wAIInfo = TextWidget.Cast(parent.FindAnyWidget("Value"));
		
		if (GetGame().InPlayMode())
			m_wRoot.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void SelectAIAsFixed()
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SELECT_FIXED_AGENT))
		{
			m_FixedAI = m_TargetAI;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool UpdateUI()
	{		
		if (!m_CameraManager)
			m_CameraManager = GetGame().GetCameraManager();
		
		if (!m_CameraManager || !m_wRoot)
			return false;
		
		m_Camera = m_CameraManager.CurrentCamera();
		if (!m_Camera)
			return false;
		
		IEntity target = m_Camera.GetCursorTarget();
		
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SELECT_FIXED_AGENT))
			m_FixedAI = null;
		
		if ((!m_TargetAI || m_TargetAI != target || !m_UtilityComponent) && !m_FixedAI)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(target);
			if (!character)
			{
				m_wRoot.SetVisible(false);
				return false;
			}

			AIControlComponent comp = AIControlComponent.Cast(character.FindComponent(AIControlComponent));
			if (!comp)
			{
				m_wRoot.SetVisible(false);
				return false;
			}
			SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(comp.GetAIAgent());
			if (!agent)
			{
				m_wRoot.SetVisible(false);
				return false;
			}
			
			m_UtilityComponent = SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
			if (!m_UtilityComponent)
			{
				m_wRoot.SetVisible(false);
				return false;
			}
			
			m_CombatComponent = SCR_AICombatComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_AICombatComponent));
			if (!m_CombatComponent)
			{
				m_wRoot.SetVisible(false);
				return false;
			}
			
			m_InfoComponent = agent.m_InfoComponent;
			if (!m_InfoComponent)
			{
				m_wRoot.SetVisible(false);
				return false;
			}
			
			m_TargetAI = target;
		}
		
		SelectAIAsFixed();
		if (!m_UtilityComponent)
		{
			m_FixedAI = null;
			return false;
		}
		if (m_FixedAI)
			target = m_FixedAI;

		m_wRoot.SetVisible(true);
		vector boundMin, boundMax;
		target.GetWorldBounds(boundMin, boundMax);
		vector entityCenter = (boundMax + boundMin) * 0.5;
		
		vector pos = GetGame().GetWorkspace().ProjWorldToScreen(entityCenter, target.GetWorld());
		if (!m_FixedAI)
			FrameSlot.SetPos(m_wRoot.GetChildren(), pos[0] + OFFSET_X, pos[1]);
		else
			FrameSlot.SetPos(m_wRoot.GetChildren(), 0, 60); // moved because there is a toolbar in zeus
		
		if (m_wBehavior && m_UtilityComponent.m_CurrentBehavior)
		{
			typename type = m_UtilityComponent.m_CurrentBehavior.Type();
			m_wBehavior.SetText(type.ToString());
		}
		
		if (m_wOrder)
		{
			// @TODO: Get this back
			/*
			string name = SCR_AIDebug.GetBehaviorName(m_UtilityComponent.m_);
			m_wBehavior.SetText(name);
			SCR_AIDebug.GetOrderName(m_wOrder);
			*/
		}

		if (m_wThreat && m_UtilityComponent.m_ThreatSystem != null)
		{
			//order has to corespond to layout
			m_UtilityComponent.m_ThreatSystem.DebugPrintToWidget(m_wThreat);
		}
		
		if (m_wTable)
		{
			array<string> results = GetSortedBehaviors();
			string finalString;
			int resultsCount = results.Count();
			for (int i = 0; i < resultsCount; i++)
			{
				finalString = finalString + results[i];
				if (i + 1 < resultsCount)
					finalString = finalString + "\n";
			}
			m_wTable.SetText(finalString);
		}
		
		if (m_wCombat)
		{
			m_CombatComponent.DebugPrintToWidget(m_wCombat);
		}
		
		if (m_wAIInfo)
		{
			m_InfoComponent.DebugPrintToWidget(m_wAIInfo);
		}
		
		return false;
	}

	// Create table with results
	ref array<string> GetSortedBehaviors()
	{
		array<string> results = {};
		array<ref AIActionBase> actions = {};
			
		m_UtilityComponent.GetActions(actions);

		string resultString;
		float highScore;
		int highIndex;
		while (!actions.IsEmpty())
		{
			highScore = -float.MAX;
			highIndex = -1;
			foreach (int i, AIActionBase action : actions)
			{
				float score = action.Evaluate();
				if (score > highScore)
				{
					highScore = score;
					highIndex = i;
				}
			}
			
			resultString = actions[highIndex].Type().ToString() + "   " + Math.Round(highScore);
			results.Insert(resultString);
			
			actions.RemoveOrdered(highIndex);
		}
		
		return results;
	}
};
