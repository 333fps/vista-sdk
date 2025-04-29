/**
 * @file EmbeddedResource.cpp
 * @brief Implementation of resource loading and caching utilities for Vista SDK
 */

#include "pch.h"

#include "dnv/vista/sdk/EmbeddedResource.h"

#include "dnv/vista/sdk/CodebooksDto.h"
#include "dnv/vista/sdk/GmodDto.h"
#include "dnv/vista/sdk/GmodVersioningDto.h"
#include "dnv/vista/sdk/ISO19848Dtos.h"
#include "dnv/vista/sdk/LocationsDto.h"

namespace dnv::vista::sdk
{
	//----------------------------------------------------------------------
	// Public interface
	//----------------------------------------------------------------------

	std::vector<std::string> EmbeddedResource::visVersions()
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();
		std::vector<std::string> visVersions;

		for ( const auto& resourceName : names )
		{
			if ( resourceName.find( "gmod-vis-versioning" ) == std::string::npos && resourceName.find( ".gz" ) != std::string::npos )
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

					SPDLOG_INFO( "Found GMOD resource: {} with VIS version: {}", resourceName, gmodJson["visRelease"].GetString() );
				}
				catch ( const std::exception& e )
				{
					SPDLOG_ERROR( "Error parsing GMOD for version extraction: {}", e.what() );
				}
			}
		}

		{
			std::ostringstream versionsList;
			if ( !visVersions.empty() )
			{
				std::copy( visVersions.begin(), visVersions.end() - 1, std::ostream_iterator<std::string>( versionsList, ", " ) );
				versionsList << visVersions.back();

				auto endTime = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

				SPDLOG_INFO( "Found {} VIS versions in {} ms: {}", visVersions.size(), duration.count(), versionsList.str() );
			}
		}

		return visVersions;
	}

	std::optional<GmodDto> EmbeddedResource::gmod( const std::string& visVersion )
	{
		static std::mutex gmodCacheMutex;
		static std::unordered_map<std::string, std::optional<GmodDto>> gmodCache;

		{
			std::lock_guard<std::mutex> lock( gmodCacheMutex );
			auto cacheIt = gmodCache.find( visVersion );
			if ( cacheIt != gmodCache.end() )
			{
				SPDLOG_DEBUG( "Using cached GMOD DTO for version: {}", visVersion );
				return cacheIt->second;
			}
		}

		SPDLOG_INFO( "Fetching GMOD resource for version: {}", visVersion );

		auto names = resourceNames();
		auto searchStartTime = std::chrono::high_resolution_clock::now();

		SPDLOG_DEBUG( "Searching through {} total resources", names.size() );

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "gmod" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos &&
					   name.find( "versioning" ) == std::string::npos;
			} );

		auto searchEndTime = std::chrono::high_resolution_clock::now();
		auto searchDuration = std::chrono::duration_cast<std::chrono::microseconds>( searchEndTime - searchStartTime );
		SPDLOG_DEBUG( "Resource search completed in {} μs", searchDuration.count() );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "GMOD resource not found for version: {}", visVersion );

			std::lock_guard<std::mutex> lock( gmodCacheMutex );
			gmodCache[visVersion] = std::nullopt;
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
				SPDLOG_ERROR( "JSON parse error at offset {}: {}", gmodJson.GetErrorOffset(), rapidjson::GetParseError_En( gmodJson.GetParseError() ) );

				std::lock_guard<std::mutex> lock( gmodCacheMutex );
				gmodCache[visVersion] = std::nullopt;
				return std::nullopt;
			}

			auto gmodDto = GmodDto::fromJson( gmodJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

			SPDLOG_INFO( "Successfully loaded GMOD DTO for version {} in {} ms", visVersion, duration.count() );

			std::lock_guard<std::mutex> lock( gmodCacheMutex );
			auto [emplaceIt, inserted] = gmodCache.emplace( visVersion, std::move( gmodDto ) );
			return emplaceIt->second;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error processing GMOD resource: {}", e.what() );

			std::lock_guard<std::mutex> lock( gmodCacheMutex );
			gmodCache[visVersion] = std::nullopt;
			return std::nullopt;
		}
	}

	std::optional<std::unordered_map<std::string, GmodVersioningDto>> EmbeddedResource::gmodVersioning()
	{
		static std::mutex gmodVersioningCacheMutex;
		static std::optional<std::unordered_map<std::string, GmodVersioningDto>> gmodVersioningCache;
		static bool cacheInitialized = false;

		{
			std::lock_guard<std::mutex> lock( gmodVersioningCacheMutex );
			if ( cacheInitialized )
			{
				SPDLOG_DEBUG( "Using cached GMOD Versioning DTO" );
				return gmodVersioningCache;
			}
		}

		SPDLOG_INFO( "Fetching GMOD Versioning resources" );
		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();

		std::vector<std::string> matchingResources;
		for ( const auto& resourceName : names )
		{
			if ( resourceName.find( "gmod" ) != std::string::npos &&
				 resourceName.find( "versioning" ) != std::string::npos &&
				 resourceName.find( ".gz" ) != std::string::npos )
			{
				matchingResources.push_back( resourceName );
			}
		}

		SPDLOG_INFO( "Found {} matching versioning resources", matchingResources.size() );

		std::unordered_map<std::string, GmodVersioningDto> result;
		std::mutex resultMutex;
		bool foundAnyResource = false;

		for ( const auto& resourceName : matchingResources )
		{
			try
			{
				auto processStartTime = std::chrono::high_resolution_clock::now();

				auto stream = decompressedStream( resourceName );
				std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ),
					std::istreambuf_iterator<char>() );

				rapidjson::Document versioningJson;
				versioningJson.Parse( jsonStr.c_str() );

				if ( versioningJson.HasParseError() )
				{
					SPDLOG_ERROR( "JSON parse error at offset {}: {}", versioningJson.GetErrorOffset(), rapidjson::GetParseError_En( versioningJson.GetParseError() ) );
					continue;
				}

				if ( versioningJson.HasMember( "visVersion" ) && versioningJson["visVersion"].IsString() )
				{
					std::string visVersion = versioningJson["visVersion"].GetString();
					auto dto = GmodVersioningDto::fromJson( versioningJson );

					std::lock_guard<std::mutex> lock( resultMutex );
					result.emplace( visVersion, std::move( dto ) );
					foundAnyResource = true;

					auto processEndTime = std::chrono::high_resolution_clock::now();
					auto processDuration = std::chrono::duration_cast<std::chrono::milliseconds>( processEndTime - processStartTime );

					SPDLOG_INFO( "Loaded GMOD Versioning DTO for version {} in {} ms", visVersion, processDuration.count() );
				}
			}
			catch ( const std::exception& e )
			{
				SPDLOG_ERROR( "Error processing GMOD Versioning resource {}: {}", resourceName, e.what() );
			}
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

		std::lock_guard<std::mutex> lock( gmodVersioningCacheMutex );
		cacheInitialized = true;

		if ( foundAnyResource )
		{
			SPDLOG_INFO( "Successfully loaded {} GMOD Versioning DTOs in {} ms", result.size(), duration.count() );
			gmodVersioningCache.emplace( std::move( result ) );
			return result;
		}
		else
		{
			SPDLOG_ERROR( "No GMOD Versioning resources found after {} ms", duration.count() );
			gmodVersioningCache = std::nullopt;
			return std::nullopt;
		}
	}

	std::optional<CodebooksDto> EmbeddedResource::codebooks( const std::string& visVersion )
	{
		static std::mutex codebooksCacheMutex;
		static std::unordered_map<std::string, std::optional<CodebooksDto>> codebooksCache;

		{
			std::lock_guard<std::mutex> lock( codebooksCacheMutex );
			auto cacheIt = codebooksCache.find( visVersion );
			if ( cacheIt != codebooksCache.end() )
			{
				SPDLOG_DEBUG( "Using cached Codebooks DTO for version: {}", visVersion );
				return cacheIt->second;
			}
		}

		SPDLOG_INFO( "Fetching Codebooks resource for version: {}", visVersion );
		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "codebooks" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos;
			} );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "Codebooks resource not found for version: {}", visVersion );

			std::lock_guard<std::mutex> lock( codebooksCacheMutex );
			codebooksCache[visVersion] = std::nullopt;
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
				SPDLOG_ERROR( "JSON parse error at offset {}: {}", codebooksJson.GetErrorOffset(), rapidjson::GetParseError_En( codebooksJson.GetParseError() ) );

				std::lock_guard<std::mutex> lock( codebooksCacheMutex );
				codebooksCache[visVersion] = std::nullopt;
				return std::nullopt;
			}

			auto result = CodebooksDto::fromJson( codebooksJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

			SPDLOG_INFO( "Successfully loaded Codebooks DTO for version {} in {} ms", visVersion, duration.count() );

			std::lock_guard<std::mutex> lock( codebooksCacheMutex );

			codebooksCache.emplace( visVersion, std::move( result ) );

			return codebooksCache.at( visVersion );
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing Codebooks resource for version {}: {}", visVersion, e.what() );

			std::lock_guard<std::mutex> lock( codebooksCacheMutex );
			codebooksCache[visVersion] = std::nullopt;
			return std::nullopt;
		}
	}

	std::optional<LocationsDto> EmbeddedResource::locations( const std::string& visVersion )
	{
		static std::mutex locationsCacheMutex;
		static std::unordered_map<std::string, std::optional<LocationsDto>> locationsCache;

		{
			std::lock_guard<std::mutex> lock( locationsCacheMutex );
			auto cacheIt = locationsCache.find( visVersion );
			if ( cacheIt != locationsCache.end() )
			{
				SPDLOG_DEBUG( "Using cached Locations DTO for version: {}", visVersion );
				return cacheIt->second;
			}
		}

		SPDLOG_INFO( "Fetching Locations resource for version: {}", visVersion );
		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "locations" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.find( ".gz" ) != std::string::npos;
			} );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "Locations resource not found for version: {}", visVersion );

			std::lock_guard<std::mutex> lock( locationsCacheMutex );

			locationsCache.erase( visVersion );
			locationsCache.emplace( visVersion, std::nullopt );

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
				SPDLOG_ERROR( "JSON parse error at offset {}: {}",
					locationsJson.GetErrorOffset(),
					rapidjson::GetParseError_En( locationsJson.GetParseError() ) );

				std::lock_guard<std::mutex> lock( locationsCacheMutex );

				locationsCache.erase( visVersion );
				locationsCache.emplace( visVersion, std::nullopt );

				return std::nullopt;
			}

			auto result = LocationsDto::fromJson( locationsJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

			SPDLOG_INFO( "Successfully loaded Locations DTO for version {} in {} ms",
				visVersion, duration.count() );

			std::lock_guard<std::mutex> lock( locationsCacheMutex );

			std::optional<LocationsDto> optResult( std::move( result ) );

			locationsCache.erase( visVersion );
			auto [cacheIt, inserted] = locationsCache.emplace( visVersion, std::move( optResult ) );

			return cacheIt->second;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing Locations resource for version {}: {}",
				visVersion, e.what() );

			std::lock_guard<std::mutex> lock( locationsCacheMutex );

			locationsCache.erase( visVersion );
			locationsCache.emplace( visVersion, std::nullopt );

			return std::nullopt;
		}
	}

	std::optional<DataChannelTypeNamesDto> EmbeddedResource::dataChannelTypeNames( const std::string& version )
	{
		static std::mutex dataChannelTypeNamesCacheMutex;
		static std::unordered_map<std::string, std::optional<DataChannelTypeNamesDto>> dataChannelTypeNamesCache;

		{
			std::lock_guard<std::mutex> lock( dataChannelTypeNamesCacheMutex );
			auto cacheIt = dataChannelTypeNamesCache.find( version );
			if ( cacheIt != dataChannelTypeNamesCache.end() )
			{
				SPDLOG_DEBUG( "Using cached DataChannelTypeNames DTO for version: {}", version );
				return cacheIt->second;
			}
		}

		SPDLOG_INFO( "Fetching DataChannelTypeNames resource for version: {}", version );
		auto startTime = std::chrono::high_resolution_clock::now();
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
			SPDLOG_ERROR( "DataChannelTypeNames resource not found for version: {}", version );

			std::lock_guard<std::mutex> lock( dataChannelTypeNamesCacheMutex );
			dataChannelTypeNamesCache[version] = std::nullopt;
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
				SPDLOG_ERROR( "JSON parse error at offset {}: {}",
					dtNamesJson.GetErrorOffset(),
					rapidjson::GetParseError_En( dtNamesJson.GetParseError() ) );

				std::lock_guard<std::mutex> lock( dataChannelTypeNamesCacheMutex );

				dataChannelTypeNamesCache.erase( version );
				dataChannelTypeNamesCache.emplace( version, std::nullopt );

				return std::nullopt;
			}

			auto result = DataChannelTypeNamesDto::fromJson( dtNamesJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

			SPDLOG_INFO( "Successfully loaded DataChannelTypeNames DTO for version {} in {} ms", version, duration.count() );

			std::optional<DataChannelTypeNamesDto> optResult( std::move( result ) );

			dataChannelTypeNamesCache.erase( version );
			auto [cacheIt, inserted] = dataChannelTypeNamesCache.emplace( version, std::move( optResult ) );
			return cacheIt->second;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing DataChannelTypeNames resource for version {}: {}", version, e.what() );

			dataChannelTypeNamesCache.erase( version );
			dataChannelTypeNamesCache.emplace( version, std::nullopt );
			return std::nullopt;
		}
	}

	std::optional<FormatDataTypesDto> EmbeddedResource::formatDataTypes( const std::string& version )
	{
		static std::mutex fdTypesCacheMutex;
		static std::unordered_map<std::string, std::optional<FormatDataTypesDto>> fdTypesCache;

		{
			std::lock_guard<std::mutex> lock( fdTypesCacheMutex );
			auto cacheIt = fdTypesCache.find( version );
			if ( cacheIt != fdTypesCache.end() )
			{
				SPDLOG_DEBUG( "Using cached FormatDataTypes DTO for version: {}", version );
				return cacheIt->second;
			}
		}

		SPDLOG_INFO( "Fetching FormatDataTypes resource for version: {}", version );
		auto startTime = std::chrono::high_resolution_clock::now();
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
			SPDLOG_ERROR( "FormatDataTypes resource not found for version: {}", version );

			std::lock_guard<std::mutex> lock( fdTypesCacheMutex );

			fdTypesCache.erase( version );
			fdTypesCache.emplace( version, std::nullopt );

			return std::nullopt;
		}

		try
		{
			auto stream = decompressedStream( *it );
			std::string jsonStr( ( std::istreambuf_iterator<char>( *stream ) ), std::istreambuf_iterator<char>() );

			rapidjson::Document fdTypesJson;
			fdTypesJson.Parse( jsonStr.c_str() );

			if ( fdTypesJson.HasParseError() )
			{
				SPDLOG_ERROR( "JSON parse error at offset {}: {}", fdTypesJson.GetErrorOffset(), rapidjson::GetParseError_En( fdTypesJson.GetParseError() ) );

				std::lock_guard<std::mutex> lock( fdTypesCacheMutex );

				fdTypesCache.erase( version );
				fdTypesCache.emplace( version, std::nullopt );

				return std::nullopt;
			}

			auto result = FormatDataTypesDto::fromJson( fdTypesJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

			SPDLOG_INFO( "Successfully loaded FormatDataTypes DTO for version {} in {} ms", version, duration.count() );

			std::optional<FormatDataTypesDto> optResult( std::move( result ) );

			std::lock_guard<std::mutex> lock( fdTypesCacheMutex );

			fdTypesCache.erase( version );
			auto [cacheIt, inserted] = fdTypesCache.emplace( version, std::move( optResult ) );

			return cacheIt->second;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error parsing FormatDataTypes resource for version {}: {}", version, e.what() );

			std::lock_guard<std::mutex> lock( fdTypesCacheMutex );

			fdTypesCache.erase( version );
			fdTypesCache.emplace( version, std::nullopt );

			return std::nullopt;
		}
	}

	//----------------------------------------------------------------------
	// Resource access implementation
	//----------------------------------------------------------------------

	std::vector<std::string> EmbeddedResource::resourceNames()
	{
		static std::mutex cacheMutex;
		static std::vector<std::string> cachedResourceNames;
		static std::optional<std::string> successfulDir;
		static bool initialized = false;

		{
			std::lock_guard<std::mutex> lock( cacheMutex );

			if ( initialized )
			{
				SPDLOG_DEBUG( "Using cached resource names ({} entries)", cachedResourceNames.size() );
				return cachedResourceNames;
			}
		}

		SPDLOG_INFO( "Building resource names cache" );

		std::vector<std::string> possibleDirs{
			"resources/",
			"../resources/",
			"../../resources/",
			"./" };

		if ( successfulDir )
		{
			possibleDirs.insert( possibleDirs.begin(), *successfulDir );
			SPDLOG_DEBUG( "Trying previously successful directory first: {}", *successfulDir );
		}

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
						cachedResourceNames.push_back( filename );
					}
				}

				if ( !cachedResourceNames.empty() )
				{
					SPDLOG_INFO( "Found resources in directory: {}", dir );
					successfulDir = dir;
					break;
				}

				SPDLOG_INFO( "No resources found in directory: {}", dir );
			}
			catch ( const std::exception& e )
			{
				SPDLOG_ERROR( "Error scanning directory {}: {}", dir, e.what() );
			}
		}

		for ( const auto& n : cachedResourceNames )
		{
			SPDLOG_INFO( "Found resource: {}", n );
		}

		{
			std::lock_guard<std::mutex> lock( cacheMutex );
			initialized = true;
		}

		return cachedResourceNames;
	}

	std::shared_ptr<std::istream> EmbeddedResource::decompressedStream( const std::string& resourceName )
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		auto compressedStream = stream( resourceName );

		auto decompressedBuffer = std::make_shared<std::stringstream>();

		::z_stream zs = { 0 };
		zs.zalloc = Z_NULL;
		zs.zfree = Z_NULL;
		zs.opaque = Z_NULL;

		if ( ::inflateInit2( &zs, 16 + MAX_WBITS ) != Z_OK )
		{
			SPDLOG_ERROR( "Failed to initialize zlib for decompression" );
			throw std::runtime_error( "Failed to initialize zlib for decompression" );
		}

		SPDLOG_INFO( "Decompressing resource: {}", resourceName );

		std::vector<char> compressedData(
			( std::istreambuf_iterator<char>( *compressedStream ) ),
			std::istreambuf_iterator<char>() );

		const size_t compressedSize = compressedData.size();
		const size_t estimatedSize = compressedSize * 8;
		std::string decompressedStr;
		decompressedStr.reserve( estimatedSize );

		zs.next_in = reinterpret_cast<Bytef*>( compressedData.data() );
		zs.avail_in = static_cast<uInt>( compressedData.size() );

		const int chunkSize = 16384;
		std::vector<char> outBuffer( chunkSize );

		int ret;
		do
		{
			zs.next_out = reinterpret_cast<::Bytef*>( outBuffer.data() );
			zs.avail_out = chunkSize;

			ret = inflate( &zs, Z_NO_FLUSH );

			if ( ret != Z_OK && ret != Z_STREAM_END )
			{
				::inflateEnd( &zs );
				SPDLOG_ERROR( "Decompression failed with error code: {}", ret );
				throw std::runtime_error( "Decompression failed" );
			}

			decompressedBuffer->write( outBuffer.data(), chunkSize - zs.avail_out );
		} while ( ret != Z_STREAM_END );

		::inflateEnd( &zs );
		decompressedBuffer->seekg( 0 );

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

		const size_t decompressedSize = decompressedBuffer->str().size();

		SPDLOG_INFO( "Decompressed resource {} in {} ms - Compressed: {} bytes, Decompressed: {} bytes, Ratio: {:.2f}x",
			resourceName, duration.count(), compressedSize, decompressedSize,
			static_cast<double>( decompressedSize ) / static_cast<double>( compressedSize > 0 ? compressedSize : 1 ) );

		SPDLOG_INFO( "Memory footprint: {:.2f} MB ({:.1f} kB) decompressed from {:.2f} MB ({:.1f} kB) with {:.1f}% compression ratio",
			static_cast<double>( decompressedSize ) / ( 1024.0 * 1024.0 ),
			static_cast<double>( decompressedSize ) / 1024.0,
			static_cast<double>( compressedSize ) / ( 1024.0 * 1024.0 ),
			static_cast<double>( compressedSize ) / 1024.0,
			( static_cast<double>( compressedSize ) * 100.0 ) / static_cast<double>( decompressedSize ) );

		return decompressedBuffer;
	}

	std::shared_ptr<std::istream> EmbeddedResource::stream( const std::string& resourceName )
	{
		static std::mutex pathCacheMutex;
		static std::unordered_map<std::string, std::string> resourcePathCache;
		static std::optional<std::string> lastSuccessfulBaseDir;
		static size_t cacheHits = 0;
		static size_t cacheMisses = 0;

		{
			std::lock_guard<std::mutex> lock( pathCacheMutex );
			auto it = resourcePathCache.find( resourceName );
			if ( it != resourcePathCache.end() )
			{
				auto fileStream = std::make_shared<std::ifstream>( it->second, std::ios::binary );
				if ( fileStream->is_open() )
				{
					cacheHits++;
					SPDLOG_DEBUG( "Resource path cache hit: {} -> {}", resourceName, it->second );
					return fileStream;
				}
				resourcePathCache.erase( it );
				SPDLOG_INFO( "Cached resource path invalid, removing from cache: {}", it->second );
			}
			cacheMisses++;
		}

		std::vector<std::string> possiblePaths;
		if ( lastSuccessfulBaseDir )
		{
			possiblePaths.push_back( *lastSuccessfulBaseDir + resourceName );
		}

		possiblePaths.insert( possiblePaths.end(), { "resources/" + resourceName,
													   "../resources/" + resourceName,
													   "../../resources/" + resourceName,
													   resourceName } );

		std::shared_ptr<std::ifstream> fileStream;
		std::string attemptedPaths;

		for ( const auto& path : possiblePaths )
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			fileStream = std::make_shared<std::ifstream>( path, std::ios::binary );
			if ( fileStream->is_open() )
			{
				auto endTime = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::microseconds>( endTime - startTime );

				SPDLOG_INFO( "Found resource at path: {} (opened in {} μs)", path, duration.count() );

				size_t lastSlash = path.find_last_of( "/\\" );
				if ( lastSlash != std::string::npos )
				{
					lastSuccessfulBaseDir = path.substr( 0, lastSlash + 1 );
					SPDLOG_DEBUG( "Caching base directory: {}", *lastSuccessfulBaseDir );
				}

				{
					std::lock_guard<std::mutex> lock( pathCacheMutex );
					resourcePathCache[resourceName] = path;

					SPDLOG_INFO( "Cache effectiveness: {:.1f}% hit rate ({} hits, {} misses)",
						( static_cast<double>( cacheHits ) * 100.0 ) / static_cast<double>( cacheHits + cacheMisses ),
						cacheHits, cacheMisses );
				}

				return fileStream;
			}
			attemptedPaths += path + ", ";

			SPDLOG_DEBUG( "Failed to open resource file: {}", path );
		}

		SPDLOG_ERROR( "Failed to open resource file: {}. Attempted paths: {}", resourceName, attemptedPaths );
		throw std::runtime_error( "Failed to open resource file: " + resourceName + ". Attempted paths: " + attemptedPaths );
	}
}
