[EntityEditorProps(category: "GameScripted/Sound", description: "")]
class SCR_SoundDataComponentClass : ScriptComponentClass
{
	[Attribute("", UIWidgets.Auto, desc: "Audio Source Configuration)")]
	ref array<ref SCR_AudioSourceConfiguration> m_aAudioSourceConfiguration;
}

class SCR_SoundDataComponent : ScriptComponent
{	
	//------------------------------------------------------------------------------------------------
	//! \param[in] eventName Sound event name used for seach for audio project resource
	//! \return audio project resource for the corresponding sound event or null if not found or invalid
	SCR_AudioSourceConfiguration GetAudioSourceConfiguration(string eventName)
	{
		SCR_SoundDataComponentClass prefabData = SCR_SoundDataComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;
		
		foreach (SCR_AudioSourceConfiguration audioSourceConfiguration : prefabData.m_aAudioSourceConfiguration)
		{
			if (audioSourceConfiguration.m_sSoundEventName == eventName)
			{
				if (audioSourceConfiguration.m_sSoundProject != string.Empty)
				{
					return audioSourceConfiguration;
				}
				else
				{
					return null;
				}
			}
		}
				
		return null;
	}
}