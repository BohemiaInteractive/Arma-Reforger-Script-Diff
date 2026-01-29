class AnimPhysCommandScripted
{
	//! constructor is protected - we don't wnat instances of this "abstract" class 
	protected void AnimPhysCommandScripted();

	//! virtual to be overridden
	//! called when command starts
	void OnActivate();

	//! called when command ends
	void OnDeactivate();

	//------------------------------------------------------------------------
	// Flags - usable anywhere
	//------------------------------------------------------------------------

	//! autodelete flag (when set, agent will delete the command after returning PreAnimUpdate or PostPhysicsUpdate automaticaly)
	//! usable for additive commands
	proto native bool IsFlagAutoDelete();
	proto native void SetFlagAutoDelete(bool pV);
	
	//! this terminates human command script and shows CommandHandler(  ... pCurrentCommandFinished == true );
	proto native void SetFlagFinished(bool pFinished);
	proto native bool IsFlagFinished();

	//------------------------------------------------------------------------
	// Graph controls - PreAnim Update
	//------------------------------------------------------------------------
	
	//! override this !
	//! called before any animation is processed
	//! here change animation values, add animation commands	
	void 	PreAnimUpdate(float pDt);

	proto native void PreAnim_CallCommand(int pCommand, int pParamInt, float pParamFloat);
	proto native void PreAnim_CallCommand4I(int pCommand, int pParamInt1, int pParamInt2, int pParamInt3, int pParamInt4, float pParamFloat);
	proto native void PreAnim_SetFloat(int pVar, float pFlt);
	proto native void PreAnim_SetInt(int pVar, int pInt);
	proto native void PreAnim_SetBool(int pVar, bool pBool);
	proto native void PreAnim_SetAttachment(string bindingName, string animGraphResourceName, string animInstanceResourceName, string startingNodeName);
				
	//------------------------------------------------------------------------
	// Graph states/modifications - PrePhys Update
	//------------------------------------------------------------------------
	
	//! override this !
	//! after animation is processed, before physics is processed
	void 	PrePhysUpdate(float pDt);

	//! script function usable in PrePhysUpdate
	proto native bool PrePhys_IsEvent(int pEvent);
	proto native bool PrePhys_IsTag(int pTag);
	proto        bool PrePhys_GetPredictionEval(int pPrediction, out int pGlobalBoneIndex, out bool pIsTranslation, out bool pIsRotation, out vector pOutTransl, out float pOutRot[4], out float pTimeToPrediction, out float pNormTimeInAnims);
	proto native bool PrePhys_GetTranslation(out vector pOutTransl);
	proto native bool PrePhys_GetRotation(out float pOutRot[4]);
	proto native void PrePhys_SetTranslation(vector pInTransl);
	proto native void PrePhys_SetRotation(float pInRot[4]);

	//------------------------------------------------------------------------
	// Graph states/modifications - PostPhys Update
	//------------------------------------------------------------------------
	
	//! override this !
	//! final adjustment of physics state (after physics was applied)
	//! returns true if command continues running / false if command should end (or you can use SetFlagFinished(true))
	bool PostPhysUpdate(float pDt);

	//! script function usable in PostPhysUpdate
	proto native void PostPhys_GetPosition(out vector pOutTransl);
	proto native void PostPhys_GetRotation(out float pOutRot[4]);
	proto native void PostPhys_SetPosition(vector pInTransl);
	proto native void PostPhys_SetRotation(float pInRot[4]);
	proto native void PostPhys_LockRotation();
}
