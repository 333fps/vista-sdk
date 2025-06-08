/**
 * @file TestDataLoader.h
 * @brief Helper utilities for loading test data from JSON files.
 */

#pragma once

namespace dnv::vista::sdk
{
	const simdjson::dom::element& loadTestData( const char* testDataPath );
}
