class SCR_LightSlot : BaseLightSlot 
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Name of the sound event that should be called when the lights are activated.", category: "Sounds")]
	protected string m_sLightOnEventName;

	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Name of the sound event that should be called when the lights are deactivated.", category: "Sounds")]
	protected string m_sLightOffEventName;
	
	protected bool m_bIsActive;
	protected SoundComponent m_SoundComponent;

	protected ref ScriptInvoker m_OnLightStateChanged; // TODO: ScriptInvokerBool

	//------------------------------------------------------------------------------------------------
	//! Get event called when the light is turned on or off.
	//! \return Script invoker (bool)
	ScriptInvoker GetOnLightStateChanged()
	{
		if (!m_OnLightStateChanged)
			m_OnLightStateChanged = new ScriptInvoker();

		return m_OnLightStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	override void OnLightStateChanged(bool newState)
	{
		if (m_bIsActive == newState)
			return;
		
		m_bIsActive = newState;
		if (m_OnLightStateChanged)
			m_OnLightStateChanged.Invoke(newState);
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		super.OnInit(owner);
		m_SoundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		
		if (m_SoundComponent)
			GetOnLightStateChanged().Insert(PlaySound);
	}

	//------------------------------------------------------------------------------------------------
	//! \param newState
	void PlaySound(bool newState)
	{
		string eventName;
		
		if (newState)
			eventName = m_sLightOnEventName;
		else
			eventName = m_sLightOffEventName;
		
		if (m_SoundComponent && !eventName.IsEmpty())
			m_SoundComponent.SoundEvent(eventName);	
	}
};
