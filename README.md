# **`SX.NewDex`**

> C++ interface and data structures for interacting with NewDex smart contract `newdexpublic`


## Quickstart

```c++
#include <sx.newdex/newdex.hpp>

// user input
const asset quantity = asset{10000, symbol{"EOS", 4}};
const uint64_t mid = 470; // EOS/USN pair
const symbol out_sym = symbol{"USDT", 4};

// get newdex info
const auto [out, order] = newdex::get_amount_out( mid, quantity, out_sym );

// => [ "4.6500 USDT", "sell-market" ]
```

## Table of Content

- [STATIC `get_amount_out`](#static-get_amount_out)
- [STATIC `get_fee`](#static-get_fee)

## STATIC `get_amount_out`

Get amount to receive when market selling `quantity` to `out_sym`

### params

- `{uint64_t} mid` - market id
- `{asset} quantity` - quantity to convert
- `{symbol} out_sym` - symbol to convert to

### returns

- `{pair<asset, string>}` - return amount, type of order (sell_market or buy_market)

### example
```c++
#include <sx.newdex/newdex.hpp>

// user input
const asset quantity = asset{10000, symbol{"EOS", 4}};
const uint64_t mid = 470; // EOS/USN pair
const symbol out_sym = symbol{"USDT", 4};

// get newdex info
const auto [out, order] = newdex::get_amount_out( mid, quantity, out_sym );

// => [ "4.6500 USDT", "sell-market" ]
```

## STATIC `get_fee`

Get NewDex total fee

### params

### returns

- `{uint8_t}` - total fee (trade + protocol)

### example

```c++
const uint8_t fee = newdex::get_fee();
// => 15
```
