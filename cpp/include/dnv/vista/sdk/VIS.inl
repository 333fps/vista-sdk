/**
 * @file VIS.inl
 * @brief Inline implementations for performance-critical VIS operations
 */

namespace dnv::vista::sdk
{
	//----------------------------------------------
	// ISO string validation methods
	//----------------------------------------------

	inline bool VIS::matchISOLocalIdString( const std::stringstream& builder ) noexcept
	{
		return matchISOLocalIdString( std::string_view( builder.str() ) );
	}

	inline bool VIS::matchISOLocalIdString( std::string_view value ) noexcept
	{
		const char* data = value.data();
		const char* end = data + value.size();

		for ( const char* charPtr = data; charPtr != end; ++charPtr )
		{
			if ( *charPtr == '/' )
			{
				continue;
			}

			if ( !matchAsciiDecimal( static_cast<int>( *charPtr ) ) )
			{

				return false;
			}
		}

		return true;
	}

	inline bool VIS::isISOString( std::string_view value ) noexcept
	{
		if ( value.empty() )
		{
			return true;
		}

		const char* data = value.data();
		const char* end = data + value.size();

		for ( const char* charPtr = data; charPtr != end; ++charPtr )
		{
			if ( !matchAsciiDecimal( static_cast<int>( *charPtr ) ) )
			{

				return false;
			}
		}

		return true;
	}

	inline bool VIS::isISOString( const std::string& value ) noexcept
	{
		return isISOString( std::string_view( value ) );
	}

	inline bool VIS::isISOString( const std::stringstream& builder ) noexcept
	{
		std::string str = builder.str();

		return isISOString( std::string_view( str ) );
	}

	inline bool VIS::isISOLocalIdString( const std::string& value ) noexcept
	{
		return !value.empty() && matchISOLocalIdString( value );
	}

	inline bool VIS::isISOString( char c ) noexcept
	{
		return matchAsciiDecimal( static_cast<int>( c ) );
	}

	inline bool VIS::matchAsciiDecimal( int code ) noexcept
	{
		/* Numbers (48-57) */
		if ( code >= 48 && code <= 57 )
		{
			return true;
		}

		/* Uppercase A-Z (65-90) */
		if ( code >= 65 && code <= 90 )
		{
			return true;
		}

		/* Lowercase a-z (97-122) */
		if ( code >= 97 && code <= 122 )
		{
			return true;
		}

		/* Special chars: "-", ".", "_", "~" */
		return ( code == 45 || code == 46 || code == 95 || code == 126 );
	}

}
