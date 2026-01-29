[ComponentEditorProps(category: "GameScripted", description: "")]
class SCR_LightningComponentClass : ScriptComponentClass
{
}

//! Inform the weather manager a lightning has been spawned. Weather Manager will handle light changes.
class SCR_LightningComponent : ScriptComponent
{
	protected ref RandomGenerator m_pRandomGenerator = new RandomGenerator();
	
	[Attribute(defvalue: "4")]
	protected int m_iMinFlashes;
	
	[Attribute(defvalue: "8")]
	protected int m_iMaxFlashes;
	
	[Attribute(defvalue: "25")];
	protected float m_fFlashMinDurationMillis;
	
	[Attribute(defvalue: "200")];
	protected float m_fFlashMaxDurationMillis;
	
	[Attribute(defvalue: "30")];
	protected float m_fFlashMinCooldownMillis;
	
	[Attribute(defvalue: "100")];
	protected float m_fFlashMaxCooldownMillis;
	
	[Attribute(defvalue: "1")]
	protected float m_fFlashMinVisibilityRadiusKM;
	
	[Attribute(defvalue: "3")]
	protected float m_fFlashMaxVisibilityRadiusKM;
	
	
	[RplProp(onRplName: "OnFlashesRpl")]
	protected int m_iNumFlashes;
	
	[RplProp(onRplName: "OnFlashesRpl")]
	protected float m_fRadiusKM;
	
	[RplProp(onRplName: "OnFlashesRpl")]
	protected ref array<float> m_aFlashesStartTime = {};

	[RplProp(onRplName: "OnFlashesRpl")]
	protected ref array<float> m_aFlashesDuration = {};

	[RplProp(onRplName: "OnFlashesRpl")]
	protected ref array<float> m_aFlashesCooldownDuration = {};
	
	protected int m_iCurFlashIndex;
	
	//------------------------------------------------------------------------------------------------
	//!
	void OnFlashesRpl()
    {
    	if(m_iCurFlashIndex < m_iNumFlashes 
			&& m_iNumFlashes == m_aFlashesStartTime.Count() 
			&& m_aFlashesStartTime.Count() == m_aFlashesDuration.Count() 
			&& m_aFlashesStartTime.Count() == m_aFlashesCooldownDuration.Count())
		{
			ChimeraWorld world = GetOwner().GetWorld();
			TimeAndWeatherManagerEntity tawme =world.GetTimeAndWeatherManager();
			if(!tawme)
				return;
			
			//inform weather entity 
			WeatherLightning wl = new WeatherLightning();
			wl.SetPosition(GetOwner().GetOrigin());
			wl.SetRadius(m_fRadiusKM);
			
			for(int n = m_iCurFlashIndex; n < m_aFlashesStartTime.Count(); n++)
			{
				WeatherLightningFlash wlf = new WeatherLightningFlash();
				
				wlf.SetStartTime(m_aFlashesStartTime[n]);
				wlf.SetDuration(m_aFlashesDuration[n]);
				wlf.SetCooldown(m_aFlashesCooldownDuration[n]);
				
				wl.AddLightningFlash(wlf);
			}
			
			tawme.AddLightning(wl);
			m_iCurFlashIndex += m_aFlashesStartTime.Count();
		}	
    }
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_iCurFlashIndex = 0;
		
		//SetEventMask(owner, EntityEvent.FRAME);
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if(!world)
			return;
		
		TimeAndWeatherManagerEntity tawme = world.GetTimeAndWeatherManager();
		if(!tawme)
			return;
		
		if(rpl && rpl.Role() == RplRole.Authority)
		{
			m_fRadiusKM = m_pRandomGenerator.RandFloatXY(m_fFlashMinVisibilityRadiusKM, m_fFlashMaxVisibilityRadiusKM);
			m_iNumFlashes = m_iMinFlashes + ((int)(m_pRandomGenerator.RandFloat01() * ((float)m_iMaxFlashes - (float)m_iMinFlashes)));
			
			//decide whether to use game time or global engine time, however,
			//if we use game time and auto-advancement is disabled, this will mess
			//with the lightnings, or any other effect based on game time.
			float curEngineTime = tawme.GetEngineTime();
			
			for(int n = 0; n < m_iNumFlashes; n++)
			{
				float flashStartTime = curEngineTime;
				float flashDuration = (m_pRandomGenerator.RandFloatXY(m_fFlashMinDurationMillis, m_fFlashMaxDurationMillis) * 0.001);
				float flashCooldownDuration = (m_pRandomGenerator.RandFloatXY(m_fFlashMinCooldownMillis, m_fFlashMaxCooldownMillis) * 0.001);
				
				curEngineTime += (flashDuration + flashCooldownDuration);
				
				m_aFlashesStartTime.Insert(flashStartTime);
				m_aFlashesDuration.Insert(flashDuration);
				m_aFlashesCooldownDuration.Insert(flashCooldownDuration);
			}
			
			OnFlashesRpl();
		}
	}
}
