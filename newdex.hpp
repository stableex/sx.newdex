#pragma once

#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/asset.hpp>

#include <string>

using namespace eosio;
using namespace std;

class [[eosio::contract("newdexpublic")]] newdexpublic : public contract {
public:
    using contract::contract;

    /**
     * Construct a new contract given the contract name
     *
     * @param {name} receiver - The name of this contract
     * @param {name} code - The code name of the action this contract is processing.
     * @param {datastream} ds - The datastream used
     */
    newdexpublic( name receiver, name code, eosio::datastream<const char*> ds )
        : contract( receiver, code, ds )
    {}

    [[eosio::action]]
    void set( const uint64_t pair_id, const string pair_symbol, const double current_price );

    const static uint8_t INT_BUY_LIMIT    = 1;
    const static uint8_t INT_SELL_LIMIT   = 2;
    const static uint8_t INT_BUY_MARKET   = 3;
    const static uint8_t INT_SELL_MARKET  = 4;

    // contains NewDex buy orders
    struct [[eosio::table]] order {
        uint64_t                order_id;
        uint64_t                pair_id;
        uint8_t                 type;
        eosio::name             owner;
        eosio::time_point_sec   placed_time;
        eosio::asset            remain_quantity;
        eosio::asset            remain_convert;
        double                  price;
        eosio::name             contract;
        uint8_t                 count;
        uint8_t                 crosschain;
        uint64_t                ext1;
        string                  extstr;

        uint64_t primary_key() const { return order_id; }
        uint128_t get_price() const {
            uint64_t max = 1000000000000;
            if ( type == INT_BUY_LIMIT || type  == INT_BUY_MARKET ) {
                uint128_t high = (uint128_t)(max * price);
                uint64_t low = max - placed_time.utc_seconds;
                uint128_t price128 = (high << 64) + low;
                return price128;
            } else {
                return (uint128_t)(max * price);
            }
        }
        uint64_t get_name() const { return owner.value; }
    };

    typedef eosio::multi_index<"buyorder"_n, order,
        indexed_by< "byprice"_n, const_mem_fun<order, uint128_t, &order::get_price> >,
        indexed_by< "byname"_n, const_mem_fun<order, uint64_t, &order::get_name> >
    > buy_order_t;

    // contains NewDex exchange pairs
    struct [[eosio::table]] exchange_pair {
        uint64_t                    pair_id;
        uint8_t                     price_precision;
        uint8_t                     status;
        eosio::extended_symbol      base_symbol;
        eosio::extended_symbol      quote_symbol;
        eosio::name                 manager;
        eosio::time_point_sec       list_time;
        string                      pair_symbol;
        double                      current_price;
        uint64_t                    base_currency_id;
        uint64_t                    quote_currency_id;
        uint8_t                     pair_fee;
        uint64_t                    ext1;
        uint64_t                    ext2;
        string                      extstr;

        uint64_t primary_key() const { return pair_id; }
    };

    typedef eosio::multi_index<"exchangepair"_n, exchange_pair> exchange_pair_t;
};