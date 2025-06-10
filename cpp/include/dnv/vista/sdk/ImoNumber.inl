/**
 * @file ImoNumber.inl
 * @brief Inline implementations for performance-critical ImoNumber operations
 */

namespace dnv::vista::sdk
{

	//=====================================================================
	// ImoNumber class
	//=====================================================================

	//----------------------------------------------
	// Operators
	//----------------------------------------------

	inline bool ImoNumber::operator==( const ImoNumber& other ) const noexcept
	{
		return m_value == other.m_value;
	}

	inline bool ImoNumber::operator!=( const ImoNumber& other ) const noexcept
	{
		return !( *this == other );
	}

	inline ImoNumber::operator int() const noexcept
	{
		return m_value;
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	size_t ImoNumber::hashCode() const noexcept
	{
		return static_cast<size_t>( static_cast<uint32_t>( m_value ) );
	}
}
