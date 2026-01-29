[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_BaseEditorServerComponentClass: ScriptComponentClass
{
};
class SCR_BaseEditorServerComponent: ScriptComponent
{
	static Managed GetInstance(typename type)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core && core.GetEditorServer())
			return core.GetEditorServer().FindComponent(type);
		else
			return null;
	}
};