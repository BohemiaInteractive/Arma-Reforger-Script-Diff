// Base for data needed in custom animations
[BaseContainerProps(configRoot: true)]
class SCR_CustomAnimDataConfig_Base
{
	[Attribute("", desc: "Animation data for the start animation")]
	ref SCR_CustomAnimData_Properties m_StartAnimation;
	
	[Attribute("", desc: "Animation data for the stop animation")]
	ref SCR_CustomAnimData_Properties m_StopSlowAnimation;
	
	[Attribute("", desc: "Animation data for the fast stop / abort animation")]
	ref SCR_CustomAnimData_Properties m_HardCancelAnimation;
	
	[Attribute("-1", desc: "Duration of the animation loop, -1 for infinite")]
	float m_fAnimationDuration;
}

//----------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_CustomAnimData_Properties
{
	const static string BINDING_NAME_NPC = "NPC";
	const static string BINDING_COMMAND_NAME = "CMD_Gestures";
	
	[Attribute("", desc: "Animation graph name, *.agr file")]
	ResourceName m_sGraphName;
	
	[Attribute("", desc: "Animation instance name, *.asi file")]
	ResourceName m_sGraphInstanceName;
	
	[Attribute(BINDING_NAME_NPC , desc: "Graph binding name")]	
	string m_sGraphBindingName;
	
	[Attribute(BINDING_COMMAND_NAME, desc: "Animation type command name")]
	string m_sCommandName; 
	
	[Attribute("-1", desc: "Animation custom command value")]
	int m_iCommandValue; 
	
	[Attribute("", desc: "Animation control variable name")]
	string m_sControlVariableName;
	
	[Attribute("-1", desc: "Animation control variable value")]
	int m_iControlVariableValue;
	
	[Attribute("0", desc: "Use root motion")]
	bool m_bAllowRootMotion;
	
	TAnimGraphCommand m_iCommandBindID = -1; 	// this is the ID that will return the Bind method of anim graph for command
	TAnimGraphCommand m_iControlVariableID = -1; // this is the ID that will return the Bind method of anim graph for control variable
	
	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		bool stringsEmpty =  m_sGraphName.IsEmpty() && m_sGraphInstanceName.IsEmpty() && m_sControlVariableName.IsEmpty();
		bool stringsDefault = m_sGraphBindingName == BINDING_NAME_NPC && m_sCommandName == BINDING_COMMAND_NAME;
		bool intsDefault = m_iCommandValue == -1 && m_iControlVariableValue == -1 && m_iCommandBindID == -1 && m_iControlVariableID == -1;
		
		return stringsEmpty && stringsDefault && intsDefault && !m_bAllowRootMotion;
	}
}