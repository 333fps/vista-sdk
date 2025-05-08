#include "pch.h"

#include "dnv/vista/sdk/VisVersion.h"

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
				return false;
			default:
				return false;
		}
	}

	std::string VisVersionExtensions::toVersionString( VisVersion version )
	{
		switch ( version )
		{
			case VisVersion::v3_4a:
				return "3-4a";
			case VisVersion::v3_5a:
				return "3-5a";
			case VisVersion::v3_6a:
				return "3-6a";
			case VisVersion::v3_7a:
				return "3-7a";
			case VisVersion::v3_8a:
				return "3-8a";
			case VisVersion::Unknown:
				return "Unknown";
			default:
			{
				auto errorMsg = "Invalid VIS version: " + std::to_string( static_cast<int>( version ) );
				SPDLOG_ERROR( errorMsg );
				throw std::invalid_argument( errorMsg );
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

		auto it{ versionMap.find( versionString ) };
		if ( it != versionMap.end() )
		{
			version = it->second;
			return true;
		}

		std::string normalizedVersion = versionString;
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
			SPDLOG_ERROR( "Invalid VIS version string: {}", versionString );
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
		auto versions{ allVersions() };
		return versions.empty() ? VisVersion::Unknown : versions.back();
	}
}
