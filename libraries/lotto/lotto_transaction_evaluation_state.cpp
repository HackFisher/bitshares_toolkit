#include <algorithm>

#include <fc/io/raw.hpp>
#include <fc/log/logger.hpp>

#include <bts/lotto/lotto_transaction_evaluation_state.hpp>
#include <bts/lotto/lotto_operations.hpp>
#include <bts/lotto/lotto_db.hpp>
#include <bts/blockchain/config.hpp>
#include <bts/lotto/lotto_config.hpp>

namespace bts { namespace lotto {

    lotto_trx_evaluation_state::lotto_trx_evaluation_state(const chain_interface_ptr& blockchain)
        :transaction_evaluation_state(blockchain), total_ticket_sales(0)
    {
        // TODO_lotto_db = db_ptr;
    }

    lotto_trx_evaluation_state::~lotto_trx_evaluation_state()
    {
    }

    /**
     * If one of transaction inputs are claim ticket, then all its inputs should be claim tickets, and all output should be claim jackpot.
     * All the mature_day of the outputs must allocate in different days.
     */
    /*
    void lotto_transaction_validator::evaluate_ticket_jackpot_transactions(lotto_trx_evaluation_state& state)
    {
        try{
            state.inputs = _lotto_db->fetch_inputs( state.trx.inputs );

            // TODO: make sure addresses are all the same
            for ( auto in : state.inputs )
            {
                FC_ASSERT(in.output.claim_func = claim_ticket);
            }

            std::vector<uint16_t> mature_days;
            for ( auto out : state.trx.outputs )
            {
                FC_ASSERT(out.claim_func = claim_jackpot);
                auto jackpot_out = out.as<claim_jackpot_output>();
                mature_days.push_back(jackpot_out.mature_day);
            }

            std::sort(mature_days.begin(), mature_days.end());
            for ( size_t i = 0; i < mature_days.size(); i ++ )
            {
                if ( i == 0 )
                {
                    continue;
                }
                else
                {
                    FC_ASSERT((mature_days[i] - mature_days[i-1]) >= 1)
                }
            }
        } FC_RETHROW_EXCEPTIONS( warn, "")
    }
    */

    void lotto_trx_evaluation_state::evaluate_operation(const operation& op){
        
        switch ((operation_type_enum)op.type.value) // TODO: Fix me, why type value
        {
            case ticket_op_type:
                evaluate_ticket(op.as<ticket_operation>());
                break;
            case jackpot_op_type:
                evaluate_jackpot(op.as<jackpot_operation>());
                break;
        }

        // TODO: maybe should not use Bitshares Me chain_database;
        transaction_evaluation_state::evaluate_operation(op);
    }

    void lotto_trx_evaluation_state::evaluate_ticket(const ticket_operation& op)
    {
        /*
        try {
            auto claim_ticket = out.as<claim_ticket_output>();
            asset_id_type u = _lotto_db->get_rule_ptr(claim_ticket.ticket.ticket_func)->get_asset_unit();
            FC_ASSERT(u == out.amount.unit,"Ticket amount's unit should be same with ticket's unit ${u}", ("u", u));
            
            auto lotto_state = dynamic_cast<lotto_trx_evaluation_state&>(state);
            lotto_state.total_ticket_sales += out.amount.get_rounded_amount();
            lotto_state.add_output_asset( out.amount );

            // the ticket out's owner could be other, in this case, A buy/pay a ticket for B.
    } FC_RETHROW_EXCEPTIONS( warn, "" ) }
        */
    }

    void lotto_trx_evaluation_state::evaluate_jackpot(const jackpot_operation& op)
    {
        /*
        try {
            auto lotto_state = dynamic_cast<lotto_trx_evaluation_state&>(state);

            auto claim_ticket = in.output.as<claim_ticket_output>();

            auto trx_loc = in.source;

            // returns the jackpot based upon which lottery the ticket was for.
            auto jackpot = _lotto_db->get_rule_ptr(claim_ticket.ticket.ticket_func)
                ->jackpot_payout(meta_ticket_output(trx_loc, in.output_num, claim_ticket, in.output.amount));
            if( jackpot.get_rounded_amount() > 0 ) // we have a winner!
            {
                // @see state.balance_assets();
                state.add_input_asset(jackpot);
                // TODO: improve ticket_winnings to support multi assets
                lotto_state.ticket_winnings += jackpot.get_rounded_amount();
                if (in.output.amount.unit == 0)
                {
                    accumulate_votes(in.output.amount.get_rounded_amount(), in.source.block_num, state);
                    block_state->add_input_delegate_votes(in.delegate_id, in.output.amount);
                    block_state->add_output_delegate_votes(state.trx.vote, in.output.amount);
                }
            } else {
                //throws or invalid when they did not win
                //Since the ticket input should be generated in deterministic trxs and has already been calcuated there, this should not hapen 

    }
} FC_RETHROW_EXCEPTIONS(warn, "")
        */

        /*  The jackpot output amount should be less than the maximum limitation
        try {
        FC_ASSERT(out.amount.get_rounded_amount() <= BTS_LOTTO_RULE_MAXIMUM_REWARDS_EACH_JACKPOT_OUTPUT);
        wlog("validate_jackpot_output jackpots: ${j}", ("j", out.amount));
        state.add_output_asset(out.amount);

        } FC_RETHROW_EXCEPTIONS( warn, "" )
        */
    }

    //void lotto_trx_evaluation_state::evaluate_cash(const cash_operation& op)
    //{
        /*
        try {
            auto claim = in.output.as<claim_jackpot_output>(); 
            FC_ASSERT( state.has_signature( claim.owner ), "", ("owner",claim.owner)("sigs",state.sigs) );
       
            FC_ASSERT( _db->head_block_num() >= (claim.mature_day * BTS_BLOCKCHAIN_BLOCKS_PER_DAY) + in.source.block_num,
                "The jackpot input should be after mature day");

            state.add_input_asset( in.output.amount );

            // TODO: https://github.com/HackFisher/bitshares_toolkit/issues/24
            if( in.output.amount.unit == 0 )
            {
                accumulate_votes( in.output.amount.get_rounded_amount(), in.source.block_num, state );
                block_state->add_input_delegate_votes(in.delegate_id, in.output.amount);
                block_state->add_output_delegate_votes( state.trx.vote, in.output.amount );
            }

    } FC_RETHROW_EXCEPTIONS( warn, "" )
        */
    //}
    void lotto_trx_evaluation_state::post_evaluate()
    {
        transaction_evaluation_state::post_evaluate();

        FC_ASSERT(found_secret_out == true, "There is no secret out found.");
    }
}} // bts::lotto
