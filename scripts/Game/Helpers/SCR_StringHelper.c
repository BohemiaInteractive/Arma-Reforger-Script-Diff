class SCR_StringHelper
{
	static const string LOWERCASE = "abcdefghijklmnopqrstuvwxyz";
	static const string UPPERCASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static const string LETTERS = LOWERCASE + UPPERCASE;
	static const string DIGITS = "0123456789";
	static const string ALPHANUMERICAL = LETTERS + DIGITS;
	static const string UNDERSCORE = "_";
	static const string ALPHANUMERICAL_U = ALPHANUMERICAL + UNDERSCORE;
	static const string DASH = "-";
	static const string COLON = ":";
	static const string SEMICOLON = ";";
	static const string COMMA = ",";
	static const string SPACE = " ";
	static const string STAR = "*";
	static const string POUND = "#";
	static const string HASHTAG = POUND;
	static const string EQUALS = "=";
	static const string QUESTION_MARK = "?";
	static const string EXCLAMATION_MARK = "!";
	static const string DOUBLE_SPACE = SPACE + SPACE;
	static const string QUADRUPLE_SPACE = DOUBLE_SPACE + DOUBLE_SPACE;
	static const string SINGLE_QUOTE = "'";
	static const string DOUBLE_QUOTE = "\"";
	static const string TAB = "\t";
	static const string LINE_RETURN = "\n";
	static const string SLASH = "/";
	static const string DOUBLE_SLASH = SLASH + SLASH;
	static const string ANTISLASH = "\\";
	static const string DOUBLE_ANTISLASH = ANTISLASH + ANTISLASH;
	static const string LIPSUM = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
	protected static const string TRANSLATION_KEY_CHARS = UNDERSCORE + DASH;

	protected static const int MIN_LC = 97;		// a
	protected static const int MAX_LC = 122;	// z

	protected static const int MIN_UC = 65;		// A
	protected static const int MAX_UC = 90;		// Z

	protected static const int MIN_DIGIT = 48;	// 0
	protected static const int MAX_DIGIT = 57;	// 9

	//------------------------------------------------------------------------------------------------
	//! Check if the provided input contains any digit
	//! \param[in] input
	//! \return true if input contains any digit, false otherwise
	static bool ContainsDigit(string input)
	{
		for (int i, len = input.Length(); i < len; i++)
		{
			int asciiValue = input[i].ToAscii();
			if (asciiValue >= MIN_DIGIT && asciiValue <= MAX_DIGIT)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provided input contains any uppercase character (A-Z)
	//! \param[in] input
	//! \return true if input contains any uppercase character, false otherwise
	static bool ContainsUppercase(string input)
	{
		for (int i, len = input.Length(); i < len; i++)
		{
			int asciiValue = input[i].ToAscii();
			if (asciiValue >= MIN_UC && asciiValue <= MAX_UC)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provided input contains any lowercase character (a-z)
	//! \param[in] input
	//! \return true if input contains any lowercase character, false otherwise
	static bool ContainsLowercase(string input)
	{
		for (int i, len = input.Length(); i < len; i++)
		{
			int asciiValue = input[i].ToAscii();
			if (asciiValue >= MIN_LC && asciiValue <= MAX_LC)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the amount of times a needle is found in the haystack.
	//! e.g looking for "AA" in "AAAAA" will find it twice.
	//! \param[in] haystack the string in which to search
	//! \param[in] needle the string to find
	//! \param[in] caseInsensitive if set to true, the search will be case-insensitive (e.g "A" will match "a" and vice-versa)
	//! \return the amount of needle occurrences in haystack, 0 if haystack or needle is empty
	static int CountOccurrences(string haystack, string needle, bool caseInsensitive = false)
	{
		if (needle.IsEmpty() || haystack.IsEmpty())
			return 0;

		if (caseInsensitive)
		{
			needle.ToLower();
			haystack.ToLower();
		}

		int needleLength = needle.Length();
		int haystackLength = haystack.Length();

		if (needleLength > haystackLength)
			return 0;

		int result;
		int searchIndex;
		while (searchIndex < haystackLength)
		{
			int resultIndex = haystack.IndexOfFrom(searchIndex, needle);
			if (resultIndex < 0)
				break;

			result++;
			searchIndex = resultIndex + needleLength;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input the input string
	//! \param[in] characters the characters to either respect or remove depending on useCharactersAsBlacklist
	//! \param[in] useCharactersAsBlacklist false for characters to be a whitelist, true for a blacklist
	//! \return the resulting string
	static string Filter(string input, string characters, bool useCharactersAsBlacklist = false)
	{
		if (input.IsEmpty() || (!useCharactersAsBlacklist && characters.IsEmpty()))
			return string.Empty;

		string result;
		for (int i, length = input.Length(); i < length; i++)
		{
			string letter = input[i];
			if (characters.Contains(letter) != useCharactersAsBlacklist) // if it contains but shouldn't, or vice-versa
				result += letter;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] format
	//! \param[in] input must be ASCII
	static bool IsFormat(SCR_EStringFormat format, string input)
	{
		switch (format)
		{
			case SCR_EStringFormat.ALPHABETICAL_UC:		return CheckCharacters(input, false, true, false);
			case SCR_EStringFormat.ALPHABETICAL_LC:		return CheckCharacters(input, true, false, false);
			case SCR_EStringFormat.ALPHABETICAL_I:		return CheckCharacters(input, true, true, false);
			case SCR_EStringFormat.ALPHANUMERICAL_UC:	return CheckCharacters(input, false, true, true);
			case SCR_EStringFormat.ALPHANUMERICAL_LC:	return CheckCharacters(input, true, false, true);
			case SCR_EStringFormat.ALPHANUMERICAL_I:	return CheckCharacters(input, true, true, true);
			case SCR_EStringFormat.DIGITS_ONLY:			return CheckCharacters(input, false, false, true);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the provided string respects all the limitations
	//! \param[in] allowLC allow LowerCase characters (abc..xyz)
	//! \param[in] allowUC allow UpperCase characters (ABC..XYZ)
	//! \param[in] allowDigits allow all numbers (012..789)
	//! \param[in] allowUnderscore allow underscore (_)
	//! \return whether or not input respects allowed characters filters
	static bool CheckCharacters(string input, bool allowLC, bool allowUC, bool allowDigits, bool allowUnderscore = false)
	{
		if (input.IsEmpty())
			return false;

		for (int i, len = input.Length(); i < len; i++)
		{
			int asciiValue = input[i].ToAscii();
			if (!(
				(allowLC && asciiValue >= MIN_LC && asciiValue <= MAX_LC) ||
				(allowUC && asciiValue >= MIN_UC && asciiValue <= MAX_UC) ||
				(allowDigits && asciiValue >= MIN_DIGIT && asciiValue <= MAX_DIGIT) ||
				(allowUnderscore && asciiValue == 95)
			))
				return false;
		}

		return true;
	}

//	//------------------------------------------------------------------------------------------------
//	//! old method, 4-5× slower but allows for non-ASCII values, not useful for now
//	protected static bool CheckCharactersOld(string input, bool allowLC, bool allowUC, bool allowDigits, bool allowUnderscore)
//	{
//		string filter;
//		if (allowLC)
//			filter += LOWERCASE;
//		if (allowUC)
//			filter += UPPERCASE;
//		if (allowDigits)
//			filter += DIGITS;
//		if (allowUnderscore)
//			filter += UNDERSCORE;
//
//		if (filter.IsEmpty())
//			return false;
//
//		for (int i, len = input.Length(); i < len; i++)
//		{
//			if (!filter.Contains(input[i]))
//				return false;
//		}
//
//		return true;
//	}

	//------------------------------------------------------------------------------------------------
	//! format with string arguments in the form of an array
	//! \param[in] format with %1, %2 etc
	//! \param[in] arguments array
	//! \return string.Format'ted string (with max 9 arguments)
	static string Format(string input, notnull array<string> arguments)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(input))
			return input;

		if (!input.Contains("%"))
			return input;

		switch (arguments.Count())
		{
			case 0: return string.Format(input);
			case 1: return string.Format(input, arguments[0]);
			case 2: return string.Format(input, arguments[0], arguments[1]);
			case 3: return string.Format(input, arguments[0], arguments[1], arguments[2]);
			case 4: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3]);
			case 5: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
			case 6: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
			case 7: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6]);
			case 8: return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7]);
		}

		// 9 and more
		return string.Format(input, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8]);
	}

	//------------------------------------------------------------------------------------------------
	//! Turns e.g m_bIsValid to "Is Valid", m_bUsesAMagazine to "Uses A Magazine", m_bExportToPDFDocument to "Export To PDF Document"
	//! \param[in] valueName
	//! \return space-separated words without resourceName extension
	static string FormatValueNameToUserFriendly(string valueName)
	{
		if (IsEmptyOrWhiteSpace(valueName))
			return string.Empty;

		int asciiValue;

		if (valueName.StartsWith("m_") || valueName.StartsWith("s_"))
		{
			int length = valueName.Length();
			if (length < 3)
				return string.Empty;

			valueName = valueName.Substring(2, valueName.Length() - 2);

			// is first after prefix letter lowercase
			asciiValue = valueName[0].ToAscii();
			if (asciiValue >= MIN_LC && asciiValue <= MAX_LC)
			{
				if (length > 3)
					valueName = valueName.Substring(1, valueName.Length() - 1);
			}

			if (valueName.IsEmpty())
				return string.Empty;
		}

		array<string> pieces = {};
		string piece;
		string currChar;
		int prevCharType; // 0 = LC, 1 = UC, 2 = NUM
		for (int i, len = valueName.Length(); i < len; i++)
		{
			currChar = valueName[i];

			asciiValue = currChar.ToAscii();
			if (asciiValue >= MIN_LC && asciiValue <= MAX_LC)		// lowercase
			{
				if (prevCharType < 2) // prev can be lower or upper
				{
					piece += currChar;
				}
				else
				{
					if (!piece.IsEmpty())
						pieces.Insert(piece);

					piece = currChar;
				}

				prevCharType = 0;
			}
			else if (asciiValue >= MIN_UC && asciiValue <= MAX_UC)	// uppercase
			{
				bool isNextCharUppercase;
				if (i < len - 1)
				{
					int nextCharAsciiValue = valueName[i + 1].ToAscii();
					isNextCharUppercase = nextCharAsciiValue >= MIN_UC && nextCharAsciiValue <= MAX_UC;
				}
				else // final letter
				{
					isNextCharUppercase = true;
				}

				if (
					isNextCharUppercase &&
					prevCharType == 1 &&
					(asciiValue < MIN_LC || asciiValue > MAX_LC)) // curr char is NOT lowercase
				{
					piece += currChar;
				}
				else
				{
					if (!piece.IsEmpty())
						pieces.Insert(piece);

					piece = currChar;
				}

				prevCharType = 1;
			}
			else if (asciiValue >= MIN_DIGIT && asciiValue <= MAX_DIGIT)	// digits
			{
				if (prevCharType == 2)
				{
					piece += currChar;
				}
				else
				{
					if (!piece.IsEmpty())
						pieces.Insert(piece);

					piece = currChar;
				}

				prevCharType = 2;
			}
			else											// anything else
			{
				if (!piece.IsEmpty())
				{
					pieces.Insert(piece);
					piece = string.Empty;
				}

				prevCharType = 0;
			}
		}

		if (!piece.IsEmpty())
			pieces.Insert(piece);

		return Join(SPACE, pieces);
	}

	//------------------------------------------------------------------------------------------------
	//! Turns "{GUID012345678910}Prefabs/Characters/Factions/Faction/Character_FactionName_NLAW.et" into "Character FactionName NLAW"
	//! \param[in] resourceName
	//! \return space-separated words without resourceName extension
	static string FormatResourceNameToUserFriendly(ResourceName resourceName)
	{
		array<string> pieces = {};
		FilePath.StripExtension(FilePath.StripPath(resourceName)).Split(UNDERSCORE, pieces, true);
		return Join(SPACE, pieces);
	}

	//------------------------------------------------------------------------------------------------
	//! Turns "THIS_IS_AN_ENUM" or "tHIs_is_aN_enUM" into "This Is An Enum"
	//! \param[in] snakeCase any string with words separated by underscores
	//! \return firstchar-uppercased space-separated words
	static string FormatSnakeCaseToUserFriendly(string snakeCase)
	{
		array<string> pieces = {};
		snakeCase.Split(UNDERSCORE, pieces, true);
		for (int i, count = pieces.Count(); i < count; i++)
		{
			string piece = pieces[i];
			string firstChar = piece[0];
			firstChar.ToUpper();
			piece.ToLower();
			pieces[i] = firstChar + piece.Substring(1, piece.Length() - 1);
		}

		return Join(SPACE, pieces);
	}

	//------------------------------------------------------------------------------------------------
	//! Gets float values array from a string (e.g { 0.3, 5.0, 7.9 } from "0.3 5.0 abc 7.9")
	//! \param[in] input
	//! \param[in] splitter space by default, can be a comma from e.g IEntitySource values
	//! \return
	static array<float> GetFloatsFromString(string input, string splitter = SPACE)
	{
		array<float> result = {};
		array<string> splits = {};
		input.Split(splitter, splits, true);

		foreach (string split : splits)
		{
			float value = split.ToFloat();
			if (value != 0 || split.StartsWith("0"))
				result.Insert(value);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets int values array from a string (e.g { 3, 5, 7 } from "3 5 abc 7")
	//! \param[in] input
	//! \param[in] splitter space by default, can be a comma from e.g IEntitySource values
	//! \return
	static array<int> GetIntsFromString(string input, string splitter = SPACE)
	{
		array<int> result = {};
		array<string> splits = {};
		input.Split(splitter, splits, true);

		foreach (string split : splits)
		{
			int value = split.ToInt();
			if (value != 0 || split.StartsWith("0"))
				result.Insert(value);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Obtain an array of lines from a multiline string - split is done on the \\n character
	//! \param[in] input
	//! \param[in] removeEmptyLines if true, remove empty lines (including trimmed ones)
	//! \param[in] trimLines if true, trim lines
	//! \return array of lines, with or without empty/whitespace lines - can remove an empty array if removeEmptyLines is enabled
	static array<string> GetLines(string input, bool removeEmptyLines = false, bool trimLines = false)
	{
		if (!input)
		{
			if (removeEmptyLines)
				return {};
			else
				return { string.Empty };
		}

		array<string> result = {};
		input.Split(LINE_RETURN, result, removeEmptyLines);

		if (trimLines)
		{
			for (int i = result.Count() - 1; i >= 0; --i)
			{
				string line = result[i];
				line.TrimInPlace();
				if (removeEmptyLines && !line)
					result.RemoveOrdered(i);
				else
					result[i] = line;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the first occurrence of the provided samples
	//! \see string.IndexOf
	//! \param[in] input
	//! \param[in] samples
	//! \return
	static int IndexOf(string input, notnull array<string> samples)
	{
		if (input.IsEmpty() || samples.IsEmpty())
			return -1;

		int result = int.MAX;
		foreach (string sample : samples)
		{
			int index = input.IndexOf(sample);
			if (index != -1 && index < result)
				result = index;
		}

		if (result == int.MAX)
			return -1;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds the first occurrence of the provided samples from a position
	//! \see string.IndexOfFrom
	//! \param[in] input
	//! \param[in] start
	//! \param[in] samples
	//! \return
	static int IndexOfFrom(string input, int start, notnull array<string> samples)
	{
		if (start < 0 || start > input.Length() || input.IsEmpty() || samples.IsEmpty())
			return -1;

		int result = int.MAX;
		foreach (string sample : samples)
		{
			int index = input.IndexOfFrom(start, sample);
			if (index != -1 && index < result)
				result = index;
		}

		if (result == int.MAX)
			return -1;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] input
	//! \param[in] search
	//! \return an array of indices - never returns null
	array<int> IndicesOf(string input, string search)
	{
		if (!input || !search)
			return {};

		int inputLength = input.Length();
		int searchLength = search.Length();

		array<int> result = {};
		for (int i; i < inputLength; ++i)
		{
			int index = input.IndexOfFrom(i, search);
			if (index < 0)
				break;

			result.Insert(index);
			if (searchLength > 1)
				i += searchLength - 1;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Insert a string into another string
	//! \param[in] input the text in which to insert
	//! \param[in] insertion the text to insert
	//! \param[in] insertionIndex default 0
	//! \return input with insertion inserted at insertionIndex
	static string InsertAt(string input, string insertion, int insertionIndex = 0)
	{
		if (input.IsEmpty() || insertion.IsEmpty() || insertionIndex < 0 || insertionIndex > input.Length())
			return input;

		if (insertionIndex == 0)
			return insertion + input;

		return input.Substring(0, insertionIndex) + insertion + input.Substring(insertionIndex, input.Length() - insertionIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return true if input is empty or only made of spaces or tabs
	static bool IsEmptyOrWhiteSpace(string input)
	{
		return input.Trim().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns if the provided string is in the translation key format (e.g #AR-Translation_Value_3) - the pound sign (#) must be present!
	//! \param[in] input
	//! \return true if input is in the translation key format
	static bool IsTranslationKey(string input)
	{
		if (IsEmptyOrWhiteSpace(input))
			return false;

		if (input != input.Trim())
			return false;

		if (input.Length() < 2 || !input.StartsWith("#"))
			return false;

		for (int i, count = TRANSLATION_KEY_CHARS.Length(); i < count; i++)
		{
			if (input.EndsWith(TRANSLATION_KEY_CHARS[i]))
				return false;
		}

		string filter = LETTERS;
		if (!filter.Contains(input[1])) // \#[a-zA-Z].*
			return false;

		filter += DIGITS + TRANSLATION_KEY_CHARS;
		for (int i, len = input.Length(); i < len; i++)
		{
			if (!filter.Contains(input[i]))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Joins strings together (reverse operation of string.Split)
	//! \param[in] separator usually ", " for a string array
	//! \param[in] pieces the pieces to be joined
	//! \param[in] joinEmptyEntries if set to false, will ignore empty pieces (to e.g avoid ", , " occurrences)
	//! \return the string pieces joined with separator, or empty string if pieces is empty
	// moved above other methods for autocompletion's sake
	static string Join(string separator, notnull array<string> pieces, bool joinEmptyEntries = true)
	{
		if (pieces.IsEmpty())
			return string.Empty;

		string result;
		foreach (int i, string piece : pieces)
		{
			if (i == 0)
				result = piece;
			else
			if (joinEmptyEntries || piece) // !piece.IsEmpty()'s fast version
				result += separator + piece;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Joins bools together as string
	//! \param[in] separator usually "," for an int array
	//! \param[in] pieces the pieces to be joined
	//! \param[in] numerical whether or not true is stringified as 1 and false as 0, or written full text ("true" and "false")
	//! \return the pieces joined with separator, or empty string if pieces is empty
	static string Join(string separator, notnull array<bool> pieces, bool numerical = false)
	{
		if (pieces.IsEmpty())
			return string.Empty;

		string result;
		foreach (int i, bool piece : pieces)
		{
			if (i == 0)
				result = piece.ToString(numerical);
			else
				result += separator + piece.ToString(numerical);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Joins ints together as string
	//! \param[in] separator usually "," for an int array
	//! \param[in] pieces the pieces to be joined
	//! \return the pieces joined with separator, or empty string if pieces is empty
	static string Join(string separator, notnull array<int> pieces)
	{
		if (pieces.IsEmpty())
			return string.Empty;

		string result;
		foreach (int i, int piece : pieces)
		{
			if (i == 0)
				result = piece.ToString();
			else
				result += separator + piece;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Joins floats together as string
	//! \param[in] separator usually "," for a float array
	//! \param[in] pieces the pieces to be joined
	//! \return the pieces joined with separator, or empty string if pieces is empty
	static string Join(string separator, notnull array<float> pieces)
	{
		if (pieces.IsEmpty())
			return string.Empty;

		string result;
		foreach (int i, float piece : pieces)
		{
			if (i == 0)
				result = piece.ToString();
			else
				result += separator + piece;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate the Levenshtein distance between two words (https://en.wikipedia.org/wiki/Levenshtein_distance)
	//! \note not to be confused with Damerau-Levenshtein/OSA distance (see DamerauLevenshteinDistance)
	//! \param[in] word1
	//! \param[in] word2
	//! \param[in] caseSensitive
	//! \return number of operations to go from one word to the other (always in 0..inf range)
	static int GetLevenshteinDistance(string word1, string word2, bool caseSensitive = true)
	{
		if (!word1) // .IsEmpty()
			return word2.Length();

		if (!word2) // .IsEmpty()
			return word1.Length();

		if (!caseSensitive)
		{
			word1.ToLower();
			word2.ToLower();
		}

		if (word1 == word2)
			return 0;

		int word1Length = word1.Length();
		int word2Length = word2.Length();

		array<int> v0 = {};
		array<int> v1 = {};
		v0.Resize(word1Length + 1);
		v1.Resize(word1Length + 1);

		for (int i = 1; i <= word1Length; ++i)
		{
			v0[i] = i;
		}

		bool swapped;
		for (int j = 1; j <= word2Length; ++j)
		{
			if (swapped)
				v0[0] = j;
			else
				v1[0] = j;

			for (int i = 1; i <= word1Length; ++i)
			{
				int cost = word1[i - 1] != word2[j - 1];
				if (swapped)	// v0[i] = Math.Min(v1[i] + 1, Math.Min(v0[i - 1] + 1, v1[i - 1] + cost));
				{
					int min = v1[i] + 1;
					int tmp = v0[i - 1] + 1;
					if (tmp < min)
						min = tmp;

					tmp = v1[i - 1] + cost;
					if (tmp < min)
						min = tmp;

					v0[i] = min;
				}
				else			// v1[i] = Math.Min(v0[i] + 1, Math.Min(v1[i - 1] + 1, v0[i - 1] + cost));
				{
					int min = v0[i] + 1;
					int tmp = v1[i - 1] + 1;
					if (tmp < min)
						min = tmp;

					tmp = v0[i - 1] + cost;
					if (tmp < min)
						min = tmp;

					v1[i] = min;
				}
			}

			swapped = !swapped; // simili vector swap
		}

		if (swapped)
			return v1[word1Length];
		else
			return v0[word1Length];
	}

	//------------------------------------------------------------------------------------------------
	//! \see LevenshteinDistance
	//! \return a 0..1 matching score - 0 being the farthest, 1 being the identical
	static float GetLevenshteinDistanceScore(string word1, string word2, bool caseSensitive = true)
	{
		int distance = GetLevenshteinDistance(word1, word2, caseSensitive);
		if (distance == 0)
			return 1.0;

		int word1Length = word1.Length();
		int word2Length = word2.Length();

		if (word1Length > word2Length)
			return 1 - distance / word1Length;
		else
			return 1 - distance / word2Length;
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate the Damerau-Levenshtein Optimal string Alignment (OSA) distance between two words (https://en.wikipedia.org/wiki/Damerau%E2%80%93Levenshtein_distance)\n
	//! \note not to be confused with Levenshtein distance (see LevenshteinDistance)
	//! \param[in] word1
	//! \param[in] word2
	//! \param[in] caseSensitive
	//! \return
	static int GetDamerauLevenshteinDistance(string word1, string word2, bool caseSensitive = true)
	{
		if (!word1) // .IsEmpty()
			return word2.Length();

		if (!word2) // .IsEmpty()
			return word1.Length();

		if (!caseSensitive)
		{
			word1.ToLower();
			word2.ToLower();
		}

		if (word1 == word2)
			return 0;

		int word1Length = word1.Length();
		int word2Length = word2.Length();

		array<int> line;
		array<ref array<int>> matrix = {};
		matrix.Reserve(word1Length + 1);
		for (int i; i <= word1Length; ++i)
		{
			line = { i };
			line.Resize(word2Length + 1);
			matrix.Insert(line);
		}

		for (int i = 1; i <= word1Length; ++i)
		{
			for (int j = 1; j <= word2Length; ++j)
			{
				string word1iMinus1 = word1[i - 1];
				string word2jMinus1 = word2[j - 1];
				int cost = word1iMinus1 != word2jMinus1;
				if (i == 1)
					matrix[0][j] = j;

				int min = matrix[i - 1][j] + 1;
				int tmp = matrix[i][j - 1] + 1;
				if (tmp < min)
					min = tmp;

				tmp = matrix[i - 1][j - 1] + cost;
				if (tmp < min)
					min = tmp;

				if (i > 1 && j > 1)
				{
					if (word1iMinus1 == word2[j - 2] && word1[i - 2] == word2jMinus1)
					{
						tmp = matrix[i - 2][j - 2] + cost;
						if (tmp < min)
							min = tmp;
					}
				}

				matrix[i][j] = min;
			}
		}

		return matrix[word1Length][word2Length];
	}

	//------------------------------------------------------------------------------------------------
	//! \see DamerauLevenshteinDistance
	//! \return a 0..1 matching score - 0 being the farthest, 1 being identical
	static float GetDamerauLevenshteinDistanceScore(string word1, string word2, bool caseSensitive = true)
	{
		int distance = GetDamerauLevenshteinDistance(word1, word2, caseSensitive);
		if (distance == 0)
			return 1.0;

		int word1Length = word1.Length();
		int word2Length = word2.Length();

		if (word1Length > word2Length)
			return 1 - distance / word1Length;
		else
			return 1 - distance / word2Length;
	}

	//------------------------------------------------------------------------------------------------
	//! Pads the provided string's left end (start) with the provided padding (cut to the exact length)
	//! Will not do anything if input is already longer than length or if padding is empty
	//! If input is longer, it will not be cut
	//! \param[in] input
	//! \param[in] length
	//! \param[in] padding
	//! \result the padded string
	static string PadLeft(string input, int length, string padding = SPACE)
	{
		if (!padding)
			return input;

		if (input.Length() >= length)
			return input;

		int padW = padding.Length();
		for (int i = length - input.Length() - 1; i >= 0; i -= padW)
		{
			input = padding + input;
		}

		if (input.Length() > length)
			input = input.Substring(input.Length() - length, length);

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Pads the provided string's right end (end) with the provided padding (cut to the exact length)
	//! Will not do anything if input is already longer than length or if padding is empty
	//! If input is longer, it will not be cut
	//! \param[in] input
	//! \param[in] length
	//! \param[in] padding
	//! \result the padded string
	static string PadRight(string input, int length, string padding = SPACE)
	{
		if (!padding)
			return input;

		if (input.Length() >= length)
			return input;

		int padW = padding.Length();
		for (int i = length - input.Length() - 1; i >= 0; i -= padW)
		{
			input += padding;
		}

		if (input.Length() > length)
			input = input.Substring(0, length);

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Replace until there is no trace of search
	//! \code
	//! string input = "AAAAAABAAA";
	//! input.Replace("AA", "A");										// input is now "AAABAA"
	//! SCR_StringHelper.ReplaceRecursive("AAAAAABAAA", "AA", "A");		// returns "ABA"
	//! SCR_StringHelper.ReplaceRecursive("AAAAAABAAA", "AA", "AAX");	// returns "AAAAAABAAA"
	//! \endcode
	//! \param[in] input the input in which to search and replace
	//! \param[in] what to replace - an empty sample will do nothing
	//! \param[in] replacement CANNOT contain sample for an obvious reason
	//! \return the modified input, or the original one on wrong arguments
	static string ReplaceRecursive(string input, string sample, string replacement)
	{
		if (!input || !sample || sample == replacement || replacement.Contains(sample)) // .IsEmpty() x2
			return input;

		while (input.IndexOf(sample) > -1)
		{
			input.Replace(sample, replacement);
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Replaces multiple entries by the same replacement
	//! \param[in] input
	//! \param[in] samples
	//! \param[in] replacement
	//! \return
	static string ReplaceMultiple(string input, notnull array<string> samples, string replacement)
	{
		for (int i = samples.Count() - 1; i >= 0; --i)
		{
			if (replacement.Contains(samples[i]))
				samples.Remove(i);
		}

		if (samples.IsEmpty())
			return input;

		foreach (string sample : samples)
		{
			input.Replace(sample, replacement);
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Replace X times a string from within a string from left to right
	//! the next occurrence is searched after the previous replacement, there is no overlap
	//! \code
	//! SCR_StringHelper.ReplaceTimes("Hello Hallo Yellow", "llo", "ya");		// returns "Heya Hallo Yellow"
	//! SCR_StringHelper.ReplaceTimes("Hello Hallo Yellow", "llo", "ya", 2);	// returns "Heya Haya Yellow"
	//! SCR_StringHelper.ReplaceTimes("Hello Hallo Yellow", "llo", "ya", 4);	// returns "Heya Haya Yeyaw"
	//! SCR_StringHelper.ReplaceTimes("A A A", "A", "BA", 2);		// returns "BA BA A"
	//! SCR_StringHelper.ReplaceTimes("A A A", "A", "B", 1, 1);	// returns "A B A"
	//! \endcode
	//! \param[in] input the input in which to search and replace
	//! \param[in] sample what to replace - an empty sample will do nothing
	//! \param[in] replacement the replacement string
	//! \param[in] howMany times the string must be replaced
	//! \param[in] skip first occurrences -not- to be replaced
	//! \return input with 'sample' replaced by 'replacement' 'howMany' times after skipping 'skip' occurrences
	static string ReplaceTimes(string input, string sample, string replacement, int howMany = 1, int skip = 0)
	{
		if (howMany < 1 || input.IsEmpty() || sample.IsEmpty() || sample == replacement)
			return input;

		int sampleLength = sample.Length();
		int replaceLength = replacement.Length();

		int index;
		while (howMany > 0)
		{
			index = input.IndexOfFrom(index, sample);
			if (index < 0)
				break;

			if (skip > 0)
			{
				skip--;
				index += sampleLength;
				continue;
			}

			if (index == 0)
				input = replacement + input.Substring(sampleLength, input.Length() - sampleLength);
			else
				input = input.Substring(0, index) + replacement + input.Substring(index + sampleLength, input.Length() - (index + sampleLength));

			// no overlap
			index += replaceLength;

			howMany--;
		}

		return input;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the provided input string reversed
	//! \code
	//! SCR_StringHelper.Reverse("ABC123"); // returns "321CBA"
	//! \endcode
	//! \param[in] input
	//! \return reversed input
	static string Reverse(string input)
	{
		string result;
		for (int i = input.Length() - 1; i >= 0; i--)
		{
			result += input[i];
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Search a string with star-keywords - only supports begin* / *end / *contains* search format
	//! \param[in] haystack
	//! \param[in] needle the star-based search:
	//! - word		= exact search
	//! - word*		= starting with
	//! - *word		= starting with
	//! - *word*	= containing
	//! \param[in] caseSensitive whether or not the search is case-sensitive
	//! \param[in] strictMatch star is a REQUIREMENT and not a POSSIBILITY:
	//! - true:		*word*	= NEEDS a before/after (e.g matches with XwordX, does not match with Xword, wordX and word)
	//! - false:	*word*	= CAN have a before/after (e.g matches with XwordX, Xword, wordX and word)
	//! \return true if matches
	static bool SimpleStarSearchMatches(string haystack, string needle, bool caseSensitive, bool strictMatch)
	{
		if (!haystack || !needle) // .IsEmpty()
			return false;

		if (!caseSensitive)
		{
			haystack.ToLower();
			needle.ToLower();
		}

		if (!needle.Contains(STAR))		// exact match, boom
			return needle == haystack;

		bool startsWithStar = needle.StartsWith(STAR);
		bool endsWithStar = needle.EndsWith(STAR);

		if (startsWithStar)
		{
			int length = needle.Length();
			if (length == 1)
				return false;

			needle = needle.Substring(1, length - 1);
		}

		if (endsWithStar)
		{
			int length = needle.Length();
			if (length == 1)
				return false;

			needle = needle.Substring(0, length - 1);
		}

		bool isMatch = !strictMatch || needle != haystack;
		if (!isMatch)
			return false;

		if (!startsWithStar && endsWithStar)	// starts with needle
			return haystack.StartsWith(needle);

		if (startsWithStar && endsWithStar)		// contains needle
			return (!strictMatch || (needle != haystack && !haystack.StartsWith(needle) && !haystack.EndsWith(needle))) && haystack.Contains(needle);

		if (startsWithStar && !endsWithStar)	// ends with needle
			return haystack.EndsWith(needle);

		// star in the middle of the word or something
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if input contains any needles
	//! \param[in] input
	//! \param[in] needles
	//! \return
	static bool ContainsAny(string input, notnull array<string> needles)
	{
		if (input.IsEmpty())
			return false;

		foreach (string needle : needles)
		{
			if (input.Contains(needle))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if input contains every needles
	//! \param[in] input
	//! \param[in] needles
	//! \return
	static bool ContainsEvery(string input, notnull array<string> needles)
	{
		if (input.IsEmpty())
			return false;

		foreach (string needle : needles)
		{
			if (!input.Contains(needle))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input the input string
	//! \param[in] characters the characters that must compose input or that must not be present depending on useCharactersAsBlacklist
	//! \param[in] useCharactersAsBlacklist false for characters to be a whitelist, true for a blacklist
	//! \return true if no forbidden characters were found or if characters is empty, false otherwise or if input is empty
	static bool ContainsOnly(string input, string characters, bool useCharactersAsBlacklist = false)
	{
		int inputLength = input.Length();
		if (inputLength < 1)
			return false;

		int charsLength = characters.Length();
		if (charsLength < 1)
			return true;

		if (charsLength == 1)	// go the easy way
		{
			for (int i; i < inputLength; ++i)
			{
				if ((input[i] == characters) == useCharactersAsBlacklist) // if it contains but shouldn't, or vice-versa
					return false;
			}

			return true;
		}

		// else					// go the hard way

		array<int> asciiValues = {};
		for (int i; i < charsLength; ++i)
		{
			asciiValues.Insert(characters[i].ToAscii());
		}

		for (int i; i < inputLength; i++)
		{
			if (asciiValues.Contains(input[i].ToAscii()) == useCharactersAsBlacklist)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Find out if a string's beginning matches one of the provided beginnings
	//! \param[in] input the string to check
	//! \param[in] lineStarts the beginnings to parse
	//! \return whether or not the input begins with one of the provided line beginnings
	static bool StartsWithAny(string input, notnull array<string> lineStarts)
	{
		if (input.IsEmpty())
			return false;

		foreach (string lineStart : lineStarts)
		{
			if (input.StartsWith(lineStart))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Find out if a string's ending matches one of the provided endings
	//! \param[in] input the string to check
	//! \param[in] lineEnds the endings to parse
	//! \return whether or not the input ends with one of the provided line endings
	static bool EndsWithAny(string input, notnull array<string> lineEnds)
	{
		if (input.IsEmpty())
			return false;

		foreach (string lineEnd : lineEnds)
		{
			if (input.EndsWith(lineEnd))
				return true;
		}

		return false;
	}


	//------------------------------------------------------------------------------------------------
	//! Remove spaces, tabs and line returns on the left end of the provided string
	//! Vertical tabs and other characters are ignored and considered as normal characters (for now?)
	//! \param[in] input
	//! \return the trimmed input
	static string TrimLeft(string input)
	{
		if (input.IsEmpty())
			return string.Empty;

		for (int i, count = input.Length(); i < count; i++)
		{
			string character = input[i];
			if (character == SPACE || character == TAB || character == LINE_RETURN)
				continue;

			return input.Substring(i, count - i);
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove spaces, tabs and line returns on the right end of the provided string
	//! Vertical tabs and other characters are ignored and considered as normal characters (for now?)
	//! \param[in] input
	//! \return the trimmed input
	static string TrimRight(string input)
	{
		if (input.IsEmpty())
			return string.Empty;

		for (int i = input.Length() - 1; i >= 0; i--)
		{
			string character = input[i];
			if (character == SPACE || character == TAB || character == LINE_RETURN)
				continue;

			return input.Substring(0, i + 1);
		}

		return string.Empty;
	}
}

enum SCR_EStringFormat
{
	ALPHABETICAL_UC,	//!< [A-Z]+			(UC = UpperCase)
	ALPHABETICAL_LC,	//!< [a-z]+			(LC = LowerCase)
	ALPHABETICAL_I,		//!< [a-zA-Z]+		(I = Insensitive)
	ALPHANUMERICAL_UC,	//!< [A-Z0-9]+		(UC = UpperCase)
	ALPHANUMERICAL_LC,	//!< [a-z0-9]+		(LC = LowerCase)
	ALPHANUMERICAL_I,	//!< [a-zA-Z0-9]+	(I = Insensitive)
	DIGITS_ONLY,		//!< [0-9]+
	// FLOATING_POINT,		//!< [0-9][0-9\.]+[0-9]
}
