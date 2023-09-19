using Vista.SDK;

namespace Vista.SDK.Tests;

public class GmodPathTests
{
    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddValidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_GmodPath_Parse(string inputPath)
    {
        var (_, vis) = VISTests.GetVis();

        var parsed = GmodPath.TryParse(inputPath, VisVersion.v3_4a, out var path);
        Assert.True(parsed);
        Assert.NotNull(path);
        Assert.Equal(inputPath, path?.ToString());
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddInvalidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_GmodPath_Parse_Invalid(string inputPath)
    {
        var (_, vis) = VISTests.GetVis();

        var parsed = GmodPath.TryParse(inputPath, VisVersion.v3_4a, out var path);
        Assert.False(parsed);
        Assert.Null(path);
    }

    [Fact]
    public void Test_GetFullPath()
    {
        var (_, vis) = VISTests.GetVis();

        var pathStr = "411.1/C101.72/I101";
        var expectation = new Dictionary<int, string>
        {
            [0] = "VE",
            [1] = "400a",
            [2] = "410",
            [3] = "411",
            [4] = "411i",
            [5] = "411.1",
            [6] = "CS1",
            [7] = "C101",
            [8] = "C101.7",
            [9] = "C101.72",
            [10] = "I101",
        };

        var seen = new HashSet<int>();
        foreach (var (depth, node) in GmodPath.Parse(pathStr, VisVersion.v3_4a).GetFullPath())
        {
            if (!seen.Add(depth))
                Assert.True(false, "Got same depth twice");
            if (seen.Count == 1)
                Assert.Equal(0, depth);
            Assert.Equal(expectation[depth], node.Code);
        }

        Assert.True(expectation.Keys.OrderBy(x => x).SequenceEqual(seen.OrderBy(x => x)));
    }

    [Fact]
    public void Test_GetFullPathFrom()
    {
        var (_, vis) = VISTests.GetVis();

        var pathStr = "411.1/C101.72/I101";
        var expectation = new Dictionary<int, string>
        {
            [4] = "411i",
            [5] = "411.1",
            [6] = "CS1",
            [7] = "C101",
            [8] = "C101.7",
            [9] = "C101.72",
            [10] = "I101",
        };

        var seen = new HashSet<int>();
        var path = GmodPath.Parse(pathStr, VisVersion.v3_4a);
        foreach (var (depth, node) in path.GetFullPathFrom(4))
        {
            if (!seen.Add(depth))
                Assert.True(false, "Got same depth twice");
            if (seen.Count == 1)
                Assert.Equal(4, depth);
            Assert.Equal(expectation[depth], node.Code);
        }

        Assert.True(expectation.Keys.OrderBy(x => x).SequenceEqual(seen.OrderBy(x => x)));
    }

    [Theory]
    [InlineData("411.1/C101.72/I101", "VE/400a/410/411/411i/411.1/CS1/C101/C101.7/C101.72/I101")]
    [InlineData("612.21-1/C701.13/S93", "VE/600a/610/612/612.2/612.2i-1/612.21-1/CS10/C701/C701.1/C701.13/S93")]
    public void Test_FullPathParsing(string shortPathStr, string expectedFullPathStr)
    {
        var version = VisVersion.v3_4a;

        var path = GmodPath.Parse(shortPathStr, version);
        var fullString = path.ToFullPathString();
        Assert.Equal(expectedFullPathStr, fullString);

        var parsed = GmodPath.TryParseFullPath(fullString, version, out var parsedPath);
        Assert.NotNull(parsedPath);
        Assert.True(parsed);
        Assert.StrictEqual(path, parsedPath);
        Assert.Equal(fullString, path.ToFullPathString());
        Assert.Equal(fullString, parsedPath.ToFullPathString());
        Assert.Equal(shortPathStr, path.ToString());
        Assert.Equal(shortPathStr, parsedPath.ToString());

        parsedPath = GmodPath.ParseFullPath(fullString, version);
        Assert.NotNull(parsedPath);
        Assert.True(parsed);
        Assert.StrictEqual(path, parsedPath);
        Assert.Equal(fullString, path.ToFullPathString());
        Assert.Equal(fullString, parsedPath.ToFullPathString());
        Assert.Equal(shortPathStr, path.ToString());
        Assert.Equal(shortPathStr, parsedPath.ToString());
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddIndividualizableSetsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_IndividualizableSets(string inputPath, string[][] expected)
    {
        var version = VisVersion.v3_4a;
        var gmod = VIS.Instance.GetGmod(version);

        var path = gmod.ParsePath(inputPath);
        var sets = path.IndividualizableSets;
        Assert.Equal(expected.Length, sets.Count);
        for (int i = 0; i < expected.Length; i++)
        {
            Assert.Equal(expected[i], sets[i].Nodes.Select(n => n.Code));
        }
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddIndividualizableSetsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_IndividualizableSets_FullPath(string inputPath, string[][] expected)
    {
        var version = VisVersion.v3_4a;
        var gmod = VIS.Instance.GetGmod(version);

        var path = GmodPath.ParseFullPath(gmod.ParsePath(inputPath).ToFullPathString(), version);
        var sets = path.IndividualizableSets;
        Assert.Equal(expected.Length, sets.Count);
        for (int i = 0; i < expected.Length; i++)
        {
            Assert.Equal(expected[i], sets[i].Nodes.Select(n => n.Code));
        }
    }

    [Fact]
    public void Test_GmodPath_Individualizes()
    {
        var version = VisVersion.v3_7a;
        var gmod = VIS.Instance.GetGmod(version);
        var path = gmod.ParsePath("411.1/C101.62/S205");
        var sets = path.IndividualizableSets;
        Assert.Equal(2, sets.Count);
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddValidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Valid_GmodPath_IndividualizableSets(string inputPath)
    {
        var version = VisVersion.v3_4a;
        var gmod = VIS.Instance.GetGmod(version);

        var path = gmod.ParsePath(inputPath);
        var sets = path.IndividualizableSets;

        var uniqueCodes = new HashSet<string>();
        foreach (var set in sets)
        {
            foreach (var node in set.Nodes)
                Assert.True(uniqueCodes.Add(node.Code));
        }
    }

    [Theory]
    [MemberData(nameof(VistaSDKTestData.AddValidGmodPathsData), MemberType = typeof(VistaSDKTestData))]
    public void Test_Valid_GmodPath_IndividualizableSets_FullPath(string inputPath)
    {
        var version = VisVersion.v3_4a;
        var gmod = VIS.Instance.GetGmod(version);

        var path = GmodPath.ParseFullPath(gmod.ParsePath(inputPath).ToFullPathString(), version);
        var sets = path.IndividualizableSets;

        var uniqueCodes = new HashSet<string>();
        foreach (var set in sets)
        {
            foreach (var node in set.Nodes)
                Assert.True(uniqueCodes.Add(node.Code));
        }
    }
}
