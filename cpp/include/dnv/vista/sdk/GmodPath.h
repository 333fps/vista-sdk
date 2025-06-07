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
	// GmodPath class
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
		// Construction / destruction
		//----------------------------------------------

		GmodPath( const Gmod& gmod, GmodNode* node, std::vector<GmodNode*> parents = {} );

		GmodPath();
		GmodPath( const GmodPath& other );
		GmodPath( GmodPath&& other ) noexcept;

		~GmodPath();

		//----------------------------------------------
		// Assignment operators
		//----------------------------------------------

		GmodPath& operator=( const GmodPath& other );
		GmodPath& operator=( GmodPath&& other ) noexcept;

		//----------------------------------------------
		// Equality operators
		//----------------------------------------------

		[[nodiscard]] bool operator==( const GmodPath& other ) const noexcept;
		[[nodiscard]] bool operator!=( const GmodPath& other ) const noexcept;

		//----------------------------------------------
		// Lookup operators
		//----------------------------------------------

		[[nodiscard]] GmodNode* operator[]( size_t index ) const;
		[[nodiscard]] GmodNode*& operator[]( size_t index );

		//----------------------------------------------
		// Accessors
		//----------------------------------------------

		[[nodiscard]] VisVersion visVersion() const noexcept;
		[[nodiscard]] size_t hashCode() const noexcept;

		[[nodiscard]] const Gmod* gmod() const noexcept;
		[[nodiscard]] GmodNode* node() const noexcept;
		[[nodiscard]] const std::vector<GmodNode*>& parents() const noexcept;

		[[nodiscard]] size_t length() const noexcept;
		[[nodiscard]] std::vector<GmodIndividualizableSet> individualizableSets() const;

		[[nodiscard]] std::optional<std::string> normalAssignmentName( size_t nodeDepth ) const;
		[[nodiscard]] std::vector<std::pair<size_t, std::string>> commonNames() const;

		//----------------------------------------------
		// Utility methods
		//----------------------------------------------

		[[nodiscard]] static bool isValid( const std::vector<GmodNode*>& parents, const GmodNode& node );
		[[nodiscard]] static bool isValid( const std::vector<GmodNode*>& parents, const GmodNode& node, int& missingLinkAt );
		[[nodiscard]] bool isMappable() const;

		[[nodiscard]] std::string toString() const;
		[[nodiscard]] std::string toStringDump() const;
		[[nodiscard]] std::string toFullPathString() const;

		void toString( std::stringstream& builder, char separator = '/' ) const;
		void toStringDump( std::stringstream& builder ) const;
		void toFullPathString( std::stringstream& builder ) const;

		[[nodiscard]] GmodPath withoutLocations() const;

		//----------------------------------------------
		// Public static parsing methods
		//----------------------------------------------

		[[nodiscard]] static GmodPath parse( std::string_view item, VisVersion visVersion );
		[[nodiscard]] static GmodPath parse( std::string_view item, const Gmod& gmod, const Locations& locations );
		[[nodiscard]] static GmodPath parseFullPath( std::string_view item, VisVersion visVersion );

		[[nodiscard]] static bool tryParse( std::string_view item, VisVersion visVersion, std::optional<GmodPath>& outPath );
		[[nodiscard]] static bool tryParse( std::string_view item, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath );

		[[nodiscard]] static bool tryParseFullPath( std::string_view item, VisVersion visVersion, std::optional<GmodPath>& outPath );
		[[nodiscard]] static bool tryParseFullPath( std::string_view item, const Gmod& gmod, const Locations& locations, std::optional<GmodPath>& outPath );

		//----------------------------------------------
		// Enumeration
		//----------------------------------------------

		[[nodiscard]] Enumerator enumerator( size_t fromDepth = std::numeric_limits<size_t>::max() ) const;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		VisVersion m_visVersion;
		const Gmod* m_gmod;
		GmodNode* m_node;
		std::vector<GmodNode*> m_parents;
		std::vector<GmodNode*> m_ownedNodes;

	private:
		//----------------------------------------------
		// Private static parsing methods
		//----------------------------------------------

		static std::unique_ptr<GmodParsePathResult> parseInternal(
			std::string_view item, const Gmod& gmod, const Locations& locations );

		static std::unique_ptr<GmodParsePathResult> parseFullPathInternal(
			std::string_view item, const Gmod& gmod, const Locations& locations );

	public:
		//----------------------------------------------
		// GmodPath enumerator
		//----------------------------------------------

		class Enumerator final
		{
			friend class GmodPath;

		public:
			//----------------------------
			// Types definitions
			//----------------------------

			/** @brief Represents a single path element entry. */
			using PathElement = std::pair<size_t, GmodNode*>;

		private:
			//----------------------------
			// Construction / destruction
			//----------------------------

			/**
			 * @brief Internal constructor for creating Enumerator.
			 * @param pathInst Pointer to the GmodPath to enumerate.
			 * @param startIndex Starting index for enumeration.
			 */
			Enumerator( const GmodPath* pathInst, size_t startIndex = std::numeric_limits<size_t>::max() );

		public:
			/** @brief Default constructor */
			Enumerator() = delete;

			/** @brief Copy constructor */
			Enumerator( const Enumerator& ) = default;

			/** @brief Move constructor */
			Enumerator( Enumerator&& ) noexcept = default;

			/** @brief Destructor */
			~Enumerator() = default;

			//----------------------------
			// Assignment operators
			//----------------------------

			/** @brief Copy assignment operator */
			Enumerator& operator=( const Enumerator& ) = default;

			/** @brief Move assignment operator */
			Enumerator& operator=( Enumerator&& ) noexcept = default;

			//----------------------------
			// Enumeration methods
			//----------------------------

			/**
			 * @brief Advances the enumerator to the next element.
			 * @return True if the enumerator successfully moved to the next element;
			 *         false if the enumerator has passed the end of the collection.
			 */
			bool next();

			/**
			 * @brief Gets the current element.
			 * @return The current path element (depth, node).
			 */
			const PathElement& current() const;

			/**
			 * @brief Resets the enumerator to its initial position.
			 */
			void reset();

		private:
			//----------------------------
			// Private member variables
			//----------------------------

			const GmodPath* m_pathInstance;
			size_t m_currentIndex;
			mutable PathElement m_current;
		};
	};

	//=====================================================================
	// GmodIndividualizableSet class
	//=====================================================================

	class GmodIndividualizableSet final
	{
	public:
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------

		GmodIndividualizableSet( const std::vector<int>& nodeIndices, const GmodPath& sourcePath );

		GmodIndividualizableSet() = delete;
		GmodIndividualizableSet( const GmodIndividualizableSet& ) = delete;
		GmodIndividualizableSet( GmodIndividualizableSet&& ) noexcept = default;

		~GmodIndividualizableSet() = default;

		//----------------------------------------------
		// Assignment operators
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
		// Utility methods
		//----------------------------------------------

		void setLocation( const std::optional<Location>& location );
		[[nodiscard]] std::string toString() const;

	private:
		//----------------------------------------------
		// Private member variables
		//----------------------------------------------

		std::vector<int> m_nodeIndices;
		GmodPath m_path;
		bool m_isBuilt;
	};

	//=====================================================================
	// GmodParsePathResult class
	//=====================================================================

	class GmodParsePathResult
	{
		//----------------------------------------------
		// Construction / destruction
		//----------------------------------------------
	protected:
		GmodParsePathResult() = default;

		GmodParsePathResult( const GmodParsePathResult& ) = delete;
		GmodParsePathResult( GmodParsePathResult&& ) = delete;

	public:
		//----------------------------------------------
		// Assignment operators
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
