/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class EditBoxWidget: UIWidget
{
	proto external bool IsInWriteMode();
	proto external void SetText(string str);
	proto external void SetPlaceholderText(string str);
	proto external void SetVirtualKeyboardTitle(string str);
	proto external void SetVirtualKeyboardDesc(string str);
	proto external void ActivateWriteMode();
	proto string GetObfuscationChar();
	/*!
	Set a character to display instead of every character in the input
	Only the first character of the string is used.
	To turn off obfuscation, pass an empty string "".
	*/
	proto void SetObfuscationChar(string obfuscationChar);
	proto string GetText();
	proto string GetPlaceholderText();
}

/*!
\}
*/
