/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SignalsComponentClass: GameComponentClass
{
}

class SignalsComponent: GameComponent
{
	//! Updates the internal state. Changes outputs based on inputs.
	proto external void Update();
	//! Returns the index of a given signal input.
	proto external int GetSignalInIndex(string name);
	//! Returns the index of a given signal output.
	proto external int GetSignalOutIndex(string name);
	//! Set the input value of a signal. Returns true if changed.
	proto external bool SetSignalInValue(int index, float value);
	//! Returns the value of a signal input.
	proto external float GetSignalInValue(int index);
	//! Returns the value of a signal output.
	proto external float GetSignalOutValue(int index);
	proto external int GetSignalIns(out notnull array<string> ins);
	proto external int GetSignalOuts(out notnull array<string> outs);
}

/*!
\}
*/
