enum EAudioSourceConfigurationFlag
{
	Static 						= 1 << 0,
	EnvironmentSignals 			= 1 << 1,
	FinishWhenEntityDestroyed 	= 1 << 2,
	BoundingVolume 				= 1 << 3,
	ExteriorSource 				= 1 << 4
}

[BaseContainerProps()]
class SCR_AudioSourceConfiguration
{
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Sound project (acp)", "acp")]
	ResourceName m_sSoundProject;
	
	[Attribute("", UIWidgets.EditBox, desc: "Sound event name")]
	string m_sSoundEventName;
	
	[Attribute(defvalue:"0 0 0", desc: "Sound offset towards entity origin")]
	vector m_vOffset;
		
	[Attribute("1", UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EAudioSourceConfigurationFlag))]
	EAudioSourceConfigurationFlag m_eFlags;
		
	//------------------------------------------------------------------------------------------------
	/*!
	Returns true if m_sSoundProject and m_sSoundEventName are defined and SCR_AudioSourceConfiguration is valid for AudioSource creation
	*/	
	bool IsValid()
	{
		return (m_sSoundProject != string.Empty && m_sSoundEventName != string.Empty);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns true if position offset is defined
	*/
	bool HasOffset()
	{
		return m_vOffset != vector.Zero;
	}
};