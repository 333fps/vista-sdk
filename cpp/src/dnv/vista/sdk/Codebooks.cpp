/**
 * @file Codebooks.cpp
 * @brief Implementation of Vessel Information Structure (VIS) codebooks
 */

#include "pch.h"

#include "dnv/vista/sdk/Codebooks.h"

#include "dnv/vista/sdk/VisVersion.h"
#include "dnv/vista/sdk/CodebooksDto.h"
#include "dnv/vista/sdk/MetadataTag.h"

namespace dnv::vista::sdk
{
	//=====================================================================
	// Iterator
	//=====================================================================

	Codebooks::Iterator::Iterator( const std::array<Codebook, NUM_CODEBOOKS>* codebooks, size_t index )
		: m_codebooks{ codebooks },
		  m_index{ index }
	{
	}

	Codebooks::Iterator::reference Codebooks::Iterator::operator*() const
	{
		if ( m_index >= NUM_CODEBOOKS )
		{
			SPDLOG_ERROR( "Iterator dereference out of range (index: {}, size: {})", m_index, NUM_CODEBOOKS );
			throw std::out_of_range( "Iterator out of range" );
		}

		CodebookName name = static_cast<CodebookName>( m_index + 1 );
		return std::make_tuple( name, std::cref( ( *m_codebooks )[m_index] ) );
	}

	Codebooks::Iterator::value_type Codebooks::Iterator::operator->() const
	{
		return operator*();
	}

	Codebooks::Iterator& Codebooks::Iterator::operator++()
	{
		if ( m_index < NUM_CODEBOOKS )
		{
			++m_index;
		}

		return *this;
	}

	Codebooks::Iterator Codebooks::Iterator::operator++( int )
	{
		Iterator temp = *this;
		++( *this );

		return temp;
	}

	bool Codebooks::Iterator::operator==( const Iterator& other ) const
	{
		return m_codebooks == other.m_codebooks && m_index == other.m_index;
	}

	bool Codebooks::Iterator::operator!=( const Iterator& other ) const
	{
		return !( *this == other );
	}

	//=====================================================================
	// Construction / Destruction
	//=====================================================================

	Codebooks::Codebooks( VisVersion version, const CodebooksDto& dto )
		: m_visVersion{ version },
		  m_codebooks{}
	{
		SPDLOG_DEBUG( "Initializing codebooks for VIS version: {}", VisVersionExtensions::toVersionString( version ) );

		for ( const auto& typeDto : dto.items() )
		{
			try
			{
				SPDLOG_DEBUG( "Processing codebook DTO: {}", typeDto.name() );
				Codebook codebook( typeDto );
				auto index = static_cast<size_t>( codebook.name() ) - 1;

				if ( index < NUM_CODEBOOKS )
				{
					m_codebooks[index] = std::move( codebook );
					SPDLOG_DEBUG( "Added codebook '{}' at index {}", typeDto.name(), index );
				}
				else
				{
					SPDLOG_WARN( "Invalid codebook index calculated: {} for name: {}", index, typeDto.name() );
				}
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Error processing codebook DTO '{}': {}", typeDto.name(), ex.what() );
			}
		}

		std::unordered_map<std::string, std::vector<std::string>> emptyValues;
		CodebookDto detailDto( "detail", emptyValues );
		Codebook detailCodebook( detailDto );

		auto detailIndex = static_cast<size_t>( CodebookName::Detail ) - 1;
		if ( detailIndex < NUM_CODEBOOKS )
		{
			m_codebooks[detailIndex] = std::move( detailCodebook );

			SPDLOG_DEBUG( "Ensured empty Detail codebook exists at index {}", detailIndex );
		}
		else
		{
			SPDLOG_ERROR( "Calculated invalid index {} for Detail codebook", detailIndex );
		}
	}

	//=====================================================================
	// Operators
	//=====================================================================

	const Codebook& Codebooks::operator[]( CodebookName name ) const
	{
		auto index = static_cast<size_t>( name ) - 1;
		SPDLOG_TRACE( "Accessing codebook via operator[] for name {} (index {})", static_cast<int>( name ), index );

		if ( index >= NUM_CODEBOOKS )
		{
			SPDLOG_ERROR( "Invalid codebook name provided: {} (calculated index {} out of bounds [0-{}])",
				static_cast<int>( name ), index, NUM_CODEBOOKS - 1 );
			throw std::invalid_argument( "Invalid codebook name: " + std::to_string( static_cast<int>( name ) ) );
		}

		return m_codebooks[index];
	}

	//=====================================================================
	// Accessors
	//=====================================================================

	const Codebook& Codebooks::codebook( CodebookName name ) const
	{
		SPDLOG_TRACE( "Accessing codebook via codebook() method for name {}", static_cast<int>( name ) );

		return ( *this )[name];
	}

	VisVersion Codebooks::visVersion() const
	{
		return m_visVersion;
	}

	//=====================================================================
	// Tag creation
	//=====================================================================

	std::optional<MetadataTag> Codebooks::tryCreateTag( CodebookName name, const std::string_view value ) const
	{
		SPDLOG_DEBUG( "Attempting to create tag '{}' for codebook: {} (enum value: {})",
			value, CodebookNames::toPrefix( name ), static_cast<int>( name ) );

		try
		{
			const Codebook& cb = ( *this )[name];

			auto result = cb.tryCreateTag( value );

			if ( result.has_value() )
			{
				SPDLOG_DEBUG( "Successfully created tag for '{}' in codebook '{}'", value, CodebookNames::toPrefix( name ) );
			}
			else
			{
				SPDLOG_DEBUG( "Failed to create tag for '{}' in codebook '{}' - invalid value for codebook", value, CodebookNames::toPrefix( name ) );
			}

			return result;
		}
		catch ( [[maybe_unused]] const std::invalid_argument& ex )
		{
			SPDLOG_ERROR( "Invalid codebook name '{}' provided to tryCreateTag: {}", static_cast<int>( name ), ex.what() );
			return std::nullopt;
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Exception during tryCreateTag for '{}' in codebook '{}': {}", value, CodebookNames::toPrefix( name ), ex.what() );
			return std::nullopt;
		}
	}

	MetadataTag Codebooks::createTag( CodebookName name, const std::string& value ) const
	{
		SPDLOG_DEBUG( "Attempting to create tag (throwing version) '{}' for codebook: {}", value, CodebookNames::toPrefix( name ) );
		const Codebook& cb = ( *this )[name];

		return cb.createTag( value );
	}

	//=====================================================================
	// Iteration
	//=====================================================================

	Codebooks::Iterator Codebooks::begin() const
	{
		return Iterator( &m_codebooks, 0 );
	}

	Codebooks::Iterator Codebooks::end() const
	{
		return Iterator( &m_codebooks, NUM_CODEBOOKS );
	}
}
