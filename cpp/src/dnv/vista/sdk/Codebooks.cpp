/**
 * @file Codebooks.cpp
 * @brief Implementation of Vessel Information Structure (VIS) codebooks
 */

#include "pch.h"

#include "dnv/vista/sdk/Codebooks.h"

#include "dnv/vista/sdk/CodebookName.h"
#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/CodebooksDto.h"
#include "dnv/vista/sdk/MetadataTag.h"

namespace dnv::vista::sdk
{
	//-------------------------------------------------------------------
	// Construction / Destruction
	//-------------------------------------------------------------------

	Codebooks::Codebooks( VisVersion version, const CodebooksDto& dto )
		: m_visVersion( version ), m_codebooks{}
	{
		SPDLOG_DEBUG( "Initializing codebooks for VIS version: {}", VisVersionExtensions::toVersionString( version ) );

		for ( const auto& typeDto : dto.items() )
		{
			try
			{
				SPDLOG_DEBUG( "Processing codebook: {}", typeDto.name );
				Codebook codebook( typeDto );
				auto index = static_cast<size_t>( codebook.name() ) - 1;

				if ( index < NUM_CODEBOOKS )
				{
					m_codebooks[index] = codebook;
					SPDLOG_DEBUG( "Added codebook '{}' at index {}", typeDto.name, index );
				}
				else
				{
					SPDLOG_WARN( "Invalid codebook index: {} for name: {}", index, typeDto.name() );
				}
			}
			catch ( const std::exception& ex )
			{
				SPDLOG_ERROR( "Error processing codebook '{}': {}", typeDto.name(), ex.what() );
			}
		}

		std::unordered_map<std::string, std::vector<std::string>> emptyValues;
		CodebookDto detailDto( "detail", emptyValues );

		Codebook detailCodebook( detailDto );

		auto detailIndex = static_cast<size_t>( CodebookName::Detail ) - 1;
		if ( detailIndex < NUM_CODEBOOKS )
		{
			m_codebooks[detailIndex] = detailCodebook;
			SPDLOG_DEBUG( "Added empty Detail codebook at index {}", detailIndex );
		}
	}

	//-------------------------------------------------------------------
	// Codebook access methods
	//-------------------------------------------------------------------

	VisVersion Codebooks::visVersion() const
	{
		return m_visVersion;
	}

	const Codebook& Codebooks::operator[]( CodebookName name ) const
	{
		if ( m_codebooks.empty() )
		{
			SPDLOG_ERROR( "Codebooks array is empty or uninitialized" );
		}

		auto index = static_cast<size_t>( name ) - 1;
		SPDLOG_DEBUG( "Accessing codebook[{}], vector size: {}", index, m_codebooks.size() );

		if ( index >= m_codebooks.size() )
		{
			SPDLOG_ERROR( "Invalid codebook name: {}", static_cast<int>( name ) );
			throw std::invalid_argument( "Invalid codebook name: " + std::to_string( static_cast<int>( name ) ) );
		}
		return m_codebooks[index];
	}

	const Codebook& Codebooks::codebook( CodebookName name ) const
	{
		return ( *this )[name];
	}

	//-------------------------------------------------------------------
	// Tag creation methods
	//-------------------------------------------------------------------

	std::optional<MetadataTag> Codebooks::tryCreateTag( CodebookName name, const std::string_view value ) const
	{
		SPDLOG_DEBUG( "Attempting to create tag '{}' for codebook: {} (enum value: {})",
			value, CodebookNames::toPrefix( name ), static_cast<int>( name ) );

		if ( m_codebooks.empty() )
		{
			SPDLOG_ERROR( "Codebooks array is empty or uninitialized" );
			return std::nullopt;
		}

		try
		{
			auto result = ( *this )[name].tryCreateTag( value );

			if ( result.has_value() )
			{
				SPDLOG_DEBUG( "Successfully created tag for '{}' in codebook '{}'", value, CodebookNames::toPrefix( name ) );
			}
			else
			{
				SPDLOG_DEBUG( "Failed to create tag for '{}' in codebook '{}' - invalid value", value, CodebookNames::toPrefix( name ) );
			}

			return result;
		}
		catch ( const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception in tryCreateTag for '{}' in codebook '{}': {}", value, CodebookNames::toPrefix( name ), ex.what() );
			throw;
		}
	}

	MetadataTag Codebooks::createTag( CodebookName name, const std::string& value ) const
	{
		return ( *this )[name].createTag( value );
	}

	//-------------------------------------------------------------------
	// Iterator implementation
	//-------------------------------------------------------------------

	Codebooks::Iterator::Iterator( const std::array<Codebook, NUM_CODEBOOKS>* codebooks, size_t index )
		: m_codebooks( codebooks ), m_index( index )
	{
	}

	Codebooks::Iterator::reference Codebooks::Iterator::operator*() const
	{
		if ( m_index >= m_codebooks->size() )
		{
			SPDLOG_ERROR( "Iterator out of range: index={}", m_index );
			throw std::invalid_argument( "Iterator out of range" );
		}

		const Codebook& codebook = ( *m_codebooks )[m_index];

		m_current = std::make_tuple( codebook.name(), std::cref( codebook ) );
		return *m_current;
	}

	Codebooks::Iterator::pointer Codebooks::Iterator::operator->() const
	{
		if ( m_index >= m_codebooks->size() )
		{
			SPDLOG_ERROR( "Iterator out of range: index={}", m_index );
			throw std::invalid_argument( "Iterator out of range" );
		}

		const Codebook& codebook = ( *m_codebooks )[m_index];
		if ( !m_current.has_value() )
		{
			m_current = std::make_tuple( codebook.name(), std::cref( codebook ) );
		}

		return &( *m_current );
	}

	Codebooks::Iterator& Codebooks::Iterator::operator++()
	{
		++m_index;
		return *this;
	}

	Codebooks::Iterator Codebooks::Iterator::operator++( int )
	{
		Iterator tmp = *this;
		++( *this );
		return tmp;
	}

	bool Codebooks::Iterator::operator==( const Iterator& other ) const
	{
		return m_codebooks == other.m_codebooks && m_index == other.m_index;
	}

	bool Codebooks::Iterator::operator!=( const Iterator& other ) const
	{
		return !( *this == other );
	}

	bool Codebooks::Iterator::next()
	{
		++m_index;
		return m_index < m_codebooks->size();
	}

	bool Codebooks::Iterator::current()
	{
		if ( m_index < m_codebooks->size() )
		{
			const Codebook& codebook = ( *m_codebooks )[m_index];
			m_current = std::make_tuple( codebook.name(), std::cref( codebook ) );
			return true;
		}
		return false;
	}

	void Codebooks::Iterator::reset()
	{
		m_index = std::numeric_limits<size_t>::max();
	}

	//-------------------------------------------------------------------
	// Iteration methods
	//-------------------------------------------------------------------

	Codebooks::Iterator Codebooks::begin() const
	{
		return Iterator( &m_codebooks, 0 );
	}

	Codebooks::Iterator Codebooks::end() const
	{
		return Iterator( &m_codebooks, m_codebooks.size() );
	}

	Codebooks::Enumerator Codebooks::enumerator() const
	{
		return Iterator( &m_codebooks, std::numeric_limits<size_t>::max() );
	}
}
