// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Aztec Labs.
pragma solidity >=0.8.27;

import {Test} from "forge-std/Test.sol";
import {TestERC20} from "src/mock/TestERC20.sol";
import {IERC20} from "@oz/token/ERC20/IERC20.sol";
import {IInbox} from "@aztec/core/interfaces/messagebridge/IInbox.sol";
import {InboxHarness} from "./harnesses/InboxHarness.sol";
import {Constants} from "@aztec/core/libraries/ConstantsGen.sol";
import {Errors} from "@aztec/core/libraries/Errors.sol";
import {Hash} from "@aztec/core/libraries/crypto/Hash.sol";
import {DataStructures} from "@aztec/core/libraries/DataStructures.sol";

contract InboxTest is Test {
  using Hash for DataStructures.L1ToL2Msg;

  uint256 internal constant FIRST_REAL_TREE_NUM = Constants.INITIAL_L2_BLOCK_NUM + 1;
  // We set low depth (5) to ensure we sufficiently test the tree transitions
  uint256 internal constant HEIGHT = 5;
  uint256 internal constant SIZE = 2 ** HEIGHT;

  InboxHarness internal inbox;
  uint256 internal version = 0;
  uint256 internal blockNumber = Constants.INITIAL_L2_BLOCK_NUM;
  bytes32 internal emptyTreeRoot;

  function setUp() public {
    address rollup = address(this);
    IERC20 feeAsset = new TestERC20("Fee Asset", "FA", address(this));
    inbox = new InboxHarness(rollup, feeAsset, version, HEIGHT);
    emptyTreeRoot = inbox.getEmptyRoot();
  }

  function _fakeMessage() internal view returns (DataStructures.L1ToL2Msg memory) {
    return DataStructures.L1ToL2Msg({
      sender: DataStructures.L1Actor({actor: address(this), chainId: block.chainid}),
      recipient: DataStructures.L2Actor({
        actor: 0x1000000000000000000000000000000000000000000000000000000000000000,
        version: version
      }),
      content: 0x2000000000000000000000000000000000000000000000000000000000000000,
      secretHash: 0x3000000000000000000000000000000000000000000000000000000000000000,
      index: 0x01
    });
  }

  function _divideAndRoundUp(uint256 a, uint256 b) internal pure returns (uint256) {
    return (a + b - 1) / b;
  }

  function _boundMessage(DataStructures.L1ToL2Msg memory _message, uint256 _globalLeafIndex)
    internal
    view
    returns (DataStructures.L1ToL2Msg memory)
  {
    // fix message.sender
    _message.sender = DataStructures.L1Actor({actor: address(this), chainId: block.chainid});
    // ensure actor fits in a field
    _message.recipient.actor = bytes32(uint256(_message.recipient.actor) % Constants.P);
    // ensure content fits in a field
    _message.content = bytes32(uint256(_message.content) % Constants.P);
    // ensure secret hash fits in a field
    _message.secretHash = bytes32(uint256(_message.secretHash) % Constants.P);
    // update version
    _message.recipient.version = version;
    // set leaf index
    _message.index = _globalLeafIndex;

    return _message;
  }

  // Since there is a 1 block lag between tree to be consumed and tree in progress the following invariant should never
  // be violated
  modifier checkInvariant() {
    _;
    assertLt(blockNumber, inbox.inProgress());
  }

  function testRevertIfNotConsumingFromRollup() public {
    vm.prank(address(0x1));
    vm.expectRevert(Errors.Inbox__Unauthorized.selector);
    inbox.consume(blockNumber);
  }

  function testRevertIFConsumingInFuture() public {
    vm.expectRevert(Errors.Inbox__MustBuildBeforeConsume.selector);
    inbox.consume(blockNumber + 1000);
  }

  function testFuzzInsert(DataStructures.L1ToL2Msg memory _message) public checkInvariant {
    uint256 globalLeafIndex = (FIRST_REAL_TREE_NUM - 1) * SIZE;
    DataStructures.L1ToL2Msg memory message = _boundMessage(_message, globalLeafIndex);

    bytes32 leaf = message.sha256ToField();
    vm.expectEmit(true, true, true, true);
    // event we expect
    emit IInbox.MessageSent(FIRST_REAL_TREE_NUM, globalLeafIndex, leaf);
    // event we will get
    (bytes32 insertedLeaf, uint256 insertedIndex) =
      inbox.sendL2Message(message.recipient, message.content, message.secretHash);

    assertEq(insertedLeaf, leaf);
    assertEq(insertedIndex, globalLeafIndex);
  }

  function testSendDuplicateL2Messages() public checkInvariant {
    DataStructures.L1ToL2Msg memory message = _fakeMessage();
    (bytes32 leaf1, uint256 index1) =
      inbox.sendL2Message(message.recipient, message.content, message.secretHash);
    (bytes32 leaf2, uint256 index2) =
      inbox.sendL2Message(message.recipient, message.content, message.secretHash);
    (bytes32 leaf3, uint256 index3) =
      inbox.sendL2Message(message.recipient, message.content, message.secretHash);

    // Only 1 tree should be non-zero
    assertEq(inbox.getNumTrees(), 1);

    // All the leaves should be different since the index gets mixed in
    assertNotEq(leaf1, leaf2);
    assertNotEq(leaf2, leaf3);

    // Check indices
    assertEq(index1 + 1, index2);
    assertEq(index1 + 2, index3);
  }

  function testRevertIfActorTooLarge() public {
    DataStructures.L1ToL2Msg memory message = _fakeMessage();
    message.recipient.actor = bytes32(Constants.P);
    vm.expectRevert(
      abi.encodeWithSelector(Errors.Inbox__ActorTooLarge.selector, message.recipient.actor)
    );
    inbox.sendL2Message(message.recipient, message.content, message.secretHash);
  }

  function testRevertIfVersionMismatch() public {
    DataStructures.L1ToL2Msg memory message = _fakeMessage();
    message.recipient.version = version + 1;
    vm.expectRevert(
      abi.encodeWithSelector(
        Errors.Inbox__VersionMismatch.selector, message.recipient.version, version
      )
    );
    inbox.sendL2Message(message.recipient, message.content, message.secretHash);
  }

  function testRevertIfContentTooLarge() public {
    DataStructures.L1ToL2Msg memory message = _fakeMessage();
    message.content = bytes32(Constants.P);
    vm.expectRevert(abi.encodeWithSelector(Errors.Inbox__ContentTooLarge.selector, message.content));
    inbox.sendL2Message(message.recipient, message.content, message.secretHash);
  }

  function testRevertIfSecretHashTooLarge() public {
    DataStructures.L1ToL2Msg memory message = _fakeMessage();
    message.secretHash = bytes32(Constants.P);
    vm.expectRevert(
      abi.encodeWithSelector(Errors.Inbox__SecretHashTooLarge.selector, message.secretHash)
    );
    inbox.sendL2Message(message.recipient, message.content, message.secretHash);
  }

  function testFuzzSendAndConsume(
    DataStructures.L1ToL2Msg[] memory _messagesFirstBatch,
    DataStructures.L1ToL2Msg[] memory _messagesSecondBatch,
    uint256 _numTreesToConsumeFirstBatch,
    uint256 _numTreesToConsumeSecondBatch
  ) public {
    // Send first batch of messages
    _send(_messagesFirstBatch);

    // Consume first few trees
    _consume(_numTreesToConsumeFirstBatch);

    // Send second batch of messages
    _send(_messagesSecondBatch);

    // Consume second batch of trees
    _consume(_numTreesToConsumeSecondBatch);
  }

  function _send(DataStructures.L1ToL2Msg[] memory _messages) internal checkInvariant {
    bytes32 toConsumeRoot = inbox.getToConsumeRoot(blockNumber);

    // We send the messages and then check that toConsume root did not change.
    for (uint256 i = 0; i < _messages.length; i++) {
      DataStructures.L1ToL2Msg memory message =
        _boundMessage(_messages[i], inbox.getNextMessageIndex());

      // We check whether a new tree is correctly initialized when the one in progress is full
      uint256 numTrees = inbox.getNumTrees();
      uint256 expectedNumTrees = inbox.treeInProgressFull() ? numTrees + 1 : numTrees;

      inbox.sendL2Message(message.recipient, message.content, message.secretHash);

      assertEq(inbox.getNumTrees(), expectedNumTrees, "Unexpected number of trees");
    }

    // Root of a tree waiting to be consumed should not change because we introduced a 1 block lag to prevent sequencer
    // DOS attacks
    assertEq(
      inbox.getToConsumeRoot(blockNumber),
      toConsumeRoot,
      "Root of a tree waiting to be consumed should not change"
    );
  }

  function _consume(uint256 _numTreesToConsume) internal checkInvariant {
    uint256 initialNumTrees = inbox.getNumTrees();
    // We use (initialNumTrees * 2) as upper bound here because we want to test the case where we go beyond
    // the currently initalized number of trees. When consuming the newly initialized trees we should get zero roots.
    uint256 numTreesToConsume = bound(_numTreesToConsume, 1, initialNumTrees * 2);

    // Now we consume the trees
    for (uint256 i = 0; i < numTreesToConsume; i++) {
      uint256 numTrees = inbox.getNumTrees();
      uint256 expectedNumTrees = (blockNumber + 1 == inbox.inProgress()) ? numTrees + 1 : numTrees;
      bytes32 root = inbox.consume(blockNumber);

      // We check whether a new tree is correctly initialized when the one which was in progress was set as to consume
      assertEq(inbox.getNumTrees(), expectedNumTrees, "Unexpected number of trees");

      // If we go beyong the number of trees initialized before consuming we should get empty root
      if (i > initialNumTrees) {
        assertEq(root, emptyTreeRoot, "Root of a newly initialized tree not empty");
      }
      blockNumber += 1;
    }
  }
}
