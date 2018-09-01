﻿/**
 *  @dev minakokojima
 */

#include "pomelo.hpp"


/// @abi action
void pomelo::init() {
    require_auth(_self);    

/*
    while (buyorders.begin() != buyorders.end()) {
	    buyorders.erase(buyorders.begin());
    }
    while (sellorders.begin() != sellorders.end()) {
    	sellorders.erase(sellorders.begin());
    }        
    while (txlogs.begin() != txlogs.end()) {
	    txlogs.erase(txlogs.begin());
    }*/
}

/// @abi action
void pomelo::test() {

}

/// @abi action
void pomelo::cancelsell(account_name account, uint64_t id) {
   /* require_auth(account);
    auto itr = sellorders.find(id);
    eosio_assert(itr->account == account, "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");
    // TODO: 返还
    sellorders.erase(itr);*/
}

/// @abi action
void pomelo::cancelbuy(account_name account, uint64_t id) {
    require_auth(account);
    auto itr = buyorders.find(id);
    eosio_assert(itr->account == account, "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");
    // TODO: 返还
    buyorders.erase(itr);
}

/// @abi action
void pomelo::buy(account_name account, asset bid, asset ask, account_name issuer) {
    // 生成购买订单
    buyorder o;
    o.account = account;
    o.bid = bid;
    o.ask = ask;
    // do_buy_trade(b);
    buyorders.emplace(issuer, [&](auto& t) {    
        t.id = buyorders.available_primary_key();
        t.account = account;
        t.bid = bid;
        t.ask = ask;      
    });
}

/// @abi action
void pomelo::sell(account_name account, asset bid, asset ask, account_name issuer) {
    sellorder o;
    o.account = account;
    o.bid = bid;
    o.ask = ask;

    sellorders.emplace(issuer, [&](auto& t) {    
        t.id = sellorders.available_primary_key();
        t.account = account;
        t.bid = bid;
        t.ask = ask;      
    });

    //require_auth(account);
    /*
    eosio_assert(quant.symbol != EOS, "Must sale non-EOS currency");
    eosio_assert(total_eos > 0, "");

    action(
        permission_level{ account, N(active) },
        TOKEN_CONTRACT, N(transfer),
        make_tuple(account, _self, quant, string("transfer"))) // 由合约账号代为管理欲出售的代币
        .send();

    // 生成出售订单
    sellorder s;
    s.account = account;
    s.asset = quant;
    s.per = (double)total_eos / (double)quant.amount;
    s.total_eos = total_eos;
    do_sell_trade(s);*/
}



uint64_t string_to_price(string s) {
    uint64_t z = 0;
    for (int i=0;i<s.size();++i) {
        if ('0' <= s[i] && s[i] <= '9') {
            z *= 10; 
            z += s[i] - '0';
        }
    }
    return z;
}

// memo [buy,issuer,HPY,1.23] EOS
// memo [sell,issuer,HPY,1.23] HPY
// @abi action
void pomelo::onTransfer(account_name from, account_name to, asset bid, std::string memo) {        
    if (to != _self) return;    
    require_auth(from);
    eosio_assert(bid.is_valid(), "invalid token transfer");
    eosio_assert(bid.amount > 0, "must bet a positive amount");

    if (memo.substr(0, 3) == "buy") {
        eosio_assert(bid.symbol == EOS, "only EOS allowed");
        memo.erase(0, 4);
        std::size_t p = memo.find(','); 
        auto issuer = N(memo.substr(0, p));
        memo.erase(0, p+1);
        auto ask_symbol = string_to_symbol(4, memo.substr(0, p).c_str());        
        memo.erase(0, p+1);
        auto ask_price = string_to_price(memo);
        buy(from, bid, asset(ask_symbol, ask_price), issuer);
    } else {	
        // sell 
    }
}

#define EOSIO_WAST(TYPE, MEMBERS)                                                                                  \
    extern "C"                                                                                                       \
    {                                                                                                                \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                \
        {                                                                                                            \
                                                                                                                     \
            auto self = receiver;                                                                                    \
            if (action == N(onerror))                                                                                \
            {                                                                                                        \
                eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account"); \
            }                                                                                                        \
            if (code == TOKEN_CONTRACT && action == N(transfer)) {                                                   \
                action = N(onTransfer);                                                                              \
            }                                            \
            if ((code == TOKEN_CONTRACT && action == N(onTransfer)) || code == self && action != N(onTransfer)) {                               \
                TYPE thiscontract(self);                                                                             \
                switch (action)                                                                                      \
                {                                                                                                    \
                    EOSIO_API(TYPE, MEMBERS)                                                                         \
                }                                                                                                     \
            }                                                                                                        \
        }                                                                                                            \
    }

// generate .wasm and .wast file
EOSIO_WAST(pomelo, (onTransfer)(cancelbuy)(cancelsell)(buy)(sell))

// generate .abi file
// EOSIO_ABI(pomelo, (cancelbuy)(cancelsell)(buy)(sell))