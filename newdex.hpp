#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <math.h>


namespace newdex {

    using namespace std;
    using namespace eosio;
    using eosio::asset;
    using eosio::symbol;
    using eosio::name;
    using eosio::singleton;
    using eosio::multi_index;
    using eosio::time_point_sec;

    // reference
    const name id = "newdex"_n;
    const name code = "newdexpublic"_n;
    const string description = "Newdex Converter";

    const static uint8_t INT_BUY_LIMIT    = 1;
    const static uint8_t INT_SELL_LIMIT   = 2;
    const static uint8_t INT_BUY_MARKET   = 3;
    const static uint8_t INT_SELL_MARKET  = 4;

    const static uint8_t MAX_ORDERS = 10;        // place MAX_ORDERS orders at a time only - to fit in one transaction

    /**
     * Custom Token struct
     */
    struct ndx_symbol {
        name contract;
        symbol sym;
    };

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

    typedef eosio::multi_index<"sellorder"_n, order,
        indexed_by< "byprice"_n, const_mem_fun<order, uint128_t, &order::get_price> >,
        indexed_by< "byname"_n, const_mem_fun<order, uint64_t, &order::get_name> >
    > sell_order_t;

    // contains NewDex exchange pairs
    struct [[eosio::table]] exchange_pair {
        uint64_t                    pair_id;
        uint8_t                     price_precision;
        uint8_t                     status;
        ndx_symbol                  base_symbol;
        ndx_symbol                  quote_symbol;
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

    // contains NewDex exchange pairs
    struct [[eosio::table]] currency {
        uint64_t        currency_id;
        string          chain;
        name            contract;
        symbol          sym;
        asset           balance;
        bool            is_quote_currency;
        uint8_t         in_fee;
        uint8_t         out_fee;
        uint64_t        ext1;
        uint64_t        ext2;
        string          extstr;

        uint64_t primary_key() const { return currency_id; }
    };
    typedef eosio::multi_index<"currency"_n, currency> currency_t;

    // contains NewDex exchange pairs
    struct [[eosio::table]] global_config {
        uint64_t            global_id;
        string              key;
        string              value;
        string              memo;

        uint64_t primary_key() const { return global_id; }
    };
    typedef eosio::multi_index<"globalconfig"_n, global_config> global_config_t;

    static uint8_t get_fee()
    {
        return 15;          //for some reason actual charged fee is 15 (not 20 as in config)
        // 2. status = 1
        // 3. taker_fee = 20
        // 4. maker_fee = 20
        global_config_t global_config( "newdexpublic"_n, "newdexpublic"_n.value );
        const string value = global_config.get(3, "NewdexLibrary: global config does not exists").value;
        return std::stoi(value);
    }

    static string get_pair_symbol(uint64_t pair_id){
        exchange_pair_t markets( "newdexpublic"_n, "newdexpublic"_n.value );
        auto row = markets.get(pair_id, "NewdexLibrary: no such pair");

        return row.pair_symbol;
    }

    static pair<asset, string> get_amount_out(uint64_t pair_id, asset in, symbol sym_out) {

        asset out{0, sym_out};
        string order = "sell-market";
        exchange_pair_t markets( "newdexpublic"_n, "newdexpublic"_n.value );
        auto row = markets.get(pair_id, "NewdexLibrary: no such pair");
        double fee_adj = static_cast<double>((10000 - get_fee())) / 10000;
        double price_adj = static_cast<double>(pow(10, sym_out.precision()))/pow(10, in.symbol.precision());
        int orders = MAX_ORDERS;

        if(row.base_symbol.sym == in.symbol) {

            check(row.quote_symbol.sym == sym_out, "NewdexLibrary: wrong pair_id");
            buy_order_t ordertable( "newdexpublic"_n, pair_id );
            auto index = ordertable.get_index<"byprice"_n>();

            for(auto rowit = index.rbegin(); rowit!=index.rend() && in.amount>0 && orders--; ++rowit){
                if(in.amount - rowit->remain_convert.amount >= 0)
                    out.amount += rowit->remain_quantity.amount * fee_adj;
                else
                    out.amount += in.amount * rowit->price * price_adj * fee_adj;

                in -= rowit->remain_convert;
                // eosio::print("\n", rowit->order_id, " ", rowit->remain_quantity, " : ", rowit->remain_convert, " price: ", rowit->price, " in: ", in, " out: ", out);
            }
        }
        else {

            check(row.base_symbol.sym == sym_out && row.quote_symbol.sym == in.symbol, "NewdexLibrary: wrong pair_id");
            sell_order_t ordertable( "newdexpublic"_n, pair_id );
            auto index = ordertable.get_index<"byprice"_n>();
            order = "buy-market";

            for(auto rowit = index.begin(); rowit!=index.end() && in.amount>0 && orders--; ++rowit){
                if(in.amount - rowit->remain_convert.amount >= 0)
                    out.amount += rowit->remain_quantity.amount * fee_adj;
                else
                    out.amount += in.amount / rowit->price * price_adj * fee_adj;

                in -= rowit->remain_convert;
                // eosio::print("\n", rowit->order_id, " ", rowit->remain_quantity, " : ", rowit->remain_convert, " price: ", rowit->price, " in: ", in, " out: ", out);
            }
        }
        // eosio::print("\n  in left: ", in);
        if(in.amount > 0) return { asset{0, sym_out}, order };   //if there are not enough orders out fulfill our order

        return { out, order };
    };
};
