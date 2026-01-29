/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SoundOutputStateComponentClass: GenericComponentClass
{
}

class SoundOutputStateComponent: GenericComponent
{
	/* Get list of 'signals. '*/
	proto external void GetSignalNames(out array<string> signalNames);
	/* Convert signal name to index. */
	proto external int GetSignalIndex(string name);
	/* Set signal value by 'name'. */
	proto external void SetSignalValueName(string signal, float value);
	/* Set signal value by 'index'. */
	proto external void SetSignalValue(int index, float value);
	/* Enable debug mode. */
	proto external void SetDebug(bool value);
}

/*!
\}
*/
