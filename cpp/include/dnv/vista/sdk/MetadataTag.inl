/**
 * @file MetadataTag.inl
 * @brief Inline implementations for performance-critical MetadataTag operations
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// MetadataTag class
	//=====================================================================

	//-------------------------------------------------------------------------
	// Operators
	//-------------------------------------------------------------------------

	inline bool MetadataTag::operator==( const MetadataTag& other ) const
	{
		if ( m_value != other.m_value )
		{
			return false;
		}

		if ( m_name != other.m_name )
		{
			static constexpr std::string_view errorPrefix = "Can't compare tags with different names";
			throw std::invalid_argument( std::string( errorPrefix ) );
		}

		return true;
	}

	inline bool MetadataTag::operator!=( const MetadataTag& other ) const
	{
		return !( *this == other );
	}

	inline MetadataTag::operator std::string() const
	{
		return m_value;
	}

	//-------------------------------------------------------------------------
	// Accessors
	//-------------------------------------------------------------------------

	inline CodebookName MetadataTag::name() const noexcept
	{
		return m_name;
	}

	inline std::string_view MetadataTag::value() const noexcept
	{
		return m_value;
	}

	inline char MetadataTag::prefix() const noexcept
	{
		return m_custom ? '~' : '-';
	}

	inline size_t MetadataTag::hashCode() const noexcept
	{
		return std::hash<std::string_view>{}( m_value );
	}

	//-------------------------------------------------------------------------
	// State inspection methods
	//-------------------------------------------------------------------------

	bool MetadataTag::isCustom() const noexcept
	{
		return m_custom;
	}

}
