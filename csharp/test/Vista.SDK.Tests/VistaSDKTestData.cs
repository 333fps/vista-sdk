using System.Text.Json;
using System.Text.Json.Serialization;

namespace Vista.SDK.Tests;

public class VistaSDKTestData
{
    private static JsonSerializerOptions options =
        new() { ReadCommentHandling = JsonCommentHandling.Skip, AllowTrailingCommas = true, };

    public static IEnumerable<object[]> AddValidPositionData() => AddCodebookData(CodebookTestData.ValidPosition);

    public static IEnumerable<object[]> AddStatesData() => AddCodebookData(CodebookTestData.States);

    public static IEnumerable<object[]> AddPositionsData() => AddCodebookData(CodebookTestData.Positions);

    public static IEnumerable<object[]> AddTagData() => AddCodebookData(CodebookTestData.Tag);

    public static IEnumerable<object[]> AddDetailTagData() => AddCodebookData(CodebookTestData.DetailTag);

    public static IEnumerable<object[]> AddInvalidLocalIdsData() => AddInvalidLocalId(LocalIdTestData);

    public static IEnumerable<object[]> AddValidGmodPathsData() => AddValidGmodPathsData(GmodPathTestData);

    public static IEnumerable<object[]> AddInvalidGmodPathsData() => AddInvalidGmodPathsData(GmodPathTestData);

    public static IEnumerable<object?[]> AddLocationsData() => AddLocationsData(LocationsTestData);

    public static IEnumerable<object?[]> AddIndividualizableSetsData() =>
        AddIndividualizableSetsData(IndividualizableSetsData);

    public static IEnumerable<object?[]> AddVersioningData() => AddVersioningData(VersioningTestData);

    private static CodebookTestData CodebookTestData => GetData<CodebookTestData>("Codebook");
    private static LocalIdTestData LocalIdTestData => GetData<LocalIdTestData>("InvalidLocalIds");
    private static GmodPathTestData GmodPathTestData => GetData<GmodPathTestData>("GmodPaths");
    private static LocationsTestData LocationsTestData => GetData<LocationsTestData>("Locations");
    private static IndividualizableSetData[] IndividualizableSetsData =>
        GetData<IndividualizableSetData[]>("IndividualizableSets");
    private static Dictionary<string, List<VersioningFullPathTestItem>> VersioningTestData =>
        GetData<Dictionary<string, List<VersioningFullPathTestItem>>>("VersioningTestCases");

    private static T GetData<T>(string testName)
    {
        var path = $"testdata/{testName}.json";
        var testDataJson = File.ReadAllText(path);

        return JsonSerializer.Deserialize<T>(testDataJson, options)
            ?? throw new Exception("Couldn't deserialize: " + typeof(T).Name);
    }

    public static IEnumerable<object[]> AddCodebookData(string[][] data)
    {
        foreach (var state in data)
        {
            yield return state;
        }
    }

    public static IEnumerable<object[]> AddInvalidLocalId(LocalIdTestData data)
    {
        foreach (var invalidLocalIdItem in data.InvalidLocalIds)
        {
            yield return new object[] { invalidLocalIdItem.localIdStr, invalidLocalIdItem.ExpectedErrormessages };
        }
    }

    public static IEnumerable<object[]> AddValidGmodPathsData(GmodPathTestData data)
    {
        foreach (var inputPath in data.Valid)
        {
            yield return new object[] { inputPath };
        }
    }

    public static IEnumerable<object[]> AddInvalidGmodPathsData(GmodPathTestData data)
    {
        foreach (var inputPath in data.Invalid)
        {
            yield return new object[] { inputPath };
        }
    }

    public static IEnumerable<object?[]> AddLocationsData(LocationsTestData data)
    {
        foreach (var location in data.Locations)
        {
            yield return new object?[]
            {
                location.Value,
                location.Success,
                location.Output,
                location.ExpectedErrorMessages
            };
        }
    }

    public static IEnumerable<object?[]> AddIndividualizableSetsData(IndividualizableSetData[] data)
    {
        foreach (var set in data)
        {
            yield return new object?[] { set.IsFullPath, set.VisVersion, set.Path, set.Expected, };
        }
    }

    public static IEnumerable<object?[]> AddVersioningData(Dictionary<string, List<VersioningFullPathTestItem>> data)
    {
        foreach (var kvp in data)
        {
            foreach (var v in kvp.Value)
            {
                var sourceVersion = VisVersions.Parse(v.From);
                var targetVersion = VisVersions.Parse(v.To);
                var item = new VersioningTestItem(
                    sourceVersion,
                    targetVersion,
                    GmodPath.ParseFullPath(v.SourceFullPath, sourceVersion),
                    GmodPath.ParseFullPath(v.TargetFullPath, targetVersion),
                    kvp.Key
                );
                yield return new object?[] { item };
            }
        }
    }
}

public record InvalidLocalIds(
    [property: JsonPropertyName("input")] string localIdStr,
    [property: JsonPropertyName("expectedErrorMessages")] string[] ExpectedErrormessages
);

public record LocalIdTestData([property: JsonPropertyName("InvalidLocalIds")] InvalidLocalIds[] InvalidLocalIds);

public record GmodPathTestItem(
    [property: JsonPropertyName("path")] string Path,
    [property: JsonPropertyName("visVersion")] string VisVersion
);

public record GmodPathTestData(
    [property: JsonPropertyName("Valid")] GmodPathTestItem[] Valid,
    [property: JsonPropertyName("Invalid")] GmodPathTestItem[] Invalid
);

public record CodebookTestData(
    [property: JsonPropertyName("ValidPosition")] string[][] ValidPosition,
    [property: JsonPropertyName("Positions")] string[][] Positions,
    [property: JsonPropertyName("States")] string[][] States,
    [property: JsonPropertyName("Tag")] string[][] Tag,
    [property: JsonPropertyName("DetailTag")] string[][] DetailTag
);

public sealed record LocationsTestData([property: JsonPropertyName("locations")] LocationsTestDataItem[] Locations);

public sealed record class LocationsTestDataItem(
    [property: JsonPropertyName("value")] string Value,
    [property: JsonPropertyName("success")] bool Success,
    [property: JsonPropertyName("output")] string? Output,
    [property: JsonPropertyName("expectedErrorMessages")] string[] ExpectedErrorMessages
);

public sealed record IndividualizableSetData(
    [property: JsonPropertyName("isFullPath")] bool IsFullPath,
    [property: JsonPropertyName("visVersion")] string VisVersion,
    [property: JsonPropertyName("path")] string Path,
    [property: JsonPropertyName("expected")] string[][]? Expected
);

public record VersioningFullPathTestItem(
    [property: JsonPropertyName("from")] string From,
    [property: JsonPropertyName("to")] string To,
    [property: JsonPropertyName("sourceFullPath")] string SourceFullPath,
    [property: JsonPropertyName("targetFullPath")] string TargetFullPath
);

public sealed record VersioningTestItem(
    VisVersion SourceVersion,
    VisVersion TargetVersion,
    GmodPath SourcePath,
    GmodPath TargetPath,
    string ConversionCode
);
