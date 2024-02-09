import { LocationBuilder, VIS, VisVersion, VisVersions } from "../lib";
import * as testData from "../../testdata/Locations.json";
import { LocationParsingErrorBuilder } from "../lib/internal/LocationParsingErrorBuilder";
import { getVIS, getVISMap } from "./Fixture";

describe("Location", () => {
    const version = VisVersion.v3_4a;

    beforeAll(() => {
        return getVISMap();
    });

    it.each(VisVersions.all)("Location loads for VIS version %s", (version) => {
        const location = getVIS(version).locations;

        expect(location).toBeTruthy();
        expect(location.groups).toBeTruthy();
    });

    it.each(testData.locations.map((l) => [l]))(
        "Location parsing - %s",
        ({ value, success, output, expectedErrorMessages }) => {
            const locations = getVIS(version).locations;

            const errorBuilder = new LocationParsingErrorBuilder();
            const createdLocation = locations.tryParse(value, errorBuilder);
            if (!success) {
                expect(createdLocation).toBeUndefined();
                if (expectedErrorMessages.length > 0) {
                    expect(errorBuilder).toBeDefined();
                    expect(errorBuilder.hasError).toBe(true);
                    const actualErrors = errorBuilder.errors.map(
                        (e) => e.message
                    );
                    expect(actualErrors).toEqual(expectedErrorMessages);
                }
            } else {
                expect(errorBuilder.hasError).toBe(false);
                expect(createdLocation).toBeTruthy();
                expect(createdLocation!.toString()).toBe(output);
            }
        }
    );

    it("Location parsing throws", () => {
        const locations = getVIS(version).locations;

        expect(() => locations.parse(null!)).toThrowError();
        expect(() => locations.parse(undefined!)).toThrowError();
    });

    it("Location builder", () => {
        const locations = getVIS(version).locations;

        const locationStr = "21FIPU";
        var location = locations.parse(locationStr);

        var builder = LocationBuilder.create(locations);

        builder = builder.withNumber(62);
        expect(builder.toString()).toEqual("62");

        builder = builder
            .withNumber(21)
            .withSide("P")
            .withTransverse("I")
            .withLongitudinal("F")
            .withValue("U");

        expect(builder.toString()).toEqual("21FIPU");
        expect(builder.number).toEqual(21);
        expect(builder.side).toEqual("P");
        expect(builder.vertical).toEqual("U");
        expect(builder.transverse).toEqual("I");
        expect(builder.longitudinal).toEqual("F");

        expect(() => (builder = builder.withValue("X"))).toThrowError();
        expect(() => (builder = builder.withNumber(-1))).toThrowError();
        expect(() => (builder = builder.withNumber(0))).toThrowError();
        expect(() => (builder = builder.withNumber(1.5))).toThrowError();
        expect(() => (builder = builder.withSide("A"))).toThrowError();
        expect(() => (builder = builder.withValue("a"))).toThrowError();

        expect(builder.build()).toEqual(location);

        builder = LocationBuilder.create(locations).withLocation(
            builder.build()
        );

        expect(builder.toString()).toEqual("21FIPU");
        expect(builder.number).toEqual(21);
        expect(builder.side).toEqual("P");
        expect(builder.vertical).toEqual("U");
        expect(builder.transverse).toEqual("I");
        expect(builder.longitudinal).toEqual("F");

        builder = builder.withValue("S").withValue(2);

        expect(builder.toString()).toEqual("2FISU");
        expect(builder.number).toEqual(2);
        expect(builder.side).toEqual("S");
        expect(builder.vertical).toEqual("U");
        expect(builder.transverse).toEqual("I");
        expect(builder.longitudinal).toEqual("F");
    });
});
