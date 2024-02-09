import * as fs from "fs-extra";
import readline from "readline";
import { VIS } from "../lib";

describe("MatchISOString", () => {
    const allAllowedCharacters =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";

    it.each(allAllowedCharacters.split("").concat(allAllowedCharacters))(
        "AllValidCharacters %s",
        (ch) => {
            const result = VIS.matchISOString(ch);
            expect(result).toBe(true);
        }
    );

    it.each([
        ["test", true],
        ["TeST", true],
        ["with space", false],
        ["#%/*", false],
    ])("Spot test %s", (input, expectedResult) => {
        const result = VIS.matchISOString(input);
        expect(result).toBe(expectedResult);
    });

    it("Smoke test", async () => {
        const fileStream = fs.createReadStream("../testdata/LocalIds.txt");
        const rl = readline.createInterface({
            input: fileStream,
            crlfDelay: Infinity,
        });

        let count = 0;
        let succeeded = 0;
        let errored: string[] = [];
        for await (let localIdStr of rl) {
            try {
                const match = VIS.matchISOLocalIdString(localIdStr);
                if (!match) {
                    errored.push(localIdStr);
                } else {
                    succeeded++;
                }
            } catch {
                errored.push(localIdStr);
            }
            count++;
        }

        expect(errored).toHaveLength(0);
        expect(count).toEqual(succeeded);
    });
});
