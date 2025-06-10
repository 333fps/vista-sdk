/**
 * @file Codebooks.inl
 * @brief Inline implementation of Codebooks class methods
 * @details Contains performance-critical inline implementations for
 *          fast codebook access operations.
 */

namespace dnv::vista::sdk
{
	//=====================================================================
	// Codebooks class
	//=====================================================================

	//----------------------------------------------
	// Lookup operators
	//----------------------------------------------

	inline const Codebook& Codebooks::operator[]( CodebookName name ) const noexcept
	{
		return m_codebooks[static_cast<size_t>( name ) - 1];
	}

	//----------------------------------------------
	// Accessors
	//----------------------------------------------

	inline VisVersion Codebooks::visVersion() const noexcept
	{
		return m_visVersion;
	}

	inline const Codebook& Codebooks::codebook( CodebookName name ) const
	{
		const auto rawValue = static_cast<size_t>( name );
		if ( rawValue == 0 || rawValue > NUM_CODEBOOKS ) [[unlikely]]
		{
			throw std::invalid_argument( "Invalid codebook name: " + std::to_string( rawValue ) );
		}

		const auto index = rawValue - 1;

		return m_codebooks[index];
	}

	//----------------------------------------------
	// Range-based iterator
	//----------------------------------------------

	inline Codebooks::Iterator Codebooks::begin() const noexcept
	{
		return m_codebooks.data();
	}

	inline Codebooks::Iterator Codebooks::end() const noexcept
	{
		return m_codebooks.data() + NUM_CODEBOOKS;
	}

	//----------------------------------------------
	// Tag creation
	//----------------------------------------------

	inline std::optional<MetadataTag> Codebooks::tryCreateTag( CodebookName name, const std::string_view value ) const
	{
		return m_codebooks[static_cast<size_t>( name ) - 1].tryCreateTag( value );
	}

	inline MetadataTag Codebooks::createTag( CodebookName name, const std::string& value ) const
	{
		return m_codebooks[static_cast<size_t>( name ) - 1].createTag( value );
	}
}
