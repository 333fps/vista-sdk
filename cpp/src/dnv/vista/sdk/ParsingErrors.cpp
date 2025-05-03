#include "pch.h"

#include "dnv/vista/sdk/ParsingErrors.h"

namespace dnv::vista::sdk
{
	const ParsingErrors ParsingErrors::Empty = ParsingErrors();

	ParsingErrors::ParsingErrors( const std::vector<ErrorEntry>& errors )
		: m_errors( errors )
	{
		SPDLOG_TRACE( "ParsingErrors constructed with {} errors", m_errors.size() );
	}

	void ParsingErrors::addError( const std::string& type, const std::string& message )
	{
		m_errors.emplace_back( type, message );
		SPDLOG_ERROR( "Added parsing error: {} - {}", type, message );
	}

	bool ParsingErrors::hasErrors() const
	{
		return !m_errors.empty();
	}

	bool ParsingErrors::hasErrorType( const std::string& type ) const
	{
		bool found = std::any_of( m_errors.begin(), m_errors.end(),
			[&type]( const ErrorEntry& error ) { return std::get<0>( error ) == type; } );

		SPDLOG_INFO( "Error type '{}' {} found", type, found ? "was" : "was not" );
		return found;
	}

	bool ParsingErrors::isEmpty() const
	{
		return m_errors.empty();
	}

	std::string ParsingErrors::toString() const
	{
		if ( m_errors.empty() )
			return "Success";

		std::ostringstream builder;
		builder << "Parsing errors:\n";

		for ( const auto& [type, message] : m_errors )
		{
			builder << '\t' << type << " - " << message << '\n';
		}

		std::string result = builder.str();
		SPDLOG_TRACE( "ParsingErrors toString generated: {}", result );
		return result;
	}

	bool ParsingErrors::operator==( const ParsingErrors& other ) const
	{
		return m_errors == other.m_errors;
	}

	bool ParsingErrors::operator!=( const ParsingErrors& other ) const
	{
		return !( *this == other );
	}

	ParsingErrors::Iterator::Iterator( const std::vector<ErrorEntry>* data, size_t index )
		: m_data( data ), m_index( index )
	{
	}

	ParsingErrors::Iterator::reference ParsingErrors::Iterator::operator*() const
	{
		return ( *m_data )[m_index];
	}

	ParsingErrors::Iterator::pointer ParsingErrors::Iterator::operator->() const
	{
		return &( ( *m_data )[m_index] );
	}

	ParsingErrors::Iterator& ParsingErrors::Iterator::operator++()
	{
		++m_index;
		return *this;
	}

	ParsingErrors::Iterator ParsingErrors::Iterator::operator++( int )
	{
		Iterator tmp = *this;
		++( *this );
		return tmp;
	}

	bool ParsingErrors::Iterator::operator==( const Iterator& other ) const
	{
		return m_data == other.m_data && m_index == other.m_index;
	}

	bool ParsingErrors::Iterator::operator!=( const Iterator& other ) const
	{
		return !( *this == other );
	}

	ParsingErrors::Iterator ParsingErrors::begin() const
	{
		return Iterator( &m_errors, 0 );
	}

	ParsingErrors::Iterator ParsingErrors::end() const
	{
		return Iterator( &m_errors, m_errors.size() );
	}
}
