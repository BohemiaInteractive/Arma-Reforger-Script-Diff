// Script File

class SCR_ScriptedSignalUserActionDecrease : ScriptedSignalUserAction
{
	[Attribute( defvalue: "0.5", uiwidget: UIWidgets.EditBox, desc: "How fast the signal will change." )]
	private float m_fSpeed;
	
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		SetSignalValue(GetCurrentValue() - timeSlice * m_fSpeed);
	}
};