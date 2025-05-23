---
title: CLI Wallet
tags: [sandbox, wallet, cli]
keywords: [wallet, cli wallet]
sidebar_position: 3
---

For development, it may be useful to deploy, transact, or create notes in a non-programmatic way. You can use Aztec's CLI Wallet for thing such as:

- Deploying contracts
- Sending transactions
- Bridging L1 "Fee Juice" into Aztec
- Pushing arbitrary [notes](../../guides/smart_contracts/writing_contracts/notes/index.md) to your PXE
- Creating [authwits](../../guides/smart_contracts/writing_contracts/authwit.md)
- Aliasing info and secrets for further usage
- Proving your transactions and profile gate counts

:::info

At any time, you can get an updated version of the existing commands and subcommands by adding `-h`. For example:

```bash
aztec-wallet create-account -h
```

:::

## Aliases

The CLI wallet makes extensive use of aliases, that is, when an address, artifact, secret, or other information is given a name that can be later used to reference it.

Aliases have different types like `address` or `artifact` or `contract`. You can see a list of these types by running the help command `aztec-wallet alias -h`. You can then specify a type with the `:` character whenever needed. For example `accounts:master_yoda` or `artifacts:light_saber`.

:::tip

The wallet writes to the `last` alias if it's likely that you use that same alias in the next command.

It will also try to determine which type is expected. For example, if the alias `master_yoda` is an account, you don't need to prepend `account:` if, for example, you're deploying a contract.

You can create arbitrary aliases with the `alias` command. For example `aztec-wallet alias accounts test_alias 0x2c37902cdade7710bd2355e5949416dc5e43a16e0b13a5560854d2451d92d289`.

:::

## Paying Fees

import { Why_Fees, CLI_Fees } from '@site/src/components/Snippets/general_snippets';

<Why_Fees />

Below are all the payment methods available to pay transaction fees on Aztec, starting with the simplest.

### Fee Paying Contract

Fee paying contracts specify their own criteria of payment in exchange for paying the fee juice of a transaction, e.g. an FPC
be written to accept some banana tokens to pay for another's transaction fee.

Before using a fee paying contract, you need to register it in the PXE, passing the address of the contract and specifying the `from` account (in this case `main`). For example:

```bash
aztec-wallet register-contract $FPC_ADDRESS FPCContract -f main
```

With an alias corresponding to the FPC's address (`bananaFPC`) this would be:

```bash
aztec-wallet <your transaction> --payment method=fpc,fpc-contract=contracts:bananaFPC
```

### Sponsored Fee Paying Contract

Before using a Sponsored Fee Paying Contract (FPC), you need to register it in the PXE, passing the address of the contract and specifying the `from` account (in this case `main`). For example:

```bash
aztec-wallet register-contract $FPC_ADDRESS SponsoredFPC -f main
```

This is a special type of FPC that can be used to pay for account deployment and regular txs.
Eg: to create an account paid for by the sponsoredFPC:

```bash
aztec-wallet create-account -a main --payment method=fpc-sponsored,fpc=$FPC_ADDRESS
```

:::note
In the sandbox, the sponsored FPC address is printed at the end of its initial logs.
:::

### Fee Juice from Sandbox Test accounts

In the sandbox pre-loaded test accounts can be used to cover fee juice when deploying contracts.

First import them:

```bash title="import-test-accounts" showLineNumbers
aztec-wallet import-test-accounts
```
> <sup><sub><a href="https://github.com/AztecProtocol/aztec-packages/blob/v0.85.0-alpha-testnet.0/yarn-project/cli-wallet/test/flows/basic.sh#L9-L11" target="_blank" rel="noopener noreferrer">Source code: yarn-project/cli-wallet/test/flows/basic.sh#L9-L11</a></sub></sup>


Then use the alias (test0, test1...) when paying in fee juice. Eg to create accounts:

```bash title="declare-accounts" showLineNumbers
aztec-wallet create-account -a alice --payment method=fee_juice,feePayer=test0
aztec-wallet create-account -a bob --payment method=fee_juice,feePayer=test0
```
> <sup><sub><a href="https://github.com/AztecProtocol/aztec-packages/blob/v0.85.0-alpha-testnet.0/yarn-project/end-to-end/src/guides/up_quick_start.sh#L21-L24" target="_blank" rel="noopener noreferrer">Source code: yarn-project/end-to-end/src/guides/up_quick_start.sh#L21-L24</a></sub></sup>


### Mint and Bridge Fee Juice

#### On Sandbox

First register an account, mint the fee asset on L1 and bridge it to fee juice:

```bash title="bridge-fee-juice" showLineNumbers
aztec-wallet create-account -a main --register-only
aztec-wallet bridge-fee-juice 1000000000000000000 main --mint --no-wait
```
> <sup><sub><a href="https://github.com/AztecProtocol/aztec-packages/blob/v0.85.0-alpha-testnet.0/yarn-project/cli-wallet/test/flows/create_account_pay_native.sh#L8-L11" target="_blank" rel="noopener noreferrer">Source code: yarn-project/cli-wallet/test/flows/create_account_pay_native.sh#L8-L11</a></sub></sup>


You'll have to wait for two blocks to pass for bridged fee juice to be ready on Aztec.
For the sandbox you do this by putting through two arbitrary transactions. Eg:

```bash title="force-two-blocks" showLineNumbers
aztec-wallet deploy counter_contract@Counter --init initialize --args 0 accounts:test0 -f test0 -a counter
aztec-wallet send increment -ca counter --args accounts:test0 accounts:test0 -f test0
```
> <sup><sub><a href="https://github.com/AztecProtocol/aztec-packages/blob/v0.85.0-alpha-testnet.0/yarn-project/cli-wallet/test/flows/create_account_pay_native.sh#L17-L20" target="_blank" rel="noopener noreferrer">Source code: yarn-project/cli-wallet/test/flows/create_account_pay_native.sh#L17-L20</a></sub></sup>


Now the funded account can deploy itself with the bridged fees, claiming the bridged fee juice and deploying the contract in one transaction:

```bash title="claim-deploy-account" showLineNumbers
aztec-wallet deploy-account -f main --payment method=fee_juice,claim
```
> <sup><sub><a href="https://github.com/AztecProtocol/aztec-packages/blob/v0.85.0-alpha-testnet.0/yarn-project/cli-wallet/test/flows/create_account_pay_native.sh#L25-L27" target="_blank" rel="noopener noreferrer">Source code: yarn-project/cli-wallet/test/flows/create_account_pay_native.sh#L25-L27</a></sub></sup>


#### Minting on Testnet

This will mint the max amount of fee juice on L1 and bridge it to L2.

```bash
aztec-wallet bridge-fee-juice <AztecAddress>
```

## Account Management

The wallet comes with some options for account deployment and management. You can register and deploy accounts, or only register them, and pass different options to serve your workflow.

### Create Account

Generates a secret key and deploys an account contract.

#### Example

```bash
aztec-wallet create-account -a master_yoda
```

### Deploy account

Deploy an account that is already registered (i.e. your PXE knows about it) but not deployed. Most times you should pass an alias or address registered in the PXE by passing the `-f` or `--from` flag.

#### Example

```bash
$ aztec-wallet create-account --register-only -a master_yoda
...
$ aztec-wallet deploy-account -f master_yoda
```

### Deploy

You can deploy a [compiled contract](../../guides/smart_contracts/how_to_compile_contract.md) to the network.

You probably want to look at flags such as `--init` which allows you to specify the [initializer function](../../guides/smart_contracts/writing_contracts/initializers.md) to call, or `-k` for the [encryption public key](../../../aztec/concepts/accounts/keys.md#incoming-viewing-keys) if the contract is expected to have notes being encrypted to it.

You can pass arguments with the `--arg` flag.

#### Example

This example compiles the Jedi Code and deploys it from Master Yoda's account, initializing it with the parameter "Grand Master" and aliasing it to `jedi_order`. Notice how we can simply pass `master_yoda` in the `--from` flag (because `--from` always expects an account):

```bash
aztec-nargo compile
aztec-wallet deploy ./target/jedi_code.nr --arg accounts:master_yoda --from master_yoda --alias jedi_order
```

### Send

This command sends a transaction to the network by calling a contract's function. Just calling `aztec-wallet send` gives you a list of options, but you probably want to pass `--from` as the sender, `--contract-address` for your target's address, and `--args` if it requires arguments.

#### Example

```bash
aztec-wallet send --from master_yoda --contract-address jedi_order --args "luke skywalker" train_jedi
```

Again, notice how it's not necessary to pass `contracts:jedi_order` as the wallet already knows that the only available type for `--contract-address` is a contract.

### Manage authwits

You can use the CLI wallet to quickly generate [Authentication Witnesses](../../guides/smart_contracts/writing_contracts/authwit.md). These allow you to authorize the caller to execute an action on behalf of an account. They get aliased into the `authwits` type.

### In private

The authwit management in private is a two-step process: create and add. It's not too different from a `send` command, but providing the caller that can privately execute the action on behalf of the caller.

#### Example

An example for authorizing an operator (ex. a DeFi protocol) to call the transfer_in_private action (transfer on the user's behalf):

```bash
aztec-wallet create-authwit transfer_in_private accounts:coruscant_trader -ca contracts:token --args accounts:jedi_master accounts:coruscant_trader 20 secrets:auth_nonce -f accounts:jedi_master -a secret_trade

aztec-wallet add-authwit authwits:secret_trade accounts:jedi_master -f accounts:coruscant_trader
```

### In public

A similar call to the above, but in public:

```bash
aztec-wallet authorize-action transfer_in_public accounts:coruscant_trader -ca contracts:token --args accounts:jedi_master accounts:coruscant_trader 20 secrets:auth_nonce -f accounts:jedi_master
```

### Simulate

Simulates a transaction instead of sending it. This allows you to obtain i.e. the return value before sending the transaction.

#### Example

```bash
aztec-wallet simulate --from master_yoda --contract-address jedi_order --args "luke_skywalker" train_jedi
```

### Profile

This allows you to get the gate count of each private function in the transaction. Read more about profiling [here](../../guides/smart_contracts/profiling_transactions.md).

#### Example

```bash
aztec-wallet profile --from master_yoda --contract-address jedi_order --args "luke_skywalker" train_jedi
```

### Bridge Fee Juice

The wallet provides an easy way to mint the fee-paying asset on L1 and bridging it to L2. Current placeholder-name "fee juice".

Using the sandbox, there's already a Fee Juice contract that manages this enshrined asset. You can optionally mint more Juice before bridging it.

#### Example

This example mints and bridges 1000 units of fee juice and bridges it to the `master_yoda` recipient on L2.

```bash
aztec-wallet bridge-fee-juice --mint 1000 master_yoda
```

## Proving

You can prove a transaction using the aztec-wallet with a running sandbox. Follow the guide [here](../../guides/local_env/sandbox_proving.md#proving-with-aztec-wallet)
