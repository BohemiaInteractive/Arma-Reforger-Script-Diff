/*!
\defgroup Debug Debug utilities
\{
*/

//! Prints content of variable to console/log.
proto void Print(void var, LogLevel level = LogLevel.NORMAL);


/*!
Prints formated text to console/log.
\code
	string c = "Peter";
	PrintFormat("Hello %1, how are you?", c); // prints "Hello 'Peter', how are you?"
\endcode
*/
proto void PrintFormat(string fmt, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL, LogLevel level = LogLevel.NORMAL);

/*!
\}
*/
