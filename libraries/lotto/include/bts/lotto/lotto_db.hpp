#pragma once
#include <bts/blockchain/asset.hpp>
#include <bts/blockchain/transaction.hpp>
#include <bts/blockchain/chain_database.hpp>
#include <bts/wallet/wallet.hpp>

#include <bts/lotto/lotto_transaction_validator.hpp>
#include <bts/lotto/lotto_rule.hpp>

namespace bts { namespace lotto {
using namespace bts::wallet;

namespace detail  { class lotto_db_impl; }

struct block_summary
{
   block_summary()
   :random_number(0){}

   uint64_t random_number; // the previous block uses this value to calculate jackpot, this value is generated according current block's info
};

class lotto_db : public bts::blockchain::chain_database
{
    public:
        lotto_db();
        ~lotto_db();
    
        void             open( const fc::path& dir, bool create );
        void             close();

        void set_lotto_rule(const lotto_rule_ptr& r);

        void validate_secret_transactions(const signed_transactions& deterministic_trxs, const trx_block& blk);

        /**
         * Calculate the jackpot reward for this ticket.
         */
        asset draw_jackpot_for_ticket(const output_index& out_idx, const bts::lotto::claim_ticket_output& ticket, const asset& amount);

        uint64_t fetch_blk_random_number( const uint32_t& blk_index );

        /**
         * Generate transactions for winners, claim_ticket as in, and claim_jackpot as out
         */
        virtual signed_transactions generate_deterministic_transactions();

        /**
         * Performs global validation of a block to make sure that no two transactions conflict. In
         * the case of the lotto only one transaction can claim the jackpot.
         */
        virtual block_evaluation_state_ptr validate( const trx_block& blk, const signed_transactions& deterministic_trxs );

        /** 
         *  Called after a block has been validated and appends
         *  it to the block chain storing all relevant transactions and updating the
         *  winning database.
         */
        virtual void store( const trx_block& blk, const signed_transactions& deterministic_trxs, const block_evaluation_state_ptr& state );

        /**
         * When a block is popped from the chain, this method implements the necessary code
         * to revert the blockchain database to the proper state.
         */
        virtual trx_block pop_block();
    private:
         std::unique_ptr<detail::lotto_db_impl> my;

};

typedef std::shared_ptr<lotto_db> lotto_db_ptr;

}} // bts::lotto

FC_REFLECT( bts::lotto::block_summary, (random_number))
