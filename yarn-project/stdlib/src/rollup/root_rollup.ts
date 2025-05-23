import { BlockBlobPublicInputs } from '@aztec/blob-lib';
import { AZTEC_MAX_EPOCH_DURATION } from '@aztec/constants';
import { makeTuple } from '@aztec/foundation/array';
import { Fr } from '@aztec/foundation/fields';
import { bufferSchemaFor } from '@aztec/foundation/schemas';
import { BufferReader, type Tuple, serializeToBuffer, serializeToFields } from '@aztec/foundation/serialize';
import { bufferToHex, hexToBuffer } from '@aztec/foundation/string';
import type { FieldsOf } from '@aztec/foundation/types';

import { AppendOnlyTreeSnapshot } from '../trees/append_only_tree_snapshot.js';
import { FeeRecipient } from './block_root_or_block_merge_public_inputs.js';
import { PreviousRollupBlockData } from './previous_rollup_block_data.js';

/**
 * Represents inputs of the root rollup circuit.
 */
export class RootRollupInputs {
  constructor(
    /**
     * The previous rollup data.
     * Note: Root rollup circuit is the latest circuit the chain of circuits and the previous rollup data is the data
     * from 2 block merge circuits.
     */
    public previousRollupData: [PreviousRollupBlockData, PreviousRollupBlockData],
    /** Identifier of the prover for this root rollup. */
    public proverId: Fr,
  ) {}

  /**
   * Serializes the inputs to a buffer.
   * @returns - The inputs serialized to a buffer.
   */
  toBuffer() {
    return serializeToBuffer(...RootRollupInputs.getFields(this));
  }

  /**
   * Serializes the inputs to a hex string.
   * @returns The instance serialized to a hex string.
   */
  toString() {
    return bufferToHex(this.toBuffer());
  }

  /**
   * Creates a new instance from fields.
   * @param fields - Fields to create the instance from.
   * @returns A new RootRollupInputs instance.
   */
  static from(fields: FieldsOf<RootRollupInputs>): RootRollupInputs {
    return new RootRollupInputs(...RootRollupInputs.getFields(fields));
  }

  /**
   * Extracts fields from an instance.
   * @param fields - Fields to create the instance from.
   * @returns An array of fields.
   */
  static getFields(fields: FieldsOf<RootRollupInputs>) {
    return [fields.previousRollupData, fields.proverId] as const;
  }

  /**
   * Deserializes the inputs from a buffer.
   * @param buffer - A buffer to deserialize from.
   * @returns A new RootRollupInputs instance.
   */
  static fromBuffer(buffer: Buffer | BufferReader): RootRollupInputs {
    const reader = BufferReader.asReader(buffer);
    return new RootRollupInputs(
      [reader.readObject(PreviousRollupBlockData), reader.readObject(PreviousRollupBlockData)],
      Fr.fromBuffer(reader),
    );
  }

  /**
   * Deserializes the inputs from a hex string.
   * @param str - A hex string to deserialize from.
   * @returns A new RootRollupInputs instance.
   */
  static fromString(str: string) {
    return RootRollupInputs.fromBuffer(hexToBuffer(str));
  }

  /** Returns a representation for JSON serialization. */
  toJSON() {
    return this.toBuffer();
  }

  /** Creates an instance from a string. */
  static get schema() {
    return bufferSchemaFor(RootRollupInputs);
  }
}

/**
 * Represents public inputs of the root rollup circuit.
 *
 * NOTE: in practice, we'll hash all of this up into a single public input, for cheap on-chain verification.
 */
export class RootRollupPublicInputs {
  constructor(
    /** Snapshot of archive tree before/after this rollup been processed */
    public previousArchive: AppendOnlyTreeSnapshot,
    public endArchive: AppendOnlyTreeSnapshot,
    // This is a u64 in nr, but GlobalVariables contains this as a u64 and is mapped to ts as a field, so I'm doing the same here
    public endTimestamp: Fr,
    public endBlockNumber: Fr,
    public outHash: Fr,
    public fees: Tuple<FeeRecipient, typeof AZTEC_MAX_EPOCH_DURATION>,
    public vkTreeRoot: Fr,
    public protocolContractTreeRoot: Fr,
    public proverId: Fr,
    public blobPublicInputs: Tuple<BlockBlobPublicInputs, typeof AZTEC_MAX_EPOCH_DURATION>,
  ) {}

  static getFields(fields: FieldsOf<RootRollupPublicInputs>) {
    return [
      fields.previousArchive,
      fields.endArchive,
      fields.endTimestamp,
      fields.endBlockNumber,
      fields.outHash,
      fields.fees,
      fields.vkTreeRoot,
      fields.protocolContractTreeRoot,
      fields.proverId,
      fields.blobPublicInputs,
    ] as const;
  }

  toBuffer() {
    return serializeToBuffer(...RootRollupPublicInputs.getFields(this));
  }

  toFields(): Fr[] {
    return serializeToFields(...RootRollupPublicInputs.getFields(this));
  }

  static from(fields: FieldsOf<RootRollupPublicInputs>): RootRollupPublicInputs {
    return new RootRollupPublicInputs(...RootRollupPublicInputs.getFields(fields));
  }

  /**
   * Deserializes a buffer into a `RootRollupPublicInputs` object.
   * @param buffer - The buffer to deserialize.
   * @returns The deserialized `RootRollupPublicInputs` object.
   */
  public static fromBuffer(buffer: Buffer | BufferReader): RootRollupPublicInputs {
    const reader = BufferReader.asReader(buffer);
    return new RootRollupPublicInputs(
      reader.readObject(AppendOnlyTreeSnapshot),
      reader.readObject(AppendOnlyTreeSnapshot),
      Fr.fromBuffer(reader),
      Fr.fromBuffer(reader),
      Fr.fromBuffer(reader),
      reader.readArray(AZTEC_MAX_EPOCH_DURATION, FeeRecipient),
      Fr.fromBuffer(reader),
      Fr.fromBuffer(reader),
      Fr.fromBuffer(reader),
      reader.readArray(AZTEC_MAX_EPOCH_DURATION, BlockBlobPublicInputs),
    );
  }

  toString() {
    return bufferToHex(this.toBuffer());
  }

  static fromString(str: string) {
    return RootRollupPublicInputs.fromBuffer(hexToBuffer(str));
  }

  /** Returns a representation for JSON serialization. */
  toJSON() {
    return this.toBuffer();
  }

  /** Creates an instance from a string. */
  static get schema() {
    return bufferSchemaFor(RootRollupPublicInputs);
  }

  /** Creates a random instance. */
  static random() {
    return new RootRollupPublicInputs(
      AppendOnlyTreeSnapshot.random(),
      AppendOnlyTreeSnapshot.random(),
      Fr.random(),
      Fr.random(),
      Fr.random(),
      makeTuple(AZTEC_MAX_EPOCH_DURATION, FeeRecipient.random),
      Fr.random(),
      Fr.random(),
      Fr.random(),
      makeTuple(AZTEC_MAX_EPOCH_DURATION, BlockBlobPublicInputs.empty),
    );
  }
}
