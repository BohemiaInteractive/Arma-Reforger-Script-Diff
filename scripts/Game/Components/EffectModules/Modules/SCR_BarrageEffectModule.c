[BaseContainerProps(configRoot: true)]
class SCR_BarrageEffectsModule : SCR_EffectsModule
{
	[Attribute("5", params: "1 inf 1", desc: "Min amount of Projectiles in barrage", category: "Barrage")]
	protected int m_iMinProjectilesEachBarrage;

	[Attribute("5", params: "1 inf 1", desc: "Max amount of Projectiles in barrage", category: "Barrage")]
	protected int m_iMaxProjectilesEachBarrage;

	[Attribute("1", params: "0 inf 0.05", desc: "Min delay between Projectiles in barrage", category: "Barrage")]
	protected float m_fMinProjectileDelay;

	[Attribute("2", params: "0 inf 0.05", desc: "Max delay between Projectiles in barrage", category: "Barrage")]
	protected float m_fMaxProjectileDelay;

	[Attribute("1", desc: "How many barrages there should be", category: "Barrage")]
	protected int m_iBarrageAmount;

	[Attribute("5", desc: "Min delay between barrages", params: "0 inf 0", category: "Barrage")]
	protected float m_fMinBarrageDelay;

	[Attribute("10", desc: "Max delay between barrages", params: "0 inf 0", category: "Barrage")]
	protected float m_fMaxBarrageDelay;

	[Attribute(desc: "If true allows the module to be paused", category: "General")]
	protected bool m_bCanPause;

	[Attribute(desc: "If true allows the module to loop aka continuously execute", category: "General")]
	protected bool m_bCanLoop;

	[Attribute(desc: "If true will continuously execute module", category: "General")]
	protected bool m_bIsLooping;

	//~ How many projectiles for current barrage are left
	protected int m_iProjectilesLeftInBarrage = -1;

	//~ How many projectiles where set at the start of the barrage
	protected int m_iTotalProjectilesInBarrage = -1;

	//~ Current waiting time left before the next projectile is spawned
	protected float m_fCurrentProjectileDelay;

	//~ How many barrages are left
	protected int m_iBarragesLeft = -1;

	//~ Make sure m_fMinProjectileDelay and m_fMaxProjectileDelay attribute step is set to this value. Never set to 0 or less
	protected const float UPDATE_TIME = 0.05;

	//~ If the system is currently waiting for the next barrage. This makes sure the Callque is removed on delete
	protected bool m_bWaitingForNextBarrage;

	//------------------------------------------------------------------------------------------------
	override void Init(notnull IEntity owner, notnull SCR_EffectsModuleComponent effectModuleParent)
	{
		super.Init(owner, effectModuleParent);

		if (UPDATE_TIME <= 0)
			Debug.Error2("SCR_BarrageEffectsModule", "'UPDATE_TIME' is set to 0 or less!");

		if (m_bIsLooping && !m_bCanLoop)
		{
			Print("'SCR_EffectsModule', Effect is set to looping but it cannot loop! So looping is set false", LogLevel.WARNING);
			SetLooping(false);
		}

		if (m_fMinProjectileDelay > m_fMaxProjectileDelay)
		{
			Print("'SCR_EffectsModule', m_fMinProjectileDelay is greater than m_fMaxProjectileDelay! This is not allowed. m_fMinProjectileDelay is set equal to m_fMaxProjectileDelay", LogLevel.WARNING);
			m_fMinProjectileDelay = m_fMaxProjectileDelay;
		}

		if (m_fMinBarrageDelay > m_fMaxBarrageDelay)
		{
			Print("'SCR_EffectsModule', m_fMinBarrageDelay is greater than m_fMaxBarrageDelay! This is not allowed. m_fMinBarrageDelay is set equal to m_fMaxBarrageDelay", LogLevel.WARNING);
			m_fMinBarrageDelay = m_fMaxBarrageDelay;
		}

		if (m_iMinProjectilesEachBarrage > m_iMaxProjectilesEachBarrage)
		{
			Print("'SCR_EffectsModule', m_iMinProjectilesEachBarrage is greater than m_fMaxBarrageDelay! This is not allowed. m_iMinProjectilesEachBarrage is set equal to m_iMaxProjectilesEachBarrage", LogLevel.WARNING);
			m_iMinProjectilesEachBarrage = m_iMaxProjectilesEachBarrage;
		}
	}

	//------------------------------------------------------------------------------------------------
	//~ (Server Only)
	override void OnModuleExecute()
	{
		// May already be set from e.g. save-data
		if (m_iBarragesLeft == -1)
			m_iBarragesLeft = m_iBarrageAmount;

		if (m_iProjectilesLeftInBarrage == -1)
		{
			m_iTotalProjectilesInBarrage = Math.RandomIntInclusive(m_iMinProjectilesEachBarrage, m_iMaxProjectilesEachBarrage);
			m_iProjectilesLeftInBarrage = m_iTotalProjectilesInBarrage;
		}

		StartBarrage();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanPause()
	{
		return m_bCanPause && !m_bCheckForDelete;
	}

	//------------------------------------------------------------------------------------------------
	override void SetPaused(bool paused)
	{
		if (paused == IsPaused())
			return;

		super.SetPaused(paused);

		if (m_bIsPaused)
			m_EffectsModuleParent.CancelModuleServer();
		else
			StartBarrage();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanLoop()
	{
		return m_bCanLoop && !m_bCheckForDelete;
	}

	//------------------------------------------------------------------------------------------------
	override void SetLooping(bool loop)
	{
		if (!CanLoop())
			return;

		m_bIsLooping = loop;
	}

	//------------------------------------------------------------------------------------------------
	//~ (Server Only)
	override void CancelModule()
	{
		super.CancelModule();

		if (m_iProjectilesLeftInBarrage > 0)
		{
			GetGame().GetCallqueue().Remove(BarrageUpdateLoop);
		}

		if (m_bWaitingForNextBarrage)
		{
			m_bWaitingForNextBarrage = false;
			GetGame().GetCallqueue().Remove(StartBarrage);
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetTotalProjectilesInBarrage()
	{
		return m_iTotalProjectilesInBarrage;
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_EffectsModuleComponentSerializer)]
	protected void SetTotalProjectilesInBarrage(int projectiles)
	{
		m_iTotalProjectilesInBarrage = projectiles;
	}

	//------------------------------------------------------------------------------------------------
	int GetProjectilesLeftInBarrage()
	{
		return m_iProjectilesLeftInBarrage;
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_EffectsModuleComponentSerializer)]
	protected void SetProjectilesLeftInBarrage(int projectiles)
	{
		m_iProjectilesLeftInBarrage = projectiles;
	}

	//------------------------------------------------------------------------------------------------
	int GetBarragesLeft()
	{
		return m_iBarragesLeft;
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_EffectsModuleComponentSerializer)]
	protected void SetBarragesLeft(int barrages)
	{
		m_iBarragesLeft = barrages;
	}

	//------------------------------------------------------------------------------------------------
	//~ Start executing the barrage (Server Only)
	protected void StartBarrage()
	{
		if (m_bWaitingForNextBarrage)
		{
			m_bWaitingForNextBarrage = false;
			GetGame().GetCallqueue().Remove(StartBarrage);
		}

		if (m_iProjectilesLeftInBarrage <= 0)
		{
			m_iTotalProjectilesInBarrage = SCR_Math.RandomIntInclusive(m_iMinProjectilesEachBarrage, m_iMaxProjectilesEachBarrage);
			m_iProjectilesLeftInBarrage = m_iTotalProjectilesInBarrage;
		}

		//~ No time so spawn a entity each frame
		if (m_fMinProjectileDelay == 0 && m_fMaxProjectileDelay == 0)
		{
			m_fCurrentProjectileDelay = 0;
			GetGame().GetCallqueue().CallLater(BarrageUpdateLoop, 0, true);
			return;
		}

		//~ Time is set so calculate delay
		m_fCurrentProjectileDelay = SCR_Math.RandomFloatInclusive(m_fMinProjectileDelay, m_fMaxProjectileDelay);
		GetGame().GetCallqueue().CallLater(BarrageUpdateLoop, UPDATE_TIME * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	//~ Update loop that Spawns an new entity every time the m_fCurrentProjectileDelay reaches 0 (Server Only)
	protected void BarrageUpdateLoop()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world.IsGameTimePaused())
			return;

		if (!m_Owner || m_Owner.IsDeleted())
		{
			Print("SCR_BarrageEffectsModule.BarrageUpdateLoop: Effect module owner no longer exists. Spawning of " + m_sModuleEntityPrefab + " aborted.", LogLevel.WARNING);
			GetGame().GetCallqueue().Remove(BarrageUpdateLoop);
			return;
		}

		//~ Update time
		m_fCurrentProjectileDelay -= UPDATE_TIME;

		//~ Do not execute yet
		if (m_fCurrentProjectileDelay > 0)
			return;

		m_fCurrentProjectileDelay = SCR_Math.RandomFloatInclusive(m_fMinProjectileDelay, m_fMaxProjectileDelay);

		//~ Spawn effect
		m_EffectsModuleParent.SpawnEffectEntity();
		m_iProjectilesLeftInBarrage--;

		//~ Done executing
		if (m_iProjectilesLeftInBarrage <= 0)
		{
			GetGame().GetCallqueue().Remove(BarrageUpdateLoop);
			BarrageDone();
		}
		//~ Instant call close together
		else if (m_fCurrentProjectileDelay < UPDATE_TIME && m_fMaxProjectileDelay > 0)
		{
			BarrageUpdateLoop();
		}
	}

	//------------------------------------------------------------------------------------------------
	//~ The barrage is done, check if there are more barrages, if so wait and start again otherwise set module done (Server Only)
	protected void BarrageDone()
	{
		if (!m_bIsLooping)
			m_iBarragesLeft--;

		if (m_iBarragesLeft <= 0)
		{
			//~ Done executing
			m_EffectsModuleParent.DoneExecutingModule();
			return;
		}

		//~ Reset the Zone data target positions
		m_ModuleZoneData.ResetPositions(this);

		m_bWaitingForNextBarrage = true;
		GetGame().GetCallqueue().CallLater(StartBarrage, SCR_Math.RandomFloatInclusive(m_fMinBarrageDelay, m_fMaxBarrageDelay) * 1000);
	}
}
