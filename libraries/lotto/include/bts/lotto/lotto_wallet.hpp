#pragma once
#include <bts/blockchain/address.hpp>
#include <bts/blockchain/transaction.hpp>
#include <bts/wallet/wallet.hpp>
#include <bts/lotto/lotto_db.hpp>

namespace bts { namespace lotto {
    using namespace bts::blockchain;
    using namespace bts::wallet;

    // TODO convert ticket_number to a struct
    // // TODO: To be tested. to convert to ticket_number(uint64_t) and ticket_number::as_uint64()
    // ticket_number ticket_num(
    // ticket_num_param >> 32, (ticket_num_param << 32) >> 48, (ticket_num_param << 48) >> 48);
    typedef ticket_index ticket_number;

    namespace detail { class lotto_wallet_impl; }
    
    class lotto_wallet : public bts::wallet::wallet
    {
        public:
            lotto_wallet( const lotto_db_ptr& lotto_db );
            virtual ~lotto_wallet() override;
            bts::blockchain::signed_transaction buy_ticket(const uint64_t& luckynumber, const uint16_t& odds,
                                                        asset amount);

            bts::blockchain::signed_transaction next_secret(const fc::sha256& hash_secret, const fc::sha256& reveal_last_secret,
                const uint32_t& delegate_id, address required_signee);
            
            // TODO: db is not required?
            //std::map<ticket_index, trx_output> list_tickets(lotto_db& db);

            //std::map<ticket_index, trx_output> list_jackpots(lotto_db& db);

            //bts::blockchain::signed_transaction cash_jackpot(const ticket_index& jackpot_idx);

        protected:
            //virtual bool scan_output( transaction_state& state, const trx_output& out, const output_reference& out_ref, const bts::wallet::output_index& oidx );
            //virtual void scan_input( transaction_state& state, const output_reference& ref, const output_index& idx);

        private:
             std::unique_ptr<detail::lotto_wallet_impl> my;
     };

    typedef std::shared_ptr<lotto_wallet> lotto_wallet_ptr;
} } // bts::lotto
