#include "pch.h"

#include "dnv/vista/sdk/VISVersion.h"

namespace dnv::vista::sdk
{
	bool VisVersionExtensions::isValid( VisVersion version )
	{
		switch ( version )
		{
			case VisVersion::v3_4a:
			case VisVersion::v3_5a:
			case VisVersion::v3_6a:
			case VisVersion::v3_7a:
			case VisVersion::v3_8a:
				return true;
			case VisVersion::Unknown:
			case VisVersion::COUNT_VALID:
			default:
				return false;
		}
	}

	std::string VisVersionExtensions::toVersionString( VisVersion version )
	{
		switch ( version )
		{
			case VisVersion::v3_4a:
				return "vis-3-4a";
			case VisVersion::v3_5a:
				return "vis-3-5a";
			case VisVersion::v3_6a:
				return "vis-3-6a";
			case VisVersion::v3_7a:
				return "vis-3-7a";
			case VisVersion::v3_8a:
				return "vis-3-8a";
			case VisVersion::Unknown:
				return "Unknown";
			case VisVersion::COUNT_VALID:
			default:
			{
				throw std::invalid_argument( "Invalid VIS version: " + std::to_string( static_cast<int>( version ) ) );
			}
		}
	}

	bool VisVersionExtensions::tryParse( const std::string& versionString, VisVersion& version )
	{
		static const std::unordered_map<std::string, VisVersion> versionMap = {
			{ "3.4a", VisVersion::v3_4a },
			{ "3.5a", VisVersion::v3_5a },
			{ "3.6a", VisVersion::v3_6a },
			{ "3.7a", VisVersion::v3_7a },
			{ "3.8a", VisVersion::v3_8a },

			{ "3-4a", VisVersion::v3_4a },
			{ "3-5a", VisVersion::v3_5a },
			{ "3-6a", VisVersion::v3_6a },
			{ "3-7a", VisVersion::v3_7a },
			{ "3-8a", VisVersion::v3_8a } };

		std::string normalizedVersion = versionString;

		if ( normalizedVersion.rfind( "vis-", 0 ) == 0 )
		{
			normalizedVersion = normalizedVersion.substr( 4 );
		}

		auto it = versionMap.find( normalizedVersion );
		if ( it != versionMap.end() )
		{
			version = it->second;
			return true;
		}

		std::replace( normalizedVersion.begin(), normalizedVersion.end(), '-', '.' );
		if ( normalizedVersion != versionString )
		{
			return tryParse( normalizedVersion, version );
		}

		return false;
	}

	VisVersion VisVersionExtensions::parse( const std::string& versionString )
	{
		VisVersion result;
		if ( !tryParse( versionString, result ) )
		{
			throw std::invalid_argument( "Invalid VIS version string: " + versionString );
		}
		return result;
	}

	std::vector<VisVersion> VisVersionExtensions::allVersions()
	{
		static const std::array<VisVersion, 5> allVersions = {
			VisVersion::v3_4a,
			VisVersion::v3_5a,
			VisVersion::v3_6a,
			VisVersion::v3_7a,
			VisVersion::v3_8a };

		return std::vector<VisVersion>( allVersions.begin(), allVersions.end() );
	}

	VisVersion VisVersionExtensions::latestVersion()
	{
		return VisVersion::LATEST;
	}
}
