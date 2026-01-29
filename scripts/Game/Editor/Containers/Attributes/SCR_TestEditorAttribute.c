//#define ATTRIBUTES_DEBUG
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_TestEditorAttribute: SCR_BaseEditorAttribute
{
	[Attribute()];
	private bool m_bRandomize;
	
	[Attribute()];
	private bool m_bIncompatible;
	
	private bool m_Value;
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		#ifndef ATTRIBUTES_DEBUG
			return null;
		#endif
		if (m_bIncompatible) return null;
		Print(GetUIInfo().GetName() + ": Read " + m_Value.ToString(), LogLevel.DEBUG);
		return SCR_BaseEditorAttributeVar.CreateBool(m_Value);
	}
	override SCR_BaseEditorAttributeVar CreateDefaultVariable()
	{
		return SCR_BaseEditorAttributeVar.CreateBool(false);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (var) m_Value = var.GetInt();
		Print(GetUIInfo().GetName() + ": Write " + m_Value.ToString(), LogLevel.DEBUG);
	}
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		Print(GetUIInfo().GetName() + ": Preview", LogLevel.DEBUG);
	}
	
	void SCR_TestEditorAttribute()
	{
		if (m_bRandomize) m_Value = Math.RandomIntInclusive(0, 1);
	}
};