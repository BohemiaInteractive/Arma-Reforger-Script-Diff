/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseRadioComponentClass: GameComponentClass
{
}

class BaseRadioComponent: GameComponent
{
	ref ScriptInvoker<BaseTransceiver, int, int> m_OnTansceiverFrequencyChangedInvoker = new ScriptInvoker<BaseTransceiver, int, int>();
	ref ScriptInvoker<BaseTransceiver, float, float> m_OnTansceiverRangeChangedInvoker = new ScriptInvoker<BaseTransceiver, float, float>();
	ref ScriptInvoker<bool> m_OnPowerChanged = new ScriptInvoker<bool>();
	ref ScriptInvoker<string> m_OnEncryptionKeyChanged = new ScriptInvoker<string>();

	/*!
	\return Entity this component is atached to.
	*/
	proto external IEntity GetOwner();
	//! \return Number of transceivers.
	proto external int TransceiversCount();
	//! \return Transceiver with given index.
	proto external BaseTransceiver GetTransceiver(int idx);
	/*!
	\param key Key used for transmission encryption
	*/
	proto external void SetEncryptionKey(string key);
	/*!
	\return Key used for transmission encryption
	*/
	proto external string GetEncryptionKey();
	/*!
	Sets the power state of whole radio.
	*/
	proto external void SetPower(bool powered);
	/*!
	\return Power state of whole radio
	*/
	proto external bool IsPowered();
	/*
	* \return true if radio is attached to the EditorManagerEntity.
	*/
	proto external bool IsEditorRadio();
	/*!
	* Set frequency of transceiver and sync with server.
	*/
	proto external void SetTransceiverFrequency(BaseTransceiver transceiver, int freq);

	// callbacks

	//! Event invoked on client and server when frequency is changed
	event protected void OnTansceiverFrequencyChanged(BaseTransceiver transceiver, int oldFreq, int newFreq) { m_OnTansceiverFrequencyChangedInvoker.Invoke(transceiver, oldFreq, newFreq); };
	//! Event invoked on client and server when range is changed
	event protected void OnTansceiverRangeChanged(BaseTransceiver transceiver, float oldRange, float newRange) { m_OnTansceiverRangeChangedInvoker.Invoke(transceiver, oldRange, newRange); };
	//! Event invoked on client and server when power is changed
	event protected void OnPowerChanged(bool powered) { m_OnPowerChanged.Invoke(powered); };
	//! Event invoked on client and server when encryption key is changed
	event protected void OnEncryptionKeyChanged(string newKey) { m_OnEncryptionKeyChanged.Invoke(newKey); };
}

/*!
\}
*/
