[ComponentEditorProps(category: "GameScripted/Destruction", description: "Auto-plays progressive morph destruction animation on entity spawn")]
class SCR_TapeDestructionComponentClass : ScriptComponentClass
{
}

//! Attach to a spawned destruction replacement prefab. When the original destructible entity
//! (e.g. intact safety tape) is destroyed, the destruction system spawns the replacement prefab
//! at the same transform. This component auto-plays a sequence of mesh morphs on spawn
//! using progressive (additive) activation.
//!
//! The morph names follow the baker convention: {prefix}{frame:04d}
//! e.g. Tape_Destruction0004, Tape_Destruction0006, Tape_Destruction0011, ...
//!
//! Each morph stores only the INCREMENTAL change from the previous frame (baked with
//! "Progressive (Additive)" mode in the Blender baker). At runtime, all completed morphs
//! stay at weight 1.0 and only the current morph transitions 0 to 1. The result is a
//! smooth accumulation of deformation steps.
//!
//! Gameplay flow:
//!   1. Original entity takes damage and is destroyed.
//!   2. Destruction system spawns the replacement prefab at the same transform.
//!   3. Component discovers morph targets by probing with the prefix.
//!   4. Morphs are progressively activated -- no external trigger.
//!   5. After all morphs reach 1.0 the component goes idle (zero per-frame cost).
class SCR_TapeDestructionComponent : ScriptComponent
{
	[Attribute("Tape_Destruction", desc: "Prefix of morph target names (baker convention: prefix + 4-digit frame number)")]
	protected string m_sPrefix;

	[Attribute("250", UIWidgets.Slider, "Highest frame number to probe when discovering morphs", "1 9999 1")]
	protected int m_iProbeMax;

	[Attribute("0.5", UIWidgets.Slider, "Total duration of the destruction sequence in seconds", "0.05 5.0 0.01")]
	protected float m_fDuration;

	// Discovered morph names in order.
	protected ref array<string> m_aMorphNames;

	// Playback state.
	protected float m_fElapsed;

	//------------------------------------------------------------------------------------------------
	//! Probes the entity for morphs matching the prefix and builds the ordered list.
	protected void DiscoverMorphs(IEntity owner)
	{
		m_aMorphNames = {};

		Animation anim = owner.GetAnimation();
		if (!anim)
		{
			Print("[TapeDestruction] No Animation on entity", LogLevel.WARNING);
			return;
		}

		for (int f = 0; f <= m_iProbeMax; f++)
		{
			string name = string.Format("%1%2", m_sPrefix, f.ToString(4));
			if (anim.SetMorphState(name, 0.0))
				m_aMorphNames.Insert(name);
		}

		int count = m_aMorphNames.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Applies progressive morph weights based on a continuous progress value (0..1).
	//! All morphs before the current step are fully active (1.0).
	//! The current morph transitions 0→1.
	//! All morphs after remain at 0.
	protected void ApplyProgress(IEntity owner, float progress)
	{
		Animation anim = owner.GetAnimation();
		if (!anim)
			return;

		int count = m_aMorphNames.Count();
		if (count == 0)
			return;

		// Map progress (0..1) across all morph steps.
		// Each morph represents one step. At progress=1.0 all morphs are at 1.0.
		float fStep = progress * count;
		int completedSteps = Math.ClampInt(Math.Floor(fStep), 0, count);
		float currentBlend = fStep - Math.Floor(fStep);

		// Set all completed morphs to 1.0.
		for (int i = 0; i < completedSteps && i < count; i++)
		{
			anim.SetMorphState(m_aMorphNames[i], 1.0);
		}

		// Current morph transitions 0→1.
		if (completedSteps < count)
		{
			anim.SetMorphState(m_aMorphNames[completedSteps], currentBlend);

			// Zero remaining morphs.
			for (int i = completedSteps + 1; i < count; i++)
			{
				anim.SetMorphState(m_aMorphNames[i], 0.0);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		int count = m_aMorphNames.Count();
		if (count == 0)
		{
			ClearEventMask(owner, EntityEvent.FRAME);
			return;
		}

		m_fElapsed += timeSlice;

		float progress = m_fElapsed / m_fDuration;
		if (progress > 1.0)
			progress = 1.0;

		ApplyProgress(owner, progress);

		if (progress >= 1.0)
		{
			ClearEventMask(owner, EntityEvent.FRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		DiscoverMorphs(owner);

		m_fElapsed = 0.0;

		if (m_aMorphNames.IsEmpty())
		{
			PrintFormat("[TapeDestruction] WARNING: No morphs found with prefix '%1'", m_sPrefix, level: LogLevel.WARNING);
			return;
		}

		ApplyProgress(owner, 0.0);
		SetEventMask(owner, EntityEvent.FRAME);
	}
}
