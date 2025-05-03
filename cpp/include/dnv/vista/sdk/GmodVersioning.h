#pragma once

#include "GmodVersioningDto.h"
#include "dnv/vista/sdk/GmodPath.h"

#include "LocalId.h"

namespace dnv::vista::sdk
{
	class GmodNode;
	class LocalIdBuilder;
	enum class VisVersion;

	/**
	 * @brief Class responsible for converting GMOD objects to a higher version
	 */
	class GmodVersioning final
	{
	public:
		/**
		 * @brief Constructor
		 *
		 * @param dto Dictionary of GmodVersioningDto objects indexed by version string
		 */
		explicit GmodVersioning( const std::unordered_map<std::string, GmodVersioningDto>& dto );

		//-------------------------------------------------------------------------
		// Public Conversion Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Convert a GmodNode from one version to a higher version
		 * @throws std::invalid_argument If targetVersion is not higher than sourceVersion
		 */
		[[nodiscard]] std::optional<GmodNode> convertNode(
			VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const;

		/**
		 * @brief Converts a GmodPath from a source VIS version to a target VIS version.
		 * @param sourceVersion The VIS version of the sourcePath.
		 * @param sourcePath The GmodPath to convert (passed by const reference).
		 * @param targetVersion The target VIS version.
		 * @return An optional containing the converted GmodPath if successful, otherwise std::nullopt.
		 * @throws std::invalid_argument if source or target versions are invalid or source >= target.
		 */
		// CHANGE return type from GmodPath to std::optional<GmodPath>
		[[nodiscard]] std::optional<GmodPath> convertPath(
			VisVersion sourceVersion, const GmodPath& sourcePath, VisVersion targetVersion ) const;

		/**
		 * @brief Convert a LocalIdBuilder from one version to a higher version
		 */
		std::optional<LocalIdBuilder> convertLocalId( const LocalIdBuilder& sourceLocalId, VisVersion targetVersion ) const;

		/**
		 * @brief Convert a LocalId from one version to a higher version
		 */
		std::optional<LocalId> convertLocalId( const LocalId& sourceLocalId, VisVersion targetVersion ) const;

	private:
		//-------------------------------------------------------------------------
		// Private Type Definitions
		//-------------------------------------------------------------------------

		/**
		 * @brief Enumeration of conversion operation types
		 */
		enum class ConversionType
		{
			ChangeCode = 0,
			Merge,
			Move,
			AssignmentChange = 20,
			AssignmentDelete = 21
		};

		/**
		 * @brief Structure holding information about a node conversion
		 */
		struct GmodNodeConversion final
		{
			std::unordered_set<ConversionType> operations;
			std::string source;
			std::optional<std::string> target;
			std::optional<std::string> oldAssignment;
			std::optional<std::string> newAssignment;
			std::optional<bool> deleteAssignment;
		};

		/**
		 * @brief Class representing versioning information for a specific VIS version
		 */
		class GmodVersioningNode final
		{
		public:
			/**
			 * @brief Constructor for GmodVersioningNode
			 */
			GmodVersioningNode( VisVersion visVersion,
				const std::unordered_map<std::string, GmodNodeConversionDto>& dto );

			/**
			 * @brief Get the VIS version of this versioning node
			 */
			VisVersion visVersion() const;

			/**
			 * @brief Try to get code changes for a specific node code
			 */
			bool tryGetCodeChanges( const std::string& code, GmodNodeConversion& nodeChanges ) const;

		private:
			VisVersion m_visVersion;
			std::unordered_map<std::string, GmodNodeConversion> m_versioningNodeChanges;
		};

		//-------------------------------------------------------------------------
		// Private Data Members
		//-------------------------------------------------------------------------

		std::unordered_map<VisVersion, GmodVersioningNode> m_versioningsMap;

		//-------------------------------------------------------------------------
		// Private Helper Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Internal implementation for converting a node between adjacent versions
		 */
		[[nodiscard]] std::optional<GmodNode> convertNodeInternal(
			VisVersion sourceVersion, const GmodNode& sourceNode, VisVersion targetVersion ) const;

		/**
		 * @brief Try to get a versioning node for a specific VIS version
		 */
		[[nodiscard]] const GmodVersioningNode* tryGetVersioningNode( VisVersion visVersion ) const;

		//-------------------------------------------------------------------------
		// Private Validation Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Validate source and target versions
		 */
		void validateSourceAndTargetVersions( VisVersion sourceVersion, VisVersion targetVersion ) const;

		/**
		 * @brief Validate source and target version pair
		 */
		void validateSourceAndTargetVersionPair( VisVersion sourceVersion, VisVersion targetVersion ) const;

		//-------------------------------------------------------------------------
		// Private Static Utility Methods
		//-------------------------------------------------------------------------

		/**
		 * @brief Parse a conversion type from a string
		 */
		static ConversionType parseConversionType( const std::string& type );
	};
}
