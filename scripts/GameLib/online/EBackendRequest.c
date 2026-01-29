//! Backend requests
enum EBackendRequest
{
	// To be removed after proper alternative is provided
	EBREQ_GAME_CharacterGet,
	EBREQ_GAME_DevCharacterGet,
	EBREQ_GAME_DevCharacterUpdate,
	EBREQ_GAME_CharacterUpdateS2S,

	EBREQ_WORKSHOP_GetAsset,
}