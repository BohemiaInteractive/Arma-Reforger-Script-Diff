/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Radio
\{
*/

class BaseRadioMessage: ScriptAndConfig
{
	/*!
	Encryption key used to encrypt message.
	Empty string means no encryption.
	\return Encryption key as string
	*/
	proto external string GetEncryptionKey();
	/*!
	Sets encryption of the message to given string
	*/
	proto external void SetEncryptionKey(string key);
}

/*!
\}
*/
