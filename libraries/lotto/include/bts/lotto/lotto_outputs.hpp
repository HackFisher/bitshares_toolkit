#pragma once

/**
 *  @file bts/lotto/lotto_outputs.hpp
 *  @brief defines extended output types in lotto
 */
#include <bts/blockchain/outputs.hpp>

namespace bts { namespace lotto {
   
/**
 *  @enum claim_type_enum
 *  @brief Enumerates the types of supported claim types in lotto
 *  @see bts::blockchain::claim_type_enum
 *  Using integer values reserved for Bitshares Lotto by Bitshares Toolkit
 *  TODO: Temporary using 30->39, to be confirmed with toolkit
 *  TODO: variant TBD, see void to_variant( const bts::blockchain::trx_output& var,  variant& vo ) in transaction.cpp
 */
enum claim_type_enum
{  
   claim_secret			= 30,
   claim_ticket         = 31,
   claim_jackpot        = 32
   
};

struct claim_ticket_input
{
    static const claim_type_enum type;
};

struct claim_ticket_output
{
    static const claim_type_enum type;
    
    claim_ticket_output():lucky_number(0), odds(1){}

    /**
     *  This is the number chosen by the user or at 
     *  random, ie: their lotto ticket number.
     */
    uint64_t                   lucky_number;

    /**
     *  Who owns the ticket and thus can claim the jackpot
     */
    bts::blockchain::address   owner;

    /** The probability of winning... increasing the odds will 
     * cause the amount won to grow by Jackpot * odds, but the
     * probability of winning decreases by 2*odds.
     */
    uint16_t                   odds; 
};

struct claim_secret_input
{
	static const claim_type_enum type;
};

struct claim_secret_output
{
	static const claim_type_enum type;

	claim_secret_output() :secret(fc::sha256()), revealed_secret(fc::sha256()), delegate_id(0){}

	fc::sha256					secret;  // HASH( S[n] ) where n is the index in the array of secrets generated by this delegate
	fc::sha256					revealed_secret; //  S[n-1]
	uint32_t					delegate_id;
};

struct claim_jackpot_input
{
    static const claim_type_enum type;
};

/**
 *  Basic output that can be spent with a signature of the owner and after mature days.
 */
struct claim_jackpot_output
{
    static const claim_type_enum type;

    claim_jackpot_output(const bts::blockchain::address& a, const uint16_t& m) :owner(a), mature_day(m){}
    claim_jackpot_output(const bts::blockchain::address& a) :owner(a), mature_day(0){}
    claim_jackpot_output() : mature_day(0){}

    /**
     * The beneficiary of the jackpot.
     * Must be the ticket owner or the beneficiary in the ticket
     */
    bts::blockchain::address   owner;

    /**
     * After mature days the owner can spent this output
     */
    uint16_t mature_day;
};

}} //bts::lotto

FC_REFLECT_ENUM(bts::lotto::claim_type_enum, (claim_secret)(claim_ticket)(claim_jackpot));
FC_REFLECT(bts::lotto::claim_ticket_input, BOOST_PP_SEQ_NIL);
FC_REFLECT(bts::lotto::claim_ticket_output, (lucky_number)(owner)(odds));
FC_REFLECT(bts::lotto::claim_secret_input, BOOST_PP_SEQ_NIL);
FC_REFLECT(bts::lotto::claim_secret_output, (secret)(revealed_secret)(delegate_id));
FC_REFLECT(bts::lotto::claim_jackpot_input, BOOST_PP_SEQ_NIL);
FC_REFLECT(bts::lotto::claim_jackpot_output, (owner)(mature_day));
