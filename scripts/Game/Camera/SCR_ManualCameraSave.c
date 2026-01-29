[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sWorldPath", true)]
class SCR_ManualCameraSave
{
	[Attribute()]
	ResourceName m_sWorldPath;
	
	[Attribute()]
	int m_iIndex;
	
	[Attribute()]
	ref array<ref SCR_ManualCameraComponentSave> m_aComponentData;
};

[BaseContainerProps(), BaseContainerCustomTitleField("m_sTypeName")]
class SCR_ManualCameraComponentSave
{
	[Attribute()]
	string m_sTypeName;
	
	[Attribute()]
	ref array<float> m_aValues;
	
	IEntity m_Target;
};