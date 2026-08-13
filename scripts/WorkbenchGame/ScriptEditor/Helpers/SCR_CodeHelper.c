class SCR_CodeHelper
{
	//------------------------------------------------------------------------------------------------
	//! Split line between indentation and content (trailing spaces included if any)
	//! \param[in] fullLine the line's content
	//! \param[out] indentation gets the left spacing (tabs and spaces)
	//! \param[out] content gets the text
	static void GetIndentAndLineContent(string fullLine, out string indentation, out string content)
	{
		int lineLength = fullLine.Length();
		if (lineLength < 1)
		{
			indentation = string.Empty;
			content = string.Empty;
			return;
		}

		int firstCharIndex = -1;
		for (int i; i < lineLength; i++)
		{
			string character = fullLine[i];
			if (character != SCR_StringHelper.TAB && character != SCR_StringHelper.SPACE)
			{
				firstCharIndex = i;
				break;
			}
		}

		if (firstCharIndex < 0)
		{
			indentation = fullLine;
			content = string.Empty;
			return;
		}

		if (firstCharIndex == 0)
		{
			indentation = string.Empty;
			content = fullLine;
			return;
		}

		indentation = fullLine.Substring(0, firstCharIndex);
		content = fullLine.Substring(firstCharIndex, lineLength - firstCharIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Get line content as array of strings. Cannot return an empty array
	//! \param[in] fullLine the line to analyse
	//! \param[out] indentation gets the left spacing (tabs and spaces)
	//! \param[out] pieces the line as split in code, string, comment parts - first element being indentation (empty when none). can be provided null, result is never null or empty
	static void GetIndentAndLineContentAsPieces(string fullLine, out string indentation, out array<string> pieces)
	{
		if (pieces)
			pieces.Clear();
		else
			pieces = {};

		string content;
		GetIndentAndLineContent(fullLine, indentation, content);

		if (!content) // !IsEmpty for perf
			return;

		bool isInCommentBlock;
		bool isInString;
		string currentContent;

		string prevChar, currChar;
		for (int i, contentLength = content.Length(); i < contentLength; i++)
		{
			currChar = content[i];

			//
			//	string management
			//
			if (!isInCommentBlock && currChar == SCR_StringHelper.DOUBLE_QUOTE)		// string management
			{
				if (isInString)
				{
					if (prevChar != SCR_StringHelper.ANTISLASH) // not escaped
					{
						pieces.Insert(currentContent + currChar);
						currentContent = string.Empty;
						isInString = false;
						prevChar = currChar;
						continue;
					}
				}
				else
				{
					pieces.Insert(currentContent);
					currentContent = currChar;
					isInString = true;
					prevChar = currChar;
					continue;
				}
			}
			else
			if (!isInString && currChar == SCR_StringHelper.SLASH)					// comment management
			{
				if (isInCommentBlock)
				{
					if (prevChar == "*")
					{
						pieces.Insert(currentContent + currChar);
						currentContent = string.Empty;
						isInCommentBlock = false;
						prevChar = currChar;
						continue;
					}
				}
				else if (i < contentLength - 1)
				{
					string nextChar = content[i + 1];
					if (nextChar == SCR_StringHelper.STAR)
					{
						pieces.Insert(currentContent);
						currentContent = currChar;
						isInCommentBlock = true;
						prevChar = currChar;
						continue;
					}
					else
					if (nextChar == SCR_StringHelper.SLASH) // the rest is comment
					{
						if (currentContent) // !IsEmpty for perf
							pieces.Insert(currentContent);

						pieces.Insert(content.Substring(i, contentLength - i));
						return;
					}
				}
			}

			currentContent += currChar;
			prevChar = currChar;

			if (i == contentLength - 1 && currentContent) // !IsEmpty for perf
				pieces.Insert(currentContent);
		}
	}
}
