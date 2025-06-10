/**
 * @file UniversalIdBuilder.inl
 * @brief Inline implementations for performance-critical UniversalIdBuilder operations
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// UniversalIdBuilder class
	//=====================================================================

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	inline bool UniversalIdBuilder::operator==( const UniversalIdBuilder& other ) const
	{
		return equals( other );
	}

	inline bool UniversalIdBuilder::operator!=( const UniversalIdBuilder& other ) const
	{
		return !equals( other );
	}

	inline bool UniversalIdBuilder::equals( const UniversalIdBuilder& other ) const
	{
		return m_imoNumber == other.m_imoNumber && m_localIdBuilder == other.m_localIdBuilder;
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	inline const std::optional<ImoNumber>& UniversalIdBuilder::imoNumber() const noexcept
	{
		return m_imoNumber;
	}

	inline const std::optional<LocalIdBuilder>& UniversalIdBuilder::localId() const noexcept
	{
		return m_localIdBuilder;
	}

	inline size_t UniversalIdBuilder::hashCode() const noexcept
	{
		size_t hash = 0;

		if ( m_imoNumber.has_value() )
		{
			hash ^= m_imoNumber->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}

		if ( m_localIdBuilder.has_value() )
		{
			hash ^= m_localIdBuilder->hashCode() + 0x9e3779b9 + ( hash << 6 ) + ( hash >> 2 );
		}

		return hash;
	}

	//----------------------------------------------
	// State inspection methods
	//----------------------------------------------

	inline bool UniversalIdBuilder::isValid() const noexcept
	{
		return m_imoNumber.has_value() && m_localIdBuilder.has_value() && m_localIdBuilder->isValid();
	}

}
