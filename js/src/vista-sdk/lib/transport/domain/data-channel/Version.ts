import { VisVersion } from "../../..";
import { isNullOrWhiteSpace } from "../../../util/util";

export class Version {
    private readonly _tag: number;
    private readonly _visVersion?: VisVersion;
    private readonly _otherVersion?: string;

    private constructor(version: VisVersion | string) {
        if (this.isVisVersion(version)) {
            this._tag = 1;
            this._visVersion = version;
        } else {
            this._tag = 2;
            this._otherVersion = version;
        }
    }

    public match<T>(
        onVisVersion: (version: VisVersion) => T,
        onOtherVersion: (version: string) => T
    ): T {
        switch (this._tag) {
            case 1:
                return onVisVersion(this._visVersion!);
            case 2:
                return onOtherVersion(this._otherVersion!);
            default:
                throw new Error("Tried to match on invalid Version");
        }
    }

    public switch(
        onVisVersion: (version: VisVersion) => void,
        onOtherVersion: (version: string) => void
    ) {
        switch (this._tag) {
            case 1:
                return onVisVersion(this._visVersion!);
            case 2:
                return onOtherVersion(this._otherVersion!);
            default:
                throw new Error("Tried to switch on invalid Version");
        }
    }

    public toString() {
        switch (this._tag) {
            case 1:
                return this._visVersion!.toString();
            case 2:
                return this._otherVersion!;
            default:
                throw new Error("Invalid Version");
        }
    }

    public static parse(version?: string): Version {
        if (isNullOrWhiteSpace(version))
            throw new Error(`${Version.name}.parse: value is null`);

        return new Version(version);
    }

    private isVisVersion(version: string): version is VisVersion {
        return Object.values(VisVersion).map(toString).includes(version);
    }
}
