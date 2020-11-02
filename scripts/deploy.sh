#!/bin/bash

eosio-cpp newdex.cpp -I ../

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# contract
cleos set contract miner.sx . newdex.wasm newdex.abi
