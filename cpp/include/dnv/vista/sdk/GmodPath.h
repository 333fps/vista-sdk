/**
 * @file GmodPath.h
 * @brief Declarations for GmodPath and related classes for representing paths in the Generic Product Model (GMOD).
 * @details Defines the `GmodPath` class for representing hierarchical paths according to ISO 19848,
 *          along with helper classes for parsing, validation, iteration, and modification of path segments.
 */

#pragma once

namespace dnv::vista::sdk
{
	//=====================================================================
	// Forward declarations
	//=====================================================================

	class Gmod;
	class GmodNode;
	class GmodParsePathResult;
	class GmodIndividualizableSet;
	class Location;
	class Locations;

	enum class TraversalHandlerResult;
	enum class VisVersion;

	namespace internal
	{
		struct ParseContext;

		dnv::vista::sdk::TraversalHandlerResult parseInternalTraversalHandler(
			ParseContext& context,
			const std::vector<const GmodNode*>& traversedParents,
			const GmodNode& currentNode );
	}

	//=====================================================================
	// GmodPath Class
	//=====================================================================

	class GmodPath final
	{
		//----------------------------------------------
		// Forward declarations
		//----------------------------------------------

		friend class GmodIndividualizableSet;

		friend dnv::vista::sdk::TraversalHandlerResult internal::parseInternalTraversalHandler(
			internal::ParseContext&,
			const std::vector<const GmodNode*>&,
			const GmodNode& );

	public:
		class Enumerator;

		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		GmodPath( const Gmod& gmod, GmodNode* node, std::vector<GmodNode*> parents = {} );

		GmodPath();
		GmodPath( const GmodPath& other );
		GmodPath( GmodPath&& other ) noexcept;

		~GmodPath();

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		GmodPath& operator=( const GmodPath& other );
		GmodPath& operator=( GmodPath&& other ) noexcept;

		//----------------------------------------------
		// Equality Operators
		//----------------------------------------------

		[[nodiscard]] bool operator==( const GmodPath& other ) const noexcept;
		[[nodiscard]] bool operator!=( const GmodPath& other ) const noexcept;

		//----------------------------------------------
		// Lookup Operators
		//----------------------------------------------

		[[nodiscard]] GmodNode* operator[]( size_t index ) const;
		[[nodiscard]] GmodNode*& operator[]( size_t index );

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		VisVersion visVersion() const noexcept;
		size_t hashCode() const noexcept;

		[[nodiscard]] const Gmod* gmod() const noexcept;
		[[nodiscard]] GmodNode* node() const noexcept;
		[[nodiscard]] const std::vector<GmodNode*>& parents() const noexcept;

		[[nodiscard]] size_t length() const noexcept;
		[[nodiscard]] GmodNode* rootNode() const noexcept;
		[[nodiscard]] GmodNode* parentNode() const noexcept;
		[[nodiscard]] std::vector<GmodIndividualizableSet> individualizableSets() const;

		[[nodiscard]] std::optional<std::string> normalAssignmentName( size_t nodeDepth ) const;
		[[nodiscard]] std::vector<std::pair<size_t, std::string>> commonNames() const;

		[[nodiscard]] Enumerator fullPath() const;
		[[nodiscard]] Enumerator fullPathFrom( size_t fromDepth ) const;

		//----------------------------------------------
		// Utility Methods
		//----------------------------------------------

		[[nodiscard]] static bool isValid( const std::vector<GmodNode*>& parents, const GmodNode& node );
		[[nodiscard]] static bool isValid( const std::vector<GmodNode*>& parents, const GmodNode& node, size_t& missingLinkAt );
		[[nodiscard]] bool isMappable() const;

		[[nodiscard]] std::string toString() const;
		[[nodiscard]] std::string toStringDump() const;
		[[nodiscard]] std::string toFullPathString() const;

		void toString( std::stringstream& builder, char separator = '/' ) const;
		void toStringDump( std::stringstream& builder ) const;
		void toFullPathString( std::stringstream& builder ) const;

		[[nodiscard]] GmodPath withoutLocations() const;

		//----------------------------------------------
		// Public Static Parsing Methods
		//----------------------------------------------

		[[nodiscard]] static GmodPath parse( std::string_view pathString, VisVersion visVersion );
		[[nodiscard]] static GmodPath parse( std::string_view pathString, const Gmod& gmod, const Locations& locations );
		[[nodiscard]] static GmodPath parseFullPath( std::string_view pathString, VisVersion visVersion );

		[[nodiscard]] static bool tryParse( std::string_view pathString, VisVersion visVersion, std::optional<GmodPath>& outPath );
		[[nodiscard]] static bool tryParse( std::string_view pathString, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath );
		[[nodiscard]] static bool tryParseFullPath( std::string_view pathString, VisVersion visVersion, std::optional<GmodPath>& outPath );
		[[nodiscard]] static bool tryParseFullPath( std::string_view pathString, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath );

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		VisVersion m_visVersion;
		const Gmod* m_gmod;
		GmodNode* m_node;
		std::vector<GmodNode*> m_parents;
		std::vector<GmodNode*> m_ownedNodes;

	private:
		//----------------------------------------------
		// Private Static Parsing Methods
		//----------------------------------------------

		static std::unique_ptr<GmodParsePathResult> parseInternal(
			std::string_view item, const Gmod& gmod, const Locations& locations );

		static std::unique_ptr<GmodParsePathResult> parseFullPathInternal(
			std::string_view item, const Gmod& gmod, const Locations& locations );

	public:
		//----------------------------------------------
		// GmodPath::Enumerator Class
		//----------------------------------------------

		class Enumerator final
		{
			friend class GmodPath;

		private:
			Enumerator( const GmodPath* pathInst, size_t startIndex = std::numeric_limits<size_t>::max() );

		public:
			using iterator_category = std::input_iterator_tag;
			using value_type = std::pair<size_t, GmodNode*>;
			using difference_type = std::ptrdiff_t;
			using pointer = const value_type*;
			using reference = const value_type&;

			bool operator!=( const Enumerator& other ) const;
			bool operator==( const Enumerator& other ) const;

			value_type operator*() const;
			Enumerator& operator++();

			Enumerator() = delete;
			Enumerator( const Enumerator& );
			Enumerator( Enumerator&& ) noexcept = default;
			Enumerator& operator=( const Enumerator& ) = default;
			Enumerator& operator=( Enumerator&& ) noexcept = default;

			Enumerator& begin();
			Enumerator end() const;

			[[nodiscard]] GmodNode* current() const;
			bool next();
			void reset();

		private:
			const GmodPath* m_pathInstance;
			size_t m_currentIndex;
		};
	};

	//=====================================================================
	// GmodIndividualizableSet Class
	//=====================================================================

	class GmodIndividualizableSet final
	{
	public:
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------

		GmodIndividualizableSet( const std::vector<int>& nodeIndices, const GmodPath& sourcePath );

		GmodIndividualizableSet() = delete;
		GmodIndividualizableSet( const GmodIndividualizableSet& ) = delete;
		GmodIndividualizableSet( GmodIndividualizableSet&& ) noexcept = default;

		~GmodIndividualizableSet() = default;

		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

		GmodIndividualizableSet& operator=( const GmodIndividualizableSet& ) = delete;
		GmodIndividualizableSet& operator=( GmodIndividualizableSet&& ) noexcept = delete;

		//----------------------------------------------
		// Build
		//----------------------------------------------

		GmodPath build();

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		[[nodiscard]] std::vector<GmodNode*> nodes() const;
		[[nodiscard]] const std::vector<int>& nodeIndices() const noexcept;
		[[nodiscard]] std::optional<Location> location() const;

		//----------------------------------------------
		// Utility Methods
		//----------------------------------------------

		void setLocation( const std::optional<Location>& location );
		[[nodiscard]] std::string toString() const;

	private:
		//----------------------------------------------
		// Private Member Variables
		//----------------------------------------------

		std::vector<int> m_nodeIndices;
		GmodPath m_path;
		bool m_isBuilt;
	};

	//=====================================================================
	// GmodParsePathResult Class
	//=====================================================================

	class GmodParsePathResult
	{
		//----------------------------------------------
		// Construction / Destruction
		//----------------------------------------------
	protected:
		GmodParsePathResult() = default;

		GmodParsePathResult( const GmodParsePathResult& ) = delete;
		GmodParsePathResult( GmodParsePathResult&& ) = delete;

	public:
		//----------------------------------------------
		// Assignment Operators
		//----------------------------------------------

	protected:
		GmodParsePathResult& operator=( const GmodParsePathResult& ) = delete;
		GmodParsePathResult& operator=( GmodParsePathResult&& ) = delete;

	public:
		virtual ~GmodParsePathResult() = default;

		class Ok;
		class Err;
	};

	class GmodParsePathResult::Ok final : public GmodParsePathResult
	{
	public:
		explicit Ok( GmodPath path );

		Ok( const Ok& ) = delete;
		Ok( Ok&& ) noexcept = delete;
		Ok& operator=( const Ok& ) = delete;
		Ok& operator=( Ok&& ) noexcept = delete;

		virtual ~Ok() = default;

	public:
		GmodPath path;
	};

	class GmodParsePathResult::Err final : public GmodParsePathResult
	{
	public:
		explicit Err( std::string errorString );

		Err( const Err& ) = delete;
		Err( Err&& ) noexcept = delete;
		Err& operator=( const Err& ) = delete;
		Err& operator=( Err&& ) noexcept = delete;

		virtual ~Err() = default;

	public:
		std::string error;
	};
}
