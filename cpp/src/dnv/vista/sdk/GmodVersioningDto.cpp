#include "pch.h"

#include "dnv/vista/sdk/GmodVersioningDto.h"

namespace dnv::vista::sdk
{
	GmodVersioningAssignmentChangeDto::GmodVersioningAssignmentChangeDto( std::string oldAssignment, std::string currentAssignment )
		: oldAssignment( std::move( oldAssignment ) ),
		  currentAssignment( std::move( currentAssignment ) )
	{
	}

	GmodVersioningAssignmentChangeDto GmodVersioningAssignmentChangeDto::fromJson( const rapidjson::Value& json )
	{
		SPDLOG_INFO( "Parsing assignment change from JSON" );
		GmodVersioningAssignmentChangeDto dto;

		if ( json.HasMember( "oldAssignment" ) && json["oldAssignment"].IsString() )
			dto.oldAssignment = json["oldAssignment"].GetString();

		if ( json.HasMember( "currentAssignment" ) && json["currentAssignment"].IsString() )
			dto.currentAssignment = json["currentAssignment"].GetString();

		SPDLOG_INFO( "Parsed assignment change: {} → {}", dto.oldAssignment, dto.currentAssignment );
		return dto;
	}

	bool GmodVersioningAssignmentChangeDto::tryFromJson( const rapidjson::Value& json,
		GmodVersioningAssignmentChangeDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing GmodVersioningAssignmentChangeDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value GmodVersioningAssignmentChangeDto::toJson(
		rapidjson::Document::AllocatorType& allocator ) const
	{
		SPDLOG_INFO( "Serializing assignment change to JSON" );
		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( "oldAssignment", rapidjson::Value( oldAssignment.c_str(), allocator ).Move(), allocator );
		obj.AddMember( "currentAssignment", rapidjson::Value( currentAssignment.c_str(), allocator ).Move(), allocator );

		return obj;
	}

	GmodNodeConversionDto::GmodNodeConversionDto( std::unordered_set<std::string> operations, std::string source, std::string target, std::string oldAssignment, std::string newAssignment, bool deleteAssignment )
		: operations( std::move( operations ) ),
		  source( std::move( source ) ),
		  target( std::move( target ) ),
		  oldAssignment( std::move( oldAssignment ) ),
		  newAssignment( std::move( newAssignment ) ),
		  deleteAssignment( deleteAssignment )
	{
	}

	GmodNodeConversionDto GmodNodeConversionDto::fromJson( const rapidjson::Value& json )
	{
		SPDLOG_INFO( "Parsing node conversion from JSON" );
		GmodNodeConversionDto dto;

		if ( json.HasMember( "operations" ) && json["operations"].IsArray() )
		{
			size_t opCount = json["operations"].Size();
			SPDLOG_INFO( "Found {} operations", opCount );
			dto.operations.reserve( opCount );

			for ( const auto& operation : json["operations"].GetArray() )
			{
				if ( operation.IsString() )
					dto.operations.insert( operation.GetString() );
			}
		}

		if ( json.HasMember( "source" ) && json["source"].IsString() )
			dto.source = json["source"].GetString();

		if ( json.HasMember( "target" ) && json["target"].IsString() )
			dto.target = json["target"].GetString();

		if ( json.HasMember( "oldAssignment" ) && json["oldAssignment"].IsString() )
			dto.oldAssignment = json["oldAssignment"].GetString();

		if ( json.HasMember( "newAssignment" ) && json["newAssignment"].IsString() )
			dto.newAssignment = json["newAssignment"].GetString();

		if ( json.HasMember( "deleteAssignment" ) && json["deleteAssignment"].IsBool() )
			dto.deleteAssignment = json["deleteAssignment"].GetBool();

		SPDLOG_INFO( "Parsed node conversion: source={}, target={}, operations={}",
			dto.source, dto.target, dto.operations.size() );
		return dto;
	}

	bool GmodNodeConversionDto::tryFromJson( const rapidjson::Value& json, GmodNodeConversionDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing GmodNodeConversionDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value GmodNodeConversionDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		SPDLOG_INFO( "Serializing node conversion to JSON" );
		rapidjson::Value obj( rapidjson::kObjectType );

		rapidjson::Value operationsArray( rapidjson::kArrayType );
		for ( const auto& operation : operations )
		{
			operationsArray.PushBack( rapidjson::Value( operation.c_str(), allocator ).Move(), allocator );
		}
		obj.AddMember( "operations", operationsArray, allocator );

		obj.AddMember( "source", rapidjson::Value( source.c_str(), allocator ).Move(), allocator );
		obj.AddMember( "target", rapidjson::Value( target.c_str(), allocator ).Move(), allocator );
		obj.AddMember( "oldAssignment", rapidjson::Value( oldAssignment.c_str(), allocator ).Move(), allocator );
		obj.AddMember( "newAssignment", rapidjson::Value( newAssignment.c_str(), allocator ).Move(), allocator );
		obj.AddMember( "deleteAssignment", rapidjson::Value( deleteAssignment ).Move(), allocator );

		return obj;
	}

	GmodVersioningDto::GmodVersioningDto( std::string visVersion, std::unordered_map<std::string, GmodNodeConversionDto> items )
		: visVersion( std::move( visVersion ) ),
		  items( std::move( items ) )
	{
	}

	GmodVersioningDto GmodVersioningDto::fromJson( const rapidjson::Value& json )
	{
		SPDLOG_INFO( "Parsing GMOD versioning data from JSON" );
		GmodVersioningDto dto;

		if ( json.HasMember( "visRelease" ) && json["visRelease"].IsString() )
		{
			dto.visVersion = json["visRelease"].GetString();
			SPDLOG_INFO( "GMOD versioning for VIS version: {}", dto.visVersion );
		}

		if ( json.HasMember( "items" ) && json["items"].IsObject() )
		{
			size_t itemCount = json["items"].MemberCount();
			SPDLOG_INFO( "Found {} node conversion items to parse", itemCount );
			dto.items.reserve( itemCount );

			size_t successCount = 0;
			for ( auto it = json["items"].MemberBegin(); it != json["items"].MemberEnd(); ++it )
			{
				if ( it->value.IsObject() )
				{
					try
					{
						dto.items[it->name.GetString()] = GmodNodeConversionDto::fromJson( it->value );
						successCount++;
					}
					catch ( const std::exception& e )
					{
						SPDLOG_ERROR( "Error parsing conversion item '{}': {}",
							it->name.GetString(), e.what() );
					}
				}
			}

			SPDLOG_INFO( "Successfully parsed {}/{} node conversion items",
				successCount, itemCount );
		}
		else
		{
			SPDLOG_WARN( "No 'items' object found in GMOD versioning data" );
		}

		return dto;
	}

	bool GmodVersioningDto::tryFromJson( const rapidjson::Value& json, GmodVersioningDto& dto )
	{
		try
		{
			dto = fromJson( json );
			return true;
		}
		catch ( const std::exception& e )
		{
			SPDLOG_ERROR( "Error deserializing GmodVersioningDto: {}", e.what() );
			return false;
		}
	}

	rapidjson::Value GmodVersioningDto::toJson( rapidjson::Document::AllocatorType& allocator ) const
	{
		SPDLOG_INFO( "Serializing GMOD versioning data to JSON, {} items", items.size() );
		rapidjson::Value obj( rapidjson::kObjectType );

		obj.AddMember( "visRelease", rapidjson::Value( visVersion.c_str(), allocator ).Move(), allocator );

		rapidjson::Value itemsObj( rapidjson::kObjectType );
		for ( const auto& [key, value] : items )
		{
			itemsObj.AddMember(
				rapidjson::Value( key.c_str(), allocator ).Move(),
				value.toJson( allocator ),
				allocator );
		}
		obj.AddMember( "items", itemsObj, allocator );

		SPDLOG_INFO( "Successfully serialized GMOD versioning data for VIS {}", visVersion );
		return obj;
	}
}
