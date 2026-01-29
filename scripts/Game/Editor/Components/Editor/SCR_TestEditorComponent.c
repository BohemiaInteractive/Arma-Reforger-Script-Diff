[ComponentEditorProps(category: "GameScripted/Editor", description: "", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_TestEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/// @ingroup Editor_Components
/**

@example
*/
class SCR_TestEditorComponent : SCR_BaseEditorComponent
{
	override void EOnEditorInit()
	{
		PrintFormat("%1: EOnEditorInit", this);
	}
	override void EOnEditorDelete()
	{
		PrintFormat("%1: EOnEditorDelete", this);
	}
	override void EOnEditorRequest(bool isOpen)
	{
		PrintFormat("%1: EOnEditorRequest: %2", this, isOpen);
	}
	override void EOnEditorOpen()
	{
		PrintFormat("%1: EOnEditorOpen", this);
	}
	override void EOnEditorClose()
	{
		PrintFormat("%1: EOnEditorClose", this);
	}	
	override void EOnEditorPreActivate()
	{
		PrintFormat("%1: EOnEditorPreActivate", this);
	}
	override void EOnEditorActivate()
	{
		PrintFormat("%1: EOnEditorActivate", this);
	}
	override bool EOnEditorActivateAsync(int attempt)
	{
		PrintFormat("%1: EOnEditorActivateAsync: %2", this, attempt);
		return true;
	}
	override void EOnEditorPostActivate()
	{
		PrintFormat("%1: EOnEditorPostActivate", this);
	}
	override void EOnEditorDeactivate()
	{
		PrintFormat("%1: EOnEditorDeactivate", this);
	}
	override bool EOnEditorDeactivateAsync(int attempt)
	{
		PrintFormat("%1: EOnEditorDeactivateAsync: %2", this, attempt);
		return true;
	}
	override void EOnEditorPostDeactivate()
	{
		PrintFormat("%1: EOnEditorPostDeactivate", this);
	}
	override void EOnEditorDebug(array<string> debugTexts)
	{
		debugTexts.Insert(Type().ToString());
	}
	override void EOnEffect(SCR_BaseEditorEffect effect)
	{
		PrintFormat("%1: EOnEffect %1: %2", this, effect);
	}
	
	
	override void EOnEditorInitServer()
	{
		PrintFormat("%1: EOnEditorInitServer", this);
	}
	override void EOnEditorDeleteServer()
	{
		PrintFormat("%1: EOnEditorDeleteServer", this);
	}
	override void EOnEditorOpenServer()
	{
		PrintFormat("%1: EOnEditorOpenServer", this);
	}
	override void EOnEditorOpenServerCallback()
	{
		PrintFormat("%1: EOnEditorOpenServerCallback", this);
	}
	override void EOnEditorCloseServer()
	{
		PrintFormat("%1: EOnEditorCloseServer", this);
	}
	override void EOnEditorCloseServerCallback()
	{
		PrintFormat("%1: EOnEditorCloseServerCallback", this);
	}
	override void EOnEditorActivateServer()
	{
		PrintFormat("%1: EOnEditorActivateServer", this);
	}
	override void EOnEditorDeactivateServer()
	{
		PrintFormat("%1: EOnEditorDeactivateServer", this);
	}
};