/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class VoNComponentClass: GameComponentClass
{
}

/*!
Component responsible for recording and playback of voice over network.
*/
class VoNComponent: GameComponent
{
	/*!
	Request to starts/stop audio capturing.
	\return if the request suceeded
	*/
	proto external bool SetCapture(bool isCapturing);
	//! Sets communication method used.
	proto external void SetCommMethod(ECommMethod type);
	proto external ECommMethod GetCommMethod();
	//! Sets the transceiver used for VoN transmission
	proto external void SetTransmitRadio(BaseTransceiver transceiver);
	proto external BaseTransceiver GetTransmitRadio();
	//! Transfer AI sound message through VoN.
	proto external void SoundEventPriority(string eventname, array<float> values, int priority, bool ignoreQueue = false);
	/*!
	Connects this VoNComponent to the VoNSystem and tells this VoNComponent that is is used by an editor. This is required for the VoNComponent to be properly updated.
	*/
	proto external void ConnectEditorToVoNSystem(int playerId);
	/*!
	Disconnects this VoNComponent from the VoNSystem. This will stop de VoNSystem from updating this VoNComponent.
	*/
	proto external void DisconnectEditorFromVoNSystem();

	// callbacks

	/*!
	Event polled each frame while component is recording audio
	\param transmitter Transceiver used. Null if only direct speech.
	*/
	event protected void OnCapture(BaseTransceiver transmitter);
	/*!
	Event invoked when component receives audio data for playback
	\param playerId Senders PlayerId
	\param isSenderEditor true if sender is active editor.
	\param receiver Receiving transceiver
	\param frequency Frequency in kHz, on which the transmission came from. Can be different from the frequency which transceiver have set. This is very questionable and could be changed in future.
	\param quality Value in range <0,1> describing quality of the transmission
	*/
	event protected void OnReceive(int playerId, bool isSenderEditor, BaseTransceiver receiver, int frequency, float quality);
	/*!
	Event invoked on the server, each time VoN is used.
	*/
	event protected void OnVoNUsed(int senderId);
	/*!
	returns the SCR_EditorManagerEntity for a given player id if it exists.
	*/
	event protected IEntity GetEditorEntity(int playerId);
	/*!
	returns the position of the camera used by the editor for a given player id, if it exists.
	*/
	event protected vector GetEditorWorldLocation(int playerId);
	/*!
	returns true if the entity is a SCR_EditorManagerEntity that is opened.
	*/
	event protected bool IsEntityActiveEditor(IEntity entity);
}

/*!
\}
*/
