
#include <bts/db/level_map.hpp>
#include <bts/lotto/lotto_db.hpp>
#include <fc/reflect/variant.hpp>
#include <bts/lotto/lotto_rule_validator.hpp>
#include <bts/lotto/lotto_block.hpp>
#include <bts/lotto/lotto_config.hpp>

namespace bts { namespace lotto {

    namespace detail
    {
        class lotto_db_impl
        {
            public:
                lotto_db_impl(){}
                // map drawning number to drawing record
                bts::db::level_map<uint32_t, drawing_record>  _drawing2record;
                bts::db::level_map<uint32_t, block_summary>   _block2summary;
                bts::db::level_map<uint32_t, std::vector<uint32_t>>   _delegate2blocks;
				rule_validator_ptr                           _rule_validator;
            
        };
    }

    lotto_db::lotto_db()
    :my( new detail::lotto_db_impl() )
    {
        set_transaction_validator( std::make_shared<lotto_transaction_validator>(this) );
		set_rule_validator(std::make_shared<rule_validator>(this));
    }

    lotto_db::~lotto_db()
    {
    }

    void lotto_db::open( const fc::path& dir, bool create )
    {
        try {
            chain_database::open( dir, create );
            my->_drawing2record.open( dir / "drawing2record", create );
            my->_block2summary.open( dir / "block2summary", create );
        } FC_RETHROW_EXCEPTIONS( warn, "Error loading domain database ${dir}", ("dir", dir)("create", create) );
    }

    void lotto_db::close() 
    {
        my->_drawing2record.close();
        my->_block2summary.close();
    }

	void lotto_db::set_rule_validator( const rule_validator_ptr& v )
    {
       my->_rule_validator = v;
    }

	uint64_t lotto_db::get_jackpot_for_ticket( uint64_t ticket_block_num, uint64_t lucky_number, uint16_t odds, uint16_t amount)
    {
		/* TODO: generate winning_number according to future blocks, maybe with prove of work
		 * winning_number should be validate by block validation. or generated during block mining, so we can get directly from here.
		 */
        FC_ASSERT(head_block_num() - ticket_block_num > BTS_LOTTO_BLOCKS_BEFORE_JACKPOTS_DRAW);
		// fc::sha256 winning_number;
		// using the next block generated block number
        uint64_t winning_number = my->_block2summary.fetch(ticket_block_num + BTS_LOTTO_BLOCKS_BEFORE_JACKPOTS_DRAW).winning_number;
		
        // TODO: what's global_odds, ignore currenly.
		uint64_t global_odds = 0;

		auto dr = my->_drawing2record.fetch(ticket_block_num + BTS_LOTTO_BLOCKS_BEFORE_JACKPOTS_DRAW);
        
        return my->_rule_validator->evaluate_jackpot(winning_number, lucky_number, dr.total_jackpot);

		// 3. jackpot should not be calculated here, 
		/*
		fc::sha256::encoder enc;
		enc.write( (char*)&lucky_number, sizeof(lucky_number) );
		enc.write( (char*)&winning_number, sizeof(winning_number) );
		enc.result();
		fc::bigint  result_bigint( enc.result() );

		// the ticket number must be below the winning threshold to claim the jackpot
		auto winning_threshold = result_bigint.to_int64 % fc::bigint( global_odds * odds ).to_int64();
		auto ticket_threshold = amount / odds;
		if (winning_threshold < ticket_threshold)	// we have a winners
		{
			return jackpots;
		}
		*/

		// return 0;
    }

    /**
     * Performs global validation of a block to make sure that no two transactions conflict. In
     * the case of the lotto only one transaction can claim the jackpot.
     */
    block_evaluation_state_ptr lotto_db::validate( const trx_block& blk, const signed_transactions& deterministic_trxs )
    {
        block_evaluation_state_ptr blockstate = chain_database::validate(blk, deterministic_trxs);

        // TODO:
        auto head_blk = static_cast<const bts::lotto::lotto_block&>(blk);
        auto delegate_id = lookup_delegate(blk.block_num)->delegate_id;
        auto itr = my->_delegate2blocks.find(delegate_id);
        if( itr.valid() )
        {
            auto block_ids = itr.value();
            auto trx_blk = fetch_block(block_ids[block_ids.size() - 1]);    // GetLastBlockProducedByDelegate
            auto lotto_blk = static_cast<bts::lotto::lotto_block&>(trx_blk);
            
            // TODO: reviewing the hash.
            FC_ASSERT(fc::sha256::hash(head_blk.revealed_secret) == lotto_blk.secret);
        } else {
            FC_ASSERT(head_blk.revealed_secret == fc::sha256());   //  this is the first block produced by delegate
        }
        
        for( const signed_transaction& trx : deterministic_trxs )
        {
            for ( auto i : trx.inputs)
            {
                // TODO: need to remove fees?
				auto o = fetch_output(i.output_ref);
                auto draw_block_num = fetch_trx_num(i.output_ref.trx_hash).block_num + BTS_LOTTO_BLOCKS_BEFORE_JACKPOTS_DRAW;
                uint64_t trx_paid = 0;
                if (o.claim_func == claim_ticket) {
                    // all the tickets drawing in this trx should belong to the same blocks.
                    for ( auto out : trx.outputs)
                    {
						if (out.claim_func == claim_by_signature) {
                            trx_paid += out.amount.get_rounded_amount();
                        }
                    }
					// The in.output should all be tickets, and the out should all be jackpots
                    auto draw_record = my->_drawing2record.fetch(draw_block_num);
                    FC_ASSERT(draw_record.total_paid + trx_paid <= draw_record.total_jackpot, "The paid jackpots is out of the total jackpot.");
					break;
                }
            }
        }

        return blockstate;
    }

    /** 
     *  Called after a block has been validated and appends
     *  it to the block chain storing all relevant transactions and updating the
     *  winning database.
     */
    void lotto_db::store( const trx_block& blk, const signed_transactions& deterministic_trxs, const block_evaluation_state_ptr& state )
    {
        chain_database::store(blk, deterministic_trxs, state);

        block_summary bs;
        uint64_t ticket_sales = 0;
        uint64_t amout_won = 0;
        for( const signed_transaction& trx : deterministic_trxs )
        {
            for ( auto o : trx.outputs)
            {
                if (o.claim_func == claim_ticket) {
                    ticket_sales += o.amount.get_rounded_amount();
                }
            }

            for ( auto i : trx.inputs)
            {
                // TODO: need to remove fees?
				auto o = fetch_output(i.output_ref);
                auto draw_block_num = fetch_trx_num(i.output_ref.trx_hash).block_num + BTS_LOTTO_BLOCKS_BEFORE_JACKPOTS_DRAW;
                uint64_t trx_paid = 0;
                if (o.claim_func == claim_ticket) {
                    // all the tickets drawing in this trx should belong to the same blocks.
                    for ( auto out : trx.outputs)
                    {
						if (out.claim_func == claim_by_signature) {
                            amout_won += out.amount.get_rounded_amount();
                            trx_paid += out.amount.get_rounded_amount();
                        }
                    }
					// The in.output should all be tickets, and the out should all be jackpots
                    auto draw_record = my->_drawing2record.fetch(draw_block_num);
                    draw_record.total_paid = draw_record.total_paid + trx_paid;
                    my->_drawing2record.store(draw_block_num, draw_record);
					break;
                }
            }
        }
        bs.ticket_sales = ticket_sales;
        bs.amount_won = amout_won;
        // TODO: hash according to block info, move to block summary?
        auto head_blk = static_cast<const bts::lotto::lotto_block&>(blk);
        auto random = fc::sha256::hash(head_blk.revealed_secret.str());
        for( uint32_t i = 1; i < 100; ++i )
        {
            auto h_blk = fetch_block(head_blk.block_num - i);
            auto lotto_blk = static_cast<bts::lotto::lotto_block&>(h_blk);
            random = fc::sha256::hash(lotto_blk.revealed_secret.str() + random.str()); // where + is concat
        }

        // TODO: change wining_number to sha256, and recheck whether sha356 is suitable for hashing.
        bs.winning_number = ((uint64_t)random._hash[0]) <<32 & ((uint64_t)random._hash[0]);
        my->_block2summary.store(blk.block_num, bs);

        // the drawing record in this block, corresponding to previous related ticket purchase block (blk.block_num - BTS_LOTTO_BLOCKS_BEFORE_JACKPOTS_DRAW)
        uint64_t last_jackpot_pool = 0;
        if (blk.block_num > 0)
        {
            last_jackpot_pool = my->_drawing2record.fetch(blk.block_num - 1).jackpot_pool;
        }
        drawing_record dr;
        dr.total_jackpot = my->_rule_validator->evaluate_total_jackpot(bs.winning_number, blk.block_num, last_jackpot_pool + bs.ticket_sales);
        // just the begin, still not paid
        dr.total_paid = 0;
        dr.jackpot_pool = last_jackpot_pool + bs.ticket_sales - dr.total_jackpot;
        
        // TODO: how to move to validate()
        FC_ASSERT(dr.jackpot_pool >= 0, "jackpot is out ...");
        // Assert that the jackpot_pool is large than the sum ticket sales of blk.block_num - 99 , blk.block_num - 98 ... blk.block_num.
        
        my->_drawing2record.store(blk.block_num, dr);
        
        // TODO: Should block's delegate id be retrieved this way? Then, how to achieve this before store?
        auto delegate_id = lookup_delegate(blk.block_num)->delegate_id;
        auto block_ids = my->_delegate2blocks.fetch(delegate_id);
        block_ids.push_back(blk.block_num);
        my->_delegate2blocks.store(delegate_id, block_ids);
    }

    /**
     * When a block is popped from the chain, this method implements the necessary code
     * to revert the blockchain database to the proper state.
     */
    trx_block lotto_db::pop_block()
    {
       auto blk = chain_database::pop_block();
       // TODO: remove block summary from DB
       FC_ASSERT( !"Not Implemented" );
       return blk;
    }

}} // bts::lotto
