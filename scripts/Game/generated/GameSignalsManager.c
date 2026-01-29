/*
===========================================
Do not modify, this script is generated
===========================================
*/

class GameSignalsManager
{
	/*!
	Add or Find a signal which is always local to the machine
	\param signalName Name of the signal to add / find
	\param value Value the newly created signal is going to have.
	\return index of signal. Returns -1 in case of error.
	*/
	proto external int AddOrFindSignal(string signalName, float value = 0);
	/*!
	Add or find a signal which is synchronized over the network automatically.
	\param signalName Name of the signal to add / find
	\param valueThreshold How much the value has to changed compared to the previous one in order to sync it over the network.
	\param blendSpeed How fast the value is interpolated when synced over the network. It also influences how much time can
									 pass before the values is synced over the network again. Time in seconds = 1.0/blendSpeed.
	\param value Value the newly created signal is going to have.
	\param compressionFunc Compression type to use to compress the value when transfering it over the network.
	\warning This has to be performed on authority. If an attempt to register a MP signal is made on proxies an ordinary signal
					is created instead. If authority registers a signal it makes sure to synchronize to state with proxies.
	\return Index of the signal. Returns -1 in case of error.
	*/
	proto external int AddOrFindMPSignal(string signalName, float valueThreshold, float blendSpeed, float value = 0, SignalCompressionFunc compressionFunc = SignalCompressionFunc.None);
	/*!
	Find a signal with given name
	\param signalName Name of the signal to find
	\return Index of the signal. Returns -1 in case of error.
	*/
	proto external int FindSignal(string signalName);
	//! Set the signal with an index of \param index to \param value
	proto external void SetSignalValue(int index, float value);
	/*!
	Get value of the signal with index \param index
	\return Value of the siganl. Returns 0.0f if the signal is not found.
	*/
	proto external float GetSignalValue(int index);
}
