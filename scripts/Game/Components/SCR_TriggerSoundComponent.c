class SCR_TriggerSoundComponentClass : SoundComponentClass
{
}


[EntityEditorProps(category: "GameScripted/Sound", description: "Plays sounds, within defined time and range")]
class SCR_TriggerSoundComponent : SoundComponent
{	
	[Attribute("", UIWidgets.EditBox, "")]
	protected string m_sSoundEvent;
	
	[Attribute(defvalue: "100", UIWidgets.Slider, desc: "Sounds will not be trigger if distance is less than this.", "0 5000 1")]
	protected float m_fTriggerDistanceMin;
	
	[Attribute(defvalue: "1000", UIWidgets.Slider, "Sounds will not be trigged if distance is greater than this.", "0 5000 1")]
	protected float m_fTriggerDistanceMax;
	
	[Attribute("4", UIWidgets.Slider, "Repetition count", "0 10 1")]
	protected int m_iRepCount;
	
	[Attribute("0", UIWidgets.Slider, "Repetition count randomisation", "0 10 1")]
	protected int m_iRepCountRnd;
	
	[Attribute("10", UIWidgets.Slider, "Repetition time [seconds]", "0 30 0.001")]
	protected float m_iRepTime;
	
	[Attribute("10", UIWidgets.Slider, "Repetition time randomization [seconds]", "0 30 0.001")]
	protected float m_iRepTimeRnd;
	
	[Attribute("20", UIWidgets.Slider, "Sequence reprtition time [seconds]", "0 60 0.001")]
	protected float m_iSequenceRepTime;
	
	[Attribute("20", UIWidgets.Slider, "Sequence repetition randomization [seconds]", "0 60 0.001")]
	protected float m_iSequenceRepTimeRnd;
	
	protected float m_fTime;
	protected float m_fTriggerTime;
	
	protected int m_iRepCountCurent;
			
	//------------------------------------------------------------------------------------------------	
	private int GetRepCount()
	{
		return Math.RandomIntInclusive(Math.Max(0, m_iRepCount - m_iRepCountRnd), m_iRepCount + m_iRepCountRnd);
	}
	
	//------------------------------------------------------------------------------------------------	
	private float GetRepTime()
	{
		if (m_iRepTimeRnd < 0.001)
		{
			return m_iRepTime;
		}
		else
		{
			return  Math.RandomFloatInclusive(Math.Max(0, m_iRepTime - m_iRepTimeRnd), m_iRepTime + m_iRepTimeRnd);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	private float GetSequenceRepTime()
	{
		if (m_iSequenceRepTimeRnd < 0.001)
		{
			return m_iSequenceRepTime;
		}
		else
		{
			return Math.RandomFloatInclusive(Math.Max(0, m_iSequenceRepTime - m_iSequenceRepTimeRnd), m_iSequenceRepTime + m_iSequenceRepTimeRnd);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	private bool IsInRange(IEntity owner)
	{		
		vector matOwner[4];
		vector matCamera[4];
		
		owner.GetTransform(matOwner);
		owner.GetWorld().GetCurrentCamera(matCamera);

		const float distance = vector.Distance(matOwner[3], matCamera[3]);
		return Math.IsInRange(distance, m_fTriggerDistanceMin, m_fTriggerDistanceMax);			
	}
	
	//------------------------------------------------------------------------------------------------	
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{			
		m_fTime += timeSlice;		
		
		if (m_fTime > m_fTriggerTime)
		{
			if (IsInRange(owner))
			{
				SoundEvent(m_sSoundEvent);
			}
			
			if (m_iRepCountCurent <= 1)
			{
				m_iRepCountCurent = GetRepCount();
				m_fTriggerTime = GetSequenceRepTime();
			}
			else
			{
				m_iRepCountCurent--;
				m_fTriggerTime = GetRepTime();
			}
			
			m_fTime = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdateSoundJobBegin(IEntity owner)
	{
		m_fTime = 0;
		m_iRepCountCurent = GetRepCount();
		m_fTriggerTime = GetSequenceRepTime();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_TriggerSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_TriggerSoundComponent()
	{
	}

}
