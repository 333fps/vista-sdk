#include "pch.h"

#include "dnv/vista/sdk/EmbeddedResource.h"
#include "dnv/vista/sdk/CodebooksDto.h"
#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/GmodVersioningDto.h"
#include "dnv/vista/sdk/LocationsDto.h"
#include "dnv/vista/sdk/ISO19848Dtos.h"

namespace dnv::vista::sdk
{
	std::vector<std::string> EmbeddedResource::resourceNames()
	{
		std::vector<std::string> resourceNames;

		std::vector<std::string> possibleDirs = {
			"resources/",
			"../resources/",
			"../../resources/",
			"./" };

		for ( const auto& dir : possibleDirs )
		{
			try
			{
				if ( !std::filesystem::exists( dir ) || !std::filesystem::is_directory( dir ) )
				{
					continue;
				}

				for ( const auto& entry : std::filesystem::directory_iterator( dir ) )
				{
					if ( !entry.is_regular_file() )
					{
						continue;
					}

					std::string filename = entry.path().filename().string();
					if ( filename.find( ".json.gz" ) != std::string::npos )
					{
						resourceNames.push_back( filename );
					}
				}

				if ( !resourceNames.empty() )
				{
					break;
				}

				SPDLOG_INFO( "No resources found in directory: {}", dir );
			}
			catch ( const std::exception& e )
			{
				SPDLOG_ERROR( "Error scanning directory {}: {}", dir, e.what() );
			}
		}

		return resourceNames;
	}

	std::shared_ptr<std::istream> EmbeddedResource::decompressedStream( const std::string& resourceName )
	{
		auto compressedStream = stream( resourceName );

		auto decompressedBuffer = std::make_shared<std::stringstream>();

		z_stream zs = { 0 };
		zs.zalloc = Z_NULL;
		zs.zfree = Z_NULL;
		zs.opaque = Z_NULL;

		if ( inflateInit2( &zs, 16 + MAX_WBITS ) != Z_OK )
		{
			SPDLOG_ERROR( "Failed to initialize zlib for decompression" );
			throw std::runtime_error( "Failed to initialize zlib for decompression" );
		}

		SPDLOG_INFO( "Decompressing resource: {}", resourceName );

		std::vector<char> compressedData(
			( std::istreambuf_iterator<char>( *compressedStream ) ),
			std::istreambuf_iterator<char>() );

		zs.next_in = reinterpret_cast<Bytef*>( compressedData.data() );
		zs.avail_in = static_cast<uInt>( compressedData.size() );

		const int chunkSize = 16384;
		std::vector<char> outBuffer( chunkSize );

		int ret;
		do
		{
			zs.next_out = reinterpret_cast<Bytef*>( outBuffer.data() );
			zs.avail_out = chunkSize;

			ret = inflate( &zs, Z_NO_FLUSH );

			if ( ret != Z_OK && ret != Z_STREAM_END )
			{
				inflateEnd( &zs );
				SPDLOG_ERROR( "Decompression failed with error code: {}", ret );
				throw std::runtime_error( "Decompression failed" );
			}

			decompressedBuffer->write( outBuffer.data(), chunkSize - zs.avail_out );
		} while ( ret != Z_STREAM_END );

		inflateEnd( &zs );
		decompressedBuffer->seekg( 0 );

		SPDLOG_INFO( "Decompressed size: {}", decompressedBuffer->str().size() );

		return decompressedBuffer;
	}

	std::shared_ptr<std::istream> EmbeddedResource::stream( const std::string& resourceName )
	{
		std::vector<std::string> possiblePaths = {
			"resources/" + resourceName,
			"../resources/" + resourceName,
			"../../resources/" + resourceName,
			resourceName };

		std::shared_ptr<std::ifstream> fileStream;
		std::string attemptedPaths;

		for ( const auto& path : possiblePaths )
		{
			fileStream = std::make_shared<std::ifstream>( path, std::ios::binary );
			if ( fileStream->is_open() )
			{
				SPDLOG_INFO( "Found resource at path: {}", path );
				return fileStream;
			}
			attemptedPaths += path + ", ";

			SPDLOG_INFO( "Failed to open resource file: {}", path );
		}

		SPDLOG_ERROR( "Failed to open resource file: {}. Attempted paths: {}", resourceName, attemptedPaths );
		throw std::runtime_error( "Failed to open resource file: " + resourceName +
								  ". Attempted paths: " + attemptedPaths );
	}

	std::vector<std::string> EmbeddedResource::visVersions()
	{
		auto names = resourceNames();
		std::vector<std::string> visVersions;

		for ( const auto& resourceName : names )
		{
			if ( resourceName.find( "gmod" ) != std::string::npos &&
				 resourceName.find( "versioning" ) == std::string::npos &&
				 resourceName.find( ".gz" ) != std::string::npos )
			{
				try
				{
					auto stream = decompressedStream( resourceName );
					std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ),
						std::istreambuf_iterator<char>() );

					rapidjson::Document gmodJson;
					gmodJson.Parse( jsonStr.c_str() );

					if ( !gmodJson.HasParseError() && gmodJson.HasMember( "visRelease" ) &&
						 gmodJson["visRelease"].IsString() )
					{
						visVersions.push_back( gmodJson["visRelease"].GetString() );
					}

					SPDLOG_INFO( "Found GMOD resource: {} with VIS version: {}", resourceName,
						gmodJson["visRelease"].GetString() );
				}
				catch ( const std::exception& e )
				{
					SPDLOG_ERROR( "Error parsing GMOD for version extraction: {}", e.what() );
				}
			}
		}

		return visVersions;
	}

	std::optional<GmodDto> EmbeddedResource::gmod( const std::string& visVersion )
	{
		SPDLOG_INFO( "Fetching GMOD resource for version: {}", visVersion );

		auto names = resourceNames();
		SPDLOG_INFO( "Found {} total resources to search", names.size() );

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "gmod" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos &&
					   name.find( "versioning" ) == std::string::npos;
			} );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "GMOD resource not found for version: {}", visVersion );
			return std::nullopt;
		}

		SPDLOG_INFO( "Found matching GMOD resource: {}", *it );

		try
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			auto stream = decompressedStream( *it );
			std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ),
				std::istreambuf_iterator<char>() );

			rapidjson::Document gmodJson;
			gmodJson.Parse( jsonStr.c_str() );

			if ( gmodJson.HasParseError() )
			{
				SPDLOG_ERROR( "JSON parse error at offset {}: {}",
					gmodJson.GetErrorOffset(),
					rapidjson::GetParseError_En( gmodJson.GetParseError() ) );
				return std::nullopt;
			}

			auto gmodDto = GmodDto::fromJson( gmodJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

			SPDLOG_INFO( "Successfully loaded GMOD DTO for version {} in {} ms",
				visVersion, duration.count() );

			return gmodDto;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error processing GMOD resource: {}", e.what() );
			return std::nullopt;
		}
	}

	std::optional<CodebooksDto> EmbeddedResource::codebooks( const std::string& visVersion )
	{
		SPDLOG_INFO( "Fetching Codebooks resource for version: {}", visVersion );
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "codebooks" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos;
			} );

		if ( it == names.end() )
		{
			return std::nullopt;
		}

		try
		{
			auto stream = decompressedStream( *it );
			std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ),
				std::istreambuf_iterator<char>() );

			rapidjson::Document codebooksJson;
			codebooksJson.Parse( jsonStr.c_str() );

			if ( codebooksJson.HasParseError() )
			{
				SPDLOG_ERROR( "Failed to parse Codebooks JSON." );
				return std::nullopt;
			}

			SPDLOG_INFO( "Successfully loaded Codebooks DTO for version: {} ", visVersion );

			auto result = CodebooksDto::fromJson( codebooksJson );

			return result;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing Codebooks resource: {} ", e.what() );
			return std::nullopt;
		}
	}

	std::optional<std::unordered_map<std::string, GmodVersioningDto>> EmbeddedResource::gmodVersioning()
	{
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[]( const std::string& name ) {
				return name.find( "gmod-vis-versioning" ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos;
			} );

		if ( it == names.end() )
		{
			return std::nullopt;
		}

		try
		{
			auto stream = decompressedStream( *it );
			std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ),
				std::istreambuf_iterator<char>() );

			rapidjson::Document versioningJson;
			versioningJson.Parse( jsonStr.c_str() );

			if ( versioningJson.HasParseError() )
			{
				SPDLOG_ERROR( "Failed to parse GMOD versioning JSON." );
				return std::nullopt;
			}

			std::unordered_map<std::string, GmodVersioningDto> versioningMap;
			for ( auto jsonIt = versioningJson.MemberBegin(); jsonIt != versioningJson.MemberEnd(); ++jsonIt )
			{
				GmodVersioningDto dto = GmodVersioningDto::fromJson( jsonIt->value );
				versioningMap[jsonIt->name.GetString()] = std::move( dto );
			}

			return versioningMap;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing GMOD versioning resource: {}", e.what() );
			return std::nullopt;
		}
	}

	std::optional<LocationsDto> EmbeddedResource::locations( const std::string& visVersion )
	{
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "locations" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos;
			} );

		if ( it == names.end() )
		{
			return std::nullopt;
		}

		try
		{
			auto stream = decompressedStream( *it );
			std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ),
				std::istreambuf_iterator<char>() );

			rapidjson::Document locationsJson;
			locationsJson.Parse( jsonStr.c_str() );

			if ( locationsJson.HasParseError() )
			{
				SPDLOG_ERROR( "Failed to parse Locations JSON." );
				return std::nullopt;
			}

			return LocationsDto::fromJson( locationsJson );
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing Locations resource: {}", e.what() );
			return std::nullopt;
		}
	}

	std::optional<DataChannelTypeNamesDto> EmbeddedResource::dataChannelTypeNames( const std::string& version )
	{
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&version]( const std::string& name ) {
				return name.find( "data-channel-type-names" ) != std::string::npos &&
					   name.find( "iso19848" ) != std::string::npos &&
					   name.find( version ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos;
			} );

		if ( it == names.end() )
		{
			return std::nullopt;
		}

		try
		{
			auto stream = decompressedStream( *it );
			std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ),
				std::istreambuf_iterator<char>() );

			rapidjson::Document dtNamesJson;
			dtNamesJson.Parse( jsonStr.c_str() );

			if ( dtNamesJson.HasParseError() )
			{
				SPDLOG_ERROR( "Failed to parse DataChannelTypeNames JSON." );
				return std::nullopt;
			}

			return DataChannelTypeNamesDto::fromJson( dtNamesJson );
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing DataChannelTypeNames resource: {}", e.what() );
			return std::nullopt;
		}
	}

	std::optional<FormatDataTypesDto> EmbeddedResource::formatDataTypes( const std::string& version )
	{
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&version]( const std::string& name ) {
				return name.find( "format-data-types" ) != std::string::npos &&
					   name.find( "iso19848" ) != std::string::npos &&
					   name.find( version ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos;
			} );

		if ( it == names.end() )
		{
			return std::nullopt;
		}

		try
		{
			auto stream = decompressedStream( *it );
			std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ),
				std::istreambuf_iterator<char>() );

			rapidjson::Document fdTypesJson;
			fdTypesJson.Parse( jsonStr.c_str() );

			if ( fdTypesJson.HasParseError() )
			{
				SPDLOG_ERROR( "Failed to parse FormatDataTypes JSON." );
				return std::nullopt;
			}

			return FormatDataTypesDto::fromJson( fdTypesJson );
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing FormatDataTypes resource: {}", e.what() );
			return std::nullopt;
		}
	}
}
