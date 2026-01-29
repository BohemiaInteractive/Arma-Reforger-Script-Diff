/*!
\addtogroup DamageEffects
\{
*/

class SCR_PersistentDamageEffect : PersistentDamageEffect
{
	//------------------------------------------------------------------------------------------------
	//ALWAYS OVERRIDE LIKE THIS. ALWAYS OVERRIDE THIS FUNCTION
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	protected event override void OnDiag(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnDiag(dmgManager);

		if (IsProxy())
			return;
		
		if(!IsActive())
		{
			string text = text.Format("  Activeness: Paused");
			DbgUI.Text(text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Method called locally by SCR_DamageSufferingSystem to get relevant damage amount for this specific damage effect
	//! \return
	float GetCustomDamageValue()
	{
		return GetTotalDamage();
	}

	//------------------------------------------------------------------------------------------------
	//! Method meant to be called by replicated events to use local data for sound playback
	//! \param[in] dmgManager
	//! \return true sound playback should be considered as successful
	bool ExecuteSynchronizedSoundPlayback(notnull SCR_ExtendedDamageManagerComponent dmgManager);

	//------------------------------------------------------------------------------------------------
	//! Method called locally by SCR_DamageSufferingSystem when UseBatchProcessing() returns false
	//! \param[in] dmgManager
	void IndividualProcessing(notnull SCR_ExtendedDamageManagerComponent dmgManager);

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// DUE TO THE FACT THAT WHICH VERSION OF THE STATIC METHOD IS GOING TO BE USED IS DECIDED AT	//
	// COMPILE TIME, OVERRIDES OF THE FOLLOWING METHODS SHOULD BE TREATED AS IF THEY WERE STATIC,	//
	// DESPITE THE FACT THAT THEY ARE NOT, AS SCR_DamageSufferingSystem WILL CALL A CACHED 'VIRTUAL'//
	// INSTANCE OF A DAMGE EFFECT IN ORDER TO USE THESE												//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//------------------------------------------------------------------------------------------------
	//! Method called locally by SCR_DamageSufferingSystem to determine if it should group all instances of this damage effect type for processing
	//! \return true if batch processing should be used, otherwise false
	/*static*/ bool UseBatchProcessing();

	//------------------------------------------------------------------------------------------------
	//! Method called locally by SCR_DamageSufferingSystem to store any neccessary information about effect into a derivative of a SCR_BatchedDamageEffects for further processing
	//! \param[in,out] batchedDataContainer to which batched information must be stored
	//! \param[in] effect which is currently being batched
	/*static*/ void BatchData(inout SCR_BatchedDamageEffects batchedDataContainer, notnull SCR_PersistentDamageEffect effect);

	//------------------------------------------------------------------------------------------------
	//! Method called locally by SCR_DamageSufferingSystem to do a singular processing for this damage effect type
	//! \param[in] dmgManager for which this data is being processed
	//! \param[in] batchedDataContainer
	//! \param[in] isAuthority - RplRole.Authority
	//! \return true when processing was successful, otherwise false
	/*static*/ void BatchProcessing(notnull SCR_ExtendedDamageManagerComponent dmgManager, notnull SCR_BatchedDamageEffects batchedDataContainer, bool isAuthority);
}

/*!
\}
*/
