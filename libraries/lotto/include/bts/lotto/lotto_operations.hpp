#pragma once

#include <bts/lotto/ticket.hpp>

namespace bts { namespace lotto {
   
/**
 *  @enum operation_type_enum
 *  @brief Enumerates the types of supported claim types in lotto
 *  @see bts::blockchain::claim_type_enum
 *  Using integer values reserved for Bitshares Lotto by Bitshares Toolkit
 *  TODO: Temporary using 30->39, to be confirmed with toolkit
 *  TODO: variant TBD, see void to_variant( const bts::blockchain::trx_output& var,  variant& vo ) in transaction.cpp
 */
enum operation_type_enum
{  
   ticket_op_type         = 30,
   jackpot_op_type        = 31
};

struct ticket_operation
{
    static const operation_type_enum type;
    
    ticket_operation(){}

    /**
     *  Who owns the ticket and thus can receive the jackpot
     */
    bts::blockchain::address                    owner;

    output_ticket ticket;
};

/**
 *  Basic output that can be spent with a signature of the owner and after mature days.
 */
struct jackpot_operation
{
    static const operation_type_enum type;

    jackpot_operation(const bts::blockchain::address& a, const uint16_t& m) :owner(a), mature_day(m){}
    jackpot_operation(const bts::blockchain::address& a) :owner(a), mature_day(0){}
    jackpot_operation() : mature_day(0){}

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

FC_REFLECT_ENUM(bts::lotto::operation_type_enum, (ticket_op_type)(jackpot_op_type));
FC_REFLECT(bts::lotto::ticket_operation, (owner)(ticket));
FC_REFLECT(bts::lotto::jackpot_operation, (owner)(mature_day));
