class SCR_BoundsPosition : ScriptAndConfig
{
	// This is a hack as currently Enfusion doesn't support usage of gizmo on vectors that are part of the array
	[Attribute(params: "inf inf purpose=coords space=entity anglesVar=Angles")]
	vector Offset;
}