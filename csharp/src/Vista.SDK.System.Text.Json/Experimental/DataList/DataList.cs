//----------------------
// <auto-generated>
//     Generated using the NJsonSchema v10.9.0.0 (Newtonsoft.Json v13.0.0.0) (http://NJsonSchema.org)
// </auto-generated>
//----------------------


#nullable enable


namespace Vista.SDK.Experimental.Transport.Json.DataList
{
    #pragma warning disable // Disable all warnings

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Package
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Package(DataList @dataList, Header @header)


        {

            this.Header = @header;

            this.DataList = @dataList;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Header")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public Header Header { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DataList")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public DataList DataList { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Header
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Header(string @assetId, string? @author, ConfigurationReference @dataListID, System.DateTimeOffset? @dateCreated, VersionInformation? @versionInformation)


        {

            this.AssetId = @assetId;

            this.DataListID = @dataListID;

            this.VersionInformation = @versionInformation;

            this.Author = @author;

            this.DateCreated = @dateCreated;

        }
        [System.Text.Json.Serialization.JsonPropertyName("AssetId")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string AssetId { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DataListID")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public ConfigurationReference DataListID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("VersionInformation")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public VersionInformation? VersionInformation { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Author")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? Author { get; }


        [System.Text.Json.Serialization.JsonPropertyName("DateCreated")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public System.DateTimeOffset? DateCreated { get; }



        private System.Collections.Generic.IDictionary<string, object>? _additionalProperties;

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties ?? (_additionalProperties = new System.Collections.Generic.Dictionary<string, object>()); }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class VersionInformation
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public VersionInformation(string @namingRule, string @namingSchemeVersion, string? @referenceURL)


        {

            this.NamingRule = @namingRule;

            this.NamingSchemeVersion = @namingSchemeVersion;

            this.ReferenceURL = @referenceURL;

        }
        [System.Text.Json.Serialization.JsonPropertyName("NamingRule")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string NamingRule { get; }


        [System.Text.Json.Serialization.JsonPropertyName("NamingSchemeVersion")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string NamingSchemeVersion { get; }


        [System.Text.Json.Serialization.JsonPropertyName("ReferenceURL")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? ReferenceURL { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataList
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataList(System.Collections.Generic.IReadOnlyList<Data> @data)


        {

            this.Data = @data;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Data")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public System.Collections.Generic.IReadOnlyList<Data> Data { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Data
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Data(DataID @dataID, Property @property)


        {

            this.DataID = @dataID;

            this.Property = @property;

        }
        [System.Text.Json.Serialization.JsonPropertyName("DataID")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public DataID DataID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Property")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public Property Property { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataID
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataID(string @localID, NameObject? @nameObject, string? @shortID)


        {

            this.LocalID = @localID;

            this.ShortID = @shortID;

            this.NameObject = @nameObject;

        }
        [System.Text.Json.Serialization.JsonPropertyName("LocalID")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string LocalID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("ShortID")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? ShortID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("NameObject")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public NameObject? NameObject { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class NameObject
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public NameObject(string @namingRule)


        {

            this.NamingRule = @namingRule;

        }
        [System.Text.Json.Serialization.JsonPropertyName("NamingRule")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string NamingRule { get; }



        private System.Collections.Generic.IDictionary<string, object>? _additionalProperties;

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties ?? (_additionalProperties = new System.Collections.Generic.Dictionary<string, object>()); }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Property
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Property(string? @alertPriority, DataType @dataType, Format @format, string? @name, string? @qualityCoding, Range? @range, string? @remarks, Unit? @unit)


        {

            this.DataType = @dataType;

            this.Format = @format;

            this.Range = @range;

            this.Unit = @unit;

            this.QualityCoding = @qualityCoding;

            this.AlertPriority = @alertPriority;

            this.Name = @name;

            this.Remarks = @remarks;

        }
        [System.Text.Json.Serialization.JsonPropertyName("DataType")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public DataType DataType { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Format")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public Format Format { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Range")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public Range? Range { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Unit")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public Unit? Unit { get; }


        [System.Text.Json.Serialization.JsonPropertyName("QualityCoding")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? QualityCoding { get; }


        [System.Text.Json.Serialization.JsonPropertyName("AlertPriority")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? AlertPriority { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Name")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? Name { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Remarks")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? Remarks { get; }



        private System.Collections.Generic.IDictionary<string, object>? _additionalProperties;

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties ?? (_additionalProperties = new System.Collections.Generic.Dictionary<string, object>()); }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataType
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataType(string? @calculationPeriod, string @type, string? @updateCycle)


        {

            this.Type = @type;

            this.UpdateCycle = @updateCycle;

            this.CalculationPeriod = @calculationPeriod;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Type")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string Type { get; }


        [System.Text.Json.Serialization.JsonPropertyName("UpdateCycle")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? UpdateCycle { get; }


        [System.Text.Json.Serialization.JsonPropertyName("CalculationPeriod")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? CalculationPeriod { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Format
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Format(Restriction? @restriction, string @type)


        {

            this.Type = @type;

            this.Restriction = @restriction;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Type")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string Type { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Restriction")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public Restriction? Restriction { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Range
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Range(string @high, string @low)


        {

            this.High = @high;

            this.Low = @low;

        }
        [System.Text.Json.Serialization.JsonPropertyName("High")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string High { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Low")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string Low { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Unit
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Unit(string? @quantityName, string @unitSymbol)


        {

            this.UnitSymbol = @unitSymbol;

            this.QuantityName = @quantityName;

        }
        [System.Text.Json.Serialization.JsonPropertyName("UnitSymbol")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string UnitSymbol { get; }


        [System.Text.Json.Serialization.JsonPropertyName("QuantityName")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? QuantityName { get; }



        private System.Collections.Generic.IDictionary<string, object>? _additionalProperties;

        [System.Text.Json.Serialization.JsonExtensionData]
        public System.Collections.Generic.IDictionary<string, object> AdditionalProperties
        {
            get { return _additionalProperties ?? (_additionalProperties = new System.Collections.Generic.Dictionary<string, object>()); }
            set { _additionalProperties = value; }
        }

    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class Restriction
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public Restriction(System.Collections.Generic.IReadOnlyList<string>? @enumeration, string? @fractionDigits, string? @length, string? @maxExclusive, string? @maxInclusive, string? @maxLength, string? @minExclusive, string? @minInclusive, string? @minLength, string? @pattern, string? @totalDigits, RestrictionWhiteSpace? @whiteSpace)


        {

            this.Enumeration = @enumeration;

            this.FractionDigits = @fractionDigits;

            this.Length = @length;

            this.MaxExclusive = @maxExclusive;

            this.MaxInclusive = @maxInclusive;

            this.MaxLength = @maxLength;

            this.MinExclusive = @minExclusive;

            this.MinInclusive = @minInclusive;

            this.MinLength = @minLength;

            this.Pattern = @pattern;

            this.TotalDigits = @totalDigits;

            this.WhiteSpace = @whiteSpace;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Enumeration")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public System.Collections.Generic.IReadOnlyList<string>? Enumeration { get; }


        [System.Text.Json.Serialization.JsonPropertyName("FractionDigits")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? FractionDigits { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Length")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? Length { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MaxExclusive")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? MaxExclusive { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MaxInclusive")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? MaxInclusive { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MaxLength")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? MaxLength { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MinExclusive")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? MinExclusive { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MinInclusive")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? MinInclusive { get; }


        [System.Text.Json.Serialization.JsonPropertyName("MinLength")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? MinLength { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Pattern")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? Pattern { get; }


        [System.Text.Json.Serialization.JsonPropertyName("TotalDigits")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? TotalDigits { get; }


        [System.Text.Json.Serialization.JsonPropertyName("WhiteSpace")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        [System.Text.Json.Serialization.JsonConverter(typeof(System.Text.Json.Serialization.JsonStringEnumConverter))]
        public RestrictionWhiteSpace? WhiteSpace { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class ConfigurationReference
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public ConfigurationReference(string @iD, System.DateTimeOffset @timeStamp, string? @version)


        {

            this.ID = @iD;

            this.Version = @version;

            this.TimeStamp = @timeStamp;

        }
        [System.Text.Json.Serialization.JsonPropertyName("ID")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public string ID { get; }


        [System.Text.Json.Serialization.JsonPropertyName("Version")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.WhenWritingDefault)]   
        public string? Version { get; }


        [System.Text.Json.Serialization.JsonPropertyName("TimeStamp")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public System.DateTimeOffset TimeStamp { get; }


    }

    /// <summary>
    /// An experimental generalized DataList package based on ISO19848
    /// </summary>
    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public partial class DataListPackage
    {
        [System.Text.Json.Serialization.JsonConstructor]

        public DataListPackage(Package @package)


        {

            this.Package = @package;

        }
        [System.Text.Json.Serialization.JsonPropertyName("Package")]

        [System.Text.Json.Serialization.JsonIgnore(Condition = System.Text.Json.Serialization.JsonIgnoreCondition.Never)]   
        public Package Package { get; }


    }

    [System.CodeDom.Compiler.GeneratedCode("NJsonSchema", "10.9.0.0 (Newtonsoft.Json v13.0.0.0)")]
    public enum RestrictionWhiteSpace
    {

        [System.Runtime.Serialization.EnumMember(Value = @"Preserve")]
        Preserve = 0,


        [System.Runtime.Serialization.EnumMember(Value = @"Replace")]
        Replace = 1,


        [System.Runtime.Serialization.EnumMember(Value = @"Collapse")]
        Collapse = 2,


    }
}