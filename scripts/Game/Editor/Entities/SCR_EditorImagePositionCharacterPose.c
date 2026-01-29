
//------------------------------------------------------------------------------------------------
/*!
Defines what anim should be used for character posiotion
Defines weapon holding anim
Can be faction specific
*/ 
[BaseContainerProps(configRoot: true)]
class SCR_EditorImagePositionCharacterPose
{
	[Attribute(desc: "Text key of faction for which pose should be applied. E.g. 'US'")]
	protected FactionKey m_sFactionKey;
	
	[Attribute("{146C9CE65076318A}anims/workspaces/player/Poses/poses.agr", uiwidget:UIWidgets.ResourceNamePicker, params: "agr", category: "Animation")]
	protected ResourceName m_PosesGraph;
	
	[Attribute("{19590FF36F6CF429}anims/workspaces/player/Poses/player_poses.asi", uiwidget:UIWidgets.ResourceNamePicker, params: "asi", category: "Animation")]
	protected ResourceName m_PosesInstance;
	
	[Attribute("WeaponIK", category: "Animation")]
	protected string m_sStartNode;
	
	[Attribute("PoseID", category: "Animation")]
	protected string m_sPoseVar;
	
	[Attribute("0", UIWidgets.Slider, params: "0 40 1")]
	protected int m_iPoseID;
	
	[Attribute("1", UIWidgets.Slider, params: "1 7 1")]
	protected int m_iArmIK;
	
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, category: "Animation", enums: SCR_Enum.GetList(EWeaponType, ParamEnum("<Unchanged>", "-1")))]
	protected EWeaponType m_ForceWeaponType;
	
	//------------------------------------------------------------------------------------------------
	string GetFactionKey()
	{
		return m_sFactionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetPoseGraph()
	{
		return m_PosesGraph;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetPosesInstance()
	{
		return m_PosesInstance;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetStartNode()
	{
		return m_sStartNode;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetPosVar()
	{
		return m_sPoseVar;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPosId()
	{
		return m_iPoseID;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetArmIK()
	{
		return m_iArmIK;
	}
	
	//------------------------------------------------------------------------------------------------
	EWeaponType GetForceWeaponType()
	{
		return m_ForceWeaponType;
	}
	
	//------------------------------------------------------------------------------------------------
	int BindAnimValueInteger(notnull PreviewAnimationComponent animComponent, string key)
	{
		return animComponent.BindIntVariable(key);
	}
}