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
	namespace
	{
		//=====================================================================
		// Constants
		//=====================================================================

		static constexpr const char* VIS_RELEASE_KEY = "visRelease";
	}

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
			if ( resourceName.find( "gmod" ) != std::string::npos &&
				 resourceName.find( "versioning" ) == std::string::npos &&
				 resourceName.find( ".json.gz" ) != std::string::npos )
			{
				try
				{
					auto stream = decompressedStream( resourceName );
					nlohmann::json gmodJson = nlohmann::json::parse( *stream );

					if ( gmodJson.contains( VIS_RELEASE_KEY ) && gmodJson.at( VIS_RELEASE_KEY ).is_string() )
					{
						std::string version = gmodJson.at( VIS_RELEASE_KEY ).get<std::string>();
						visVersions.push_back( version );
					}
					else
					{
						SPDLOG_WARN( "GMOD resource {} missing or has invalid '{}' field.", resourceName, VIS_RELEASE_KEY );
					}
				}
				catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
				{
					SPDLOG_ERROR( "JSON parse error in resource {}: {}", resourceName, ex.what() );
				}
				catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
				{
					SPDLOG_ERROR( "JSON error processing resource {}: {}", resourceName, ex.what() );
				}
				catch ( [[maybe_unused]] const std::exception& ex )
				{
					SPDLOG_ERROR( "Error reading/decompressing resource {} for version extraction: {}", resourceName, ex.what() );
				}
			}
		}

		std::sort( visVersions.begin(), visVersions.end() );
		visVersions.erase( std::unique( visVersions.begin(), visVersions.end() ), visVersions.end() );

		{
			std::ostringstream versionsList;
			if ( !visVersions.empty() )
			{
				std::copy( visVersions.begin(), visVersions.end() - 1, std::ostream_iterator<std::string>( versionsList, ", " ) );
				versionsList << visVersions.back();

				auto endTime = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );
			}
			else
			{
				SPDLOG_WARN( "No VIS versions found in embedded GMOD resources." );
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
				return cacheIt->second;
			}
		}

		auto names = resourceNames();
		auto searchStartTime = std::chrono::high_resolution_clock::now();

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "gmod" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.ends_with( ".json.gz" ) &&
					   name.find( "versioning" ) == std::string::npos;
			} );

		auto searchEndTime = std::chrono::high_resolution_clock::now();
		auto searchDuration = std::chrono::duration_cast<std::chrono::microseconds>( searchEndTime - searchStartTime );
		SPDLOG_DEBUG( "Resource search completed in {} μs", searchDuration.count() );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "GMOD resource not found for version: {}", visVersion );
			std::lock_guard<std::mutex> lock( gmodCacheMutex );
			gmodCache.emplace( visVersion, std::nullopt );

			return std::nullopt;
		}

		std::optional<GmodDto> resultForCache = std::nullopt;

		try
		{
			auto startTime = std::chrono::high_resolution_clock::now();

			auto stream = decompressedStream( *it );
			nlohmann::json gmodJson = nlohmann::json::parse( *stream );

			GmodDto loadedDto = GmodDto::fromJson( gmodJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

			SPDLOG_DEBUG( "Successfully loaded GMOD DTO for version {} in {} ms", visVersion, duration.count() );

			resultForCache.emplace( std::move( loadedDto ) );
		}
		catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
		{
			SPDLOG_ERROR( "JSON parse error in GMOD resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "JSON validation/deserialization error in GMOD resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Error processing GMOD resource {}: {}", *it, ex.what() );
		}

		std::lock_guard<std::mutex> lock( gmodCacheMutex );
		auto [emplaceIt, inserted] = gmodCache.emplace( visVersion, std::move( resultForCache ) );

		return emplaceIt->second;
	}

	const std::optional<std::unordered_map<std::string, GmodVersioningDto>>& EmbeddedResource::gmodVersioning()
	{
		static std::mutex gmodVersioningCacheMutex;
		static std::optional<std::unordered_map<std::string, GmodVersioningDto>> gmodVersioningCache;
		static bool cacheInitialized = false;

		{
			std::lock_guard<std::mutex> lock( gmodVersioningCacheMutex );
			if ( cacheInitialized )
			{
				return gmodVersioningCache;
			}
		}

		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();

		std::vector<std::string> matchingResources;
		for ( const auto& resourceName : names )
		{
			if ( resourceName.find( "gmod" ) != std::string::npos &&
				 resourceName.find( "versioning" ) != std::string::npos &&
				 resourceName.ends_with( ".json.gz" ) )
			{
				matchingResources.push_back( resourceName );
			}
		}

		std::unordered_map<std::string, GmodVersioningDto> resultMap;
		std::mutex resultMutex;
		bool foundAnyResource = false;

		for ( const auto& resourceName : matchingResources )
		{
			try
			{
				auto processStartTime = std::chrono::high_resolution_clock::now();

				auto stream = decompressedStream( resourceName );
				nlohmann::json versioningJson = nlohmann::json::parse( *stream );

				if ( versioningJson.contains( VIS_RELEASE_KEY ) && versioningJson.at( VIS_RELEASE_KEY ).is_string() )
				{
					std::string visVersion = versioningJson.at( VIS_RELEASE_KEY ).get<std::string>();

					auto dto = GmodVersioningDto::fromJson( versioningJson );

					{
						std::lock_guard<std::mutex> lock( resultMutex );
						resultMap.emplace( visVersion, std::move( dto ) );
						foundAnyResource = true;
					}

					auto processEndTime = std::chrono::high_resolution_clock::now();
					auto processDuration = std::chrono::duration_cast<std::chrono::milliseconds>( processEndTime - processStartTime );

					SPDLOG_DEBUG( "Loaded GMOD Versioning DTO for version {} from {} in {} ms", visVersion, resourceName, processDuration.count() );
				}
				else
				{
					SPDLOG_WARN( "GMOD Versioning resource {} missing or has invalid '{}' field.", resourceName, VIS_RELEASE_KEY );
				}
			}
			catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
			{
				SPDLOG_ERROR( "JSON parse error in GMOD Versioning resource {}: {}", resourceName, ex.what() );
			}
			catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
			{
				SPDLOG_ERROR( "JSON validation/deserialization error in GMOD Versioning resource {}: {}", resourceName, ex.what() );
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Error processing GMOD Versioning resource {}: {}", resourceName, ex.what() );
			}
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

		std::lock_guard<std::mutex> lock( gmodVersioningCacheMutex );
		cacheInitialized = true;
		if ( foundAnyResource )
		{
			gmodVersioningCache.emplace( std::move( resultMap ) );
		}
		else
		{
			SPDLOG_ERROR( "No valid GMOD Versioning resources found after {} ms", duration.count() );
			gmodVersioningCache = std::nullopt;
		}
		return gmodVersioningCache;
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
				return cacheIt->second;
			}
		}

		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "codebooks" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.ends_with( ".json.gz" );
			} );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "Codebooks resource not found for version: {}", visVersion );
			std::lock_guard<std::mutex> lock( codebooksCacheMutex );
			codebooksCache.emplace( visVersion, std::nullopt );

			return std::nullopt;
		}

		std::optional<CodebooksDto> resultForCache = std::nullopt;
		try
		{
			auto stream = decompressedStream( *it );
			nlohmann::json codebooksJson = nlohmann::json::parse( *stream );

			CodebooksDto loadedDto = CodebooksDto::fromJson( codebooksJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

			resultForCache.emplace( std::move( loadedDto ) );
		}
		catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
		{
			SPDLOG_ERROR( "JSON parse error in Codebooks resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "JSON validation/deserialization error in Codebooks resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Error processing Codebooks resource {}: {}", *it, ex.what() );
		}

		std::lock_guard<std::mutex> lock( codebooksCacheMutex );
		auto [emplaceIt, inserted] = codebooksCache.emplace( visVersion, std::move( resultForCache ) );

		return emplaceIt->second;
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
				return cacheIt->second;
			}
		}

		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&visVersion]( const std::string& name ) {
				return name.find( "locations" ) != std::string::npos &&
					   name.find( visVersion ) != std::string::npos &&
					   name.ends_with( ".json.gz" );
			} );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "Locations resource not found for version: {}", visVersion );
			std::lock_guard<std::mutex> lock( locationsCacheMutex );
			locationsCache.emplace( visVersion, std::nullopt );

			return std::nullopt;
		}

		std::optional<LocationsDto> resultForCache = std::nullopt;
		try
		{
			auto stream = decompressedStream( *it );
			nlohmann::json locationsJson = nlohmann::json::parse( *stream );

			LocationsDto loadedDto = LocationsDto::fromJson( locationsJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );
			SPDLOG_DEBUG( "Successfully loaded Locations DTO for version {} in {} ms", visVersion, duration.count() );

			resultForCache.emplace( std::move( loadedDto ) );
		}
		catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
		{
			SPDLOG_ERROR( "JSON parse error in Locations resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "JSON validation/deserialization error in Locations resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Error processing Locations resource {}: {}", *it, ex.what() );
		}

		std::lock_guard<std::mutex> lock( locationsCacheMutex );
		auto [emplaceIt, inserted] = locationsCache.emplace( visVersion, std::move( resultForCache ) );

		return emplaceIt->second;
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
				return cacheIt->second;
			}
		}

		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&version]( const std::string& name ) {
				return name.find( "data-channel-type-names" ) != std::string::npos &&
					   name.find( "iso19848" ) != std::string::npos &&
					   name.find( version ) != std::string::npos &&
					   name.ends_with( ".json.gz" );
			} );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "DataChannelTypeNames resource not found for version: {}", version );
			std::lock_guard<std::mutex> lock( dataChannelTypeNamesCacheMutex );
			dataChannelTypeNamesCache.emplace( version, std::nullopt );

			return std::nullopt;
		}

		std::optional<DataChannelTypeNamesDto> resultForCache = std::nullopt;
		try
		{
			auto stream = decompressedStream( *it );
			nlohmann::json dtNamesJson = nlohmann::json::parse( *stream );

			DataChannelTypeNamesDto loadedDto = DataChannelTypeNamesDto::fromJson( dtNamesJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );
			SPDLOG_DEBUG( "Successfully loaded DataChannelTypeNames DTO for version {} in {} ms", version, duration.count() );

			resultForCache.emplace( std::move( loadedDto ) );
		}
		catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
		{
			SPDLOG_ERROR( "JSON parse error in DataChannelTypeNames resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "JSON validation/deserialization error in DataChannelTypeNames resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Error processing DataChannelTypeNames resource {}: {}", *it, ex.what() );
		}

		std::lock_guard<std::mutex> lock( dataChannelTypeNamesCacheMutex );
		auto [emplaceIt, inserted] = dataChannelTypeNamesCache.emplace( version, std::move( resultForCache ) );

		return emplaceIt->second;
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
				return cacheIt->second;
			}
		}

		auto startTime = std::chrono::high_resolution_clock::now();
		auto names = resourceNames();

		auto it = std::find_if( names.begin(), names.end(),
			[&version]( const std::string& name ) {
				return name.find( "format-data-types" ) != std::string::npos &&
					   name.find( "iso19848" ) != std::string::npos &&
					   name.find( version ) != std::string::npos &&
					   name.ends_with( ".json.gz" );
			} );

		if ( it == names.end() )
		{
			SPDLOG_ERROR( "FormatDataTypes resource not found for version: {}", version );
			std::lock_guard<std::mutex> lock( fdTypesCacheMutex );
			fdTypesCache.emplace( version, std::nullopt );

			return std::nullopt;
		}

		std::optional<FormatDataTypesDto> resultForCache = std::nullopt;
		try
		{
			auto stream = decompressedStream( *it );
			nlohmann::json fdTypesJson = nlohmann::json::parse( *stream );

			FormatDataTypesDto loadedDto = FormatDataTypesDto::fromJson( fdTypesJson );

			auto endTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );
			SPDLOG_DEBUG( "Successfully loaded FormatDataTypes DTO for version {} in {} ms", version, duration.count() );

			resultForCache.emplace( std::move( loadedDto ) );
		}
		catch ( [[maybe_unused]] const nlohmann::json::parse_error& ex )
		{
			SPDLOG_ERROR( "JSON parse error in FormatDataTypes resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const nlohmann::json::exception& ex )
		{
			SPDLOG_ERROR( "JSON validation/deserialization error in FormatDataTypes resource {}: {}", *it, ex.what() );
		}
		catch ( [[maybe_unused]] const std::exception& ex )
		{
			SPDLOG_ERROR( "Error processing FormatDataTypes resource {}: {}", *it, ex.what() );
		}

		std::lock_guard<std::mutex> lock( fdTypesCacheMutex );
		auto [emplaceIt, inserted] = fdTypesCache.emplace( version, std::move( resultForCache ) );

		return emplaceIt->second;
	}

	//----------------------------------------------------------------------
	// Resource access implementation
	//----------------------------------------------------------------------

	std::vector<std::string> EmbeddedResource::resourceNames()
	{
		static std::mutex cacheMutex;
		static std::vector<std::string> cachedResourceNames;
		static std::optional<std::filesystem::path> successfulDir;
		static bool initialized = false;

		{
			std::lock_guard<std::mutex> lock( cacheMutex );
			if ( initialized )
			{
				return cachedResourceNames;
			}
		}
		auto startTime = std::chrono::high_resolution_clock::now();

		std::vector<std::filesystem::path> possibleDirs;

		if ( successfulDir )
		{
			possibleDirs.push_back( *successfulDir );
		}

		possibleDirs.push_back( std::filesystem::current_path() / "resources" );
		possibleDirs.push_back( std::filesystem::current_path() / "../resources" );
		possibleDirs.push_back( std::filesystem::current_path() / "../../resources" );
		possibleDirs.push_back( std::filesystem::current_path() );

		std::vector<std::string> foundNames;

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
					if ( entry.is_regular_file() )
					{
						std::string filename = entry.path().filename().string();
						if ( filename.size() > 8 && filename.substr( filename.size() - 8 ) == ".json.gz" )
						{
							foundNames.push_back( filename );
						}
					}
				}

				if ( !foundNames.empty() )
				{
					successfulDir = dir;
					break;
				}
			}
			catch ( [[maybe_unused]] const std::filesystem::filesystem_error& ex )
			{
				SPDLOG_ERROR( "Filesystem error scanning directory {}: {}", dir.string(), ex.what() );
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Error scanning directory {}: {}", dir.string(), ex.what() );
			}
		}

		if ( foundNames.empty() )
		{
			SPDLOG_WARN( "No embedded resource files (.json.gz) found in search paths." );
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );
		SPDLOG_DEBUG( "Resource names cache built in {} ms.", duration.count() );

		{
			std::lock_guard<std::mutex> lock( cacheMutex );
			cachedResourceNames = std::move( foundNames );
			initialized = true;
			return cachedResourceNames;
		}
	}

	std::shared_ptr<std::istream> EmbeddedResource::decompressedStream( const std::string& resourceName )
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		auto compressedStream = stream( resourceName );

		auto decompressedBuffer = std::make_shared<std::stringstream>();

		::z_stream zs = {};
		zs.zalloc = Z_NULL;
		zs.zfree = Z_NULL;
		zs.opaque = Z_NULL;

		if ( ::inflateInit2( &zs, 15 + 16 ) != Z_OK )
		{
			throw std::runtime_error( "Failed to initialize zlib for decompression for " + resourceName );
		}

		const size_t CHUNK_IN_SIZE = 16384;
		std::vector<char> inBuffer( CHUNK_IN_SIZE );
		const size_t CHUNK_OUT_SIZE = 32768;
		std::vector<char> outBuffer( CHUNK_OUT_SIZE );
		int ret = Z_OK;
		size_t totalCompressedRead = 0;
		size_t totalDecompressedWritten = 0;

		do
		{
			compressedStream->read( inBuffer.data(), CHUNK_IN_SIZE );
			zs.avail_in = static_cast<uInt>( compressedStream->gcount() );
			totalCompressedRead += zs.avail_in;

			if ( zs.avail_in == 0 && !compressedStream->eof() )
			{
				::inflateEnd( &zs );
				throw std::runtime_error( "Error reading compressed stream for " + resourceName );
			}

			zs.next_in = reinterpret_cast<Bytef*>( inBuffer.data() );

			do
			{
				zs.avail_out = CHUNK_OUT_SIZE;
				zs.next_out = reinterpret_cast<Bytef*>( outBuffer.data() );

				ret = ::inflate( &zs, Z_NO_FLUSH );

				if ( ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR /* Z_BUF_ERROR is ok if avail_out is 0 */ )
				{
					std::string errorMsg = ( zs.msg ) ? zs.msg : "Unknown zlib error";
					::inflateEnd( &zs );

					throw std::runtime_error( "Zlib decompression failed for resource '" + resourceName +
											  "' with error code " + std::to_string( ret ) + ": " + errorMsg );
				}

				size_t have = CHUNK_OUT_SIZE - zs.avail_out;
				decompressedBuffer->write( outBuffer.data(), static_cast<std::streamsize>( have ) );
				totalDecompressedWritten += have;
			} while ( zs.avail_out == 0 );
		} while ( ret != Z_STREAM_END );

		::inflateEnd( &zs );

		decompressedBuffer->seekg( 0 );

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

		[[maybe_unused]] double ratio = ( totalCompressedRead > 0 ) ? static_cast<double>( totalDecompressedWritten ) / static_cast<double>( totalCompressedRead ) : 0.0;
		[[maybe_unused]] double compressionPercent = ( totalDecompressedWritten > 0 ) ? ( static_cast<double>( totalCompressedRead ) * 100.0 ) / static_cast<double>( totalDecompressedWritten ) : 0.0;

		SPDLOG_DEBUG( "Decompressed resource '{}' in {} ms. Read: {} bytes, Wrote: {} bytes. Ratio: {:.2f}x. Compression: {:.1f}%.",
			resourceName, duration.count(), totalCompressedRead, totalDecompressedWritten, ratio, compressionPercent );

		SPDLOG_DEBUG( "Memory footprint: Decompressed ~{:.2f} MB", static_cast<double>( totalDecompressedWritten ) / ( 1024.0 * 1024.0 ) );

		return decompressedBuffer;
	}

	std::shared_ptr<std::istream> EmbeddedResource::stream( const std::string& resourceName )
	{
		static std::mutex pathCacheMutex;
		static std::unordered_map<std::string, std::filesystem::path> resourcePathCache;
		static std::optional<std::filesystem::path> lastSuccessfulBaseDir;
		static size_t cacheHits = 0;
		static size_t cacheMisses = 0;

		{
			std::lock_guard<std::mutex> lock( pathCacheMutex );
			auto it = resourcePathCache.find( resourceName );
			if ( it != resourcePathCache.end() )
			{
				if ( std::filesystem::exists( it->second ) && std::filesystem::is_regular_file( it->second ) )
				{
					auto fileStream = std::make_shared<std::ifstream>( it->second, std::ios::binary );
					if ( fileStream->is_open() )
					{
						cacheHits++;
						SPDLOG_DEBUG( "Resource path cache hit: '{}' -> '{}'", resourceName, it->second.string() );
						if ( ( cacheHits + cacheMisses ) % 50 == 0 && ( cacheHits + cacheMisses ) > 0 )
						{
							SPDLOG_DEBUG( "Path Cache effectiveness: {:.1f}% hit rate ({} hits, {} misses)",
								( static_cast<double>( cacheHits ) * 100.0 ) / static_cast<double>( cacheHits + cacheMisses ),
								cacheHits, cacheMisses );
						}
						return fileStream;
					}
				}
				SPDLOG_WARN( "Cached resource path '{}' for '{}' is invalid, removing from cache.", it->second.string(), resourceName );
				resourcePathCache.erase( it );
			}
			cacheMisses++;
		}

		std::vector<std::filesystem::path> possiblePaths;

		if ( lastSuccessfulBaseDir )
		{
			possiblePaths.push_back( *lastSuccessfulBaseDir / resourceName );
		}

		possiblePaths.push_back( std::filesystem::current_path() / "resources" / resourceName );
		possiblePaths.push_back( std::filesystem::current_path() / "../resources" / resourceName );
		possiblePaths.push_back( std::filesystem::current_path() / "../../resources" / resourceName );

		if ( !lastSuccessfulBaseDir || *lastSuccessfulBaseDir != std::filesystem::current_path() )
		{
			possiblePaths.push_back( std::filesystem::current_path() / resourceName );
		}

		std::string attemptedPathsStr;

		for ( const auto& path : possiblePaths )
		{
			attemptedPathsStr += "'" + path.string() + "', ";
			try
			{
				if ( std::filesystem::exists( path ) && std::filesystem::is_regular_file( path ) )
				{
					auto fileStream = std::make_shared<std::ifstream>( path, std::ios::binary );
					if ( fileStream->is_open() )
					{
						{
							std::lock_guard<std::mutex> lock( pathCacheMutex );
							resourcePathCache[resourceName] = path;
							if ( path.has_parent_path() )
							{
								lastSuccessfulBaseDir = path.parent_path();
							}
						}
						return fileStream;
					}
					else
					{
						SPDLOG_WARN( "Found file '{}' but failed to open stream.", path.string() );
					}
				}
			}
			catch ( [[maybe_unused]] const std::filesystem::filesystem_error& ex )
			{
				SPDLOG_ERROR( "Filesystem error accessing path '{}': {}", path.string(), ex.what() );
			}
			catch ( [[maybe_unused]] const std::exception& ex )
			{
				SPDLOG_ERROR( "Error checking/opening path '{}': {}", path.string(), ex.what() );
			}
		}

		if ( !attemptedPathsStr.empty() )
		{
			attemptedPathsStr.resize( attemptedPathsStr.size() - 2 ); /* Remove trailing ", " */
		}

		throw std::runtime_error( "Failed to find or open resource file: " + resourceName + ". Attempted paths: [" + attemptedPathsStr + "]" );
	}
}
