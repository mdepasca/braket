#ifndef KET_GATE_GATE_HPP
# define KET_GATE_GATE_HPP

# include <cassert>
# include <cstddef>
# include <array>
# include <iterator>
# include <algorithm>
# include <numeric>
# include <utility>
# include <type_traits>

# include <ket/qubit.hpp>
# include <ket/control.hpp>
# include <ket/meta/state_integer_of.hpp>
# include <ket/meta/bit_integer_of.hpp>
# include <ket/utility/loop_n.hpp>
# include <ket/utility/integer_exp2.hpp>
# ifndef NDEBUG
#   include <ket/utility/integer_log2.hpp>
# endif


namespace ket
{
  namespace gate
  {
    namespace gate_detail
    {
      template <typename StateInteger, std::size_t num_operated_qubits>
      inline void do_make_qubit_masks(std::array<StateInteger, num_operated_qubits>&)
      { }

      template <typename StateInteger, std::size_t num_operated_qubits, typename Qubit, typename... Qubits>
      inline void do_make_qubit_masks(std::array<StateInteger, num_operated_qubits>& result, Qubit&& qubit, Qubits&&... qubits)
      {
        static_assert(
          std::is_same<StateInteger, typename ::ket::meta::state_integer_of<typename std::remove_reference<Qubit>::type>::type>::value,
          "state_integer_type of Qubit should be the same as StateInteger");

        result[num_operated_qubits - 1u - sizeof...(Qubits)] = StateInteger{1u} << qubit;
        do_make_qubit_masks(result, std::forward<Qubits>(qubits)...);
      }

      template <typename Qubit, typename... Qubits>
      inline void make_qubit_masks(
        std::array<typename ::ket::meta::state_integer_of<typename std::remove_reference<Qubit>::type>::type, sizeof...(Qubits) + 1u>& result,
        Qubit&& qubit, Qubits&&... qubits)
      {
        using state_integer_type = typename ::ket::meta::state_integer_of<typename std::remove_reference<Qubit>::type>::type;
        static_assert(std::is_unsigned<state_integer_type>::value, "state_integer_type of Qubit should be unsigned");

        do_make_qubit_masks(result, std::forward<Qubit>(qubit), std::forward<Qubits>(qubits)...);
      }

      template <typename Qubit, typename... Qubits>
      inline void make_index_masks(
        std::array<typename ::ket::meta::state_integer_of<typename std::remove_reference<Qubit>::type>::type, sizeof...(Qubits) + 2u>& result,
        Qubit&& qubit, Qubits&&... qubits)
      {
        using state_integer_type = typename ::ket::meta::state_integer_of<typename std::remove_reference<Qubit>::type>::type;
        static_assert(std::is_unsigned<state_integer_type>::value, "state_integer_type of Qubit should be unsigned");

        static constexpr auto num_operated_qubits = sizeof...(Qubits) + 1u;
        using bit_integer_type = typename ::ket::meta::bit_integer_of<typename std::remove_reference<Qubit>::type>::type;
        using qubit_type = ::ket::qubit<state_integer_type, bit_integer_type>;
        auto sorted_qubits = std::array<qubit_type, num_operated_qubits>{::ket::remove_control(qubit), ::ket::remove_control(qubits)...};
        std::sort(std::begin(sorted_qubits), std::end(sorted_qubits));

        for (auto index = 0u; index < num_operated_qubits; ++index)
          result[index] = (state_integer_type{1u} << (sorted_qubits[index] - index)) - state_integer_type{1u};
        result.back() = compl state_integer_type{0u};

        std::adjacent_difference(std::begin(result), std::end(result), std::begin(result));
      }

      template <typename StateInteger, std::size_t num_operated_qubits>
      inline void make_indices(
        std::array<StateInteger, ::ket::utility::integer_exp2<std::size_t>(num_operated_qubits)>& result,
        StateInteger const index_wo_qubits,
        std::array<StateInteger, num_operated_qubits> const& qubit_masks,
        std::array<StateInteger, num_operated_qubits + 1u> const& index_masks)
      {
        // xx0xx0xx0xx
        result[0u] = StateInteger{0u};
        for (auto index_mask_index = std::size_t{0u}; index_mask_index < num_operated_qubits + std::size_t{1u}; ++index_mask_index)
          result[0u] |= (index_wo_qubits bitand index_masks[index_mask_index]) << index_mask_index;

        static constexpr auto num_indices = ::ket::utility::integer_exp2<std::size_t>(num_operated_qubits);
        for (auto n = std::size_t{1u}; n < num_indices; ++n)
        {
          result[n] = result[0u];
          for (auto qubit_index = std::size_t{0u}; qubit_index < num_operated_qubits; ++qubit_index)
            if (((StateInteger{1u} << qubit_index) bitand n) != StateInteger{0u})
              result[n] |= qubit_masks[qubit_index];
        }
      }
    } // namespace gate_detail

    // USAGE:
    // - for Hadamard gate
    //   ::ket::gate::gate(parallel_policy, first, last,
    //     [](auto const first, auto const& indices, int const)
    //     {
    //       auto const zero_iter = first + indices[0b0u];
    //       auto const one_iter = first + indices[0b1u];
    //       auto const zero_iter_value = *zero_iter;
    //
    //       *zero_iter += *one_iter;
    //       *zero_iter *= one_div_root_two;
    //       *one_iter = zero_iter_value - *one_iter;
    //       *one_iter *= one_div_root_two;
    //     },
    //     qubit);
    // - for CNOT gate
    //   ::ket::gate::gate(parallel_policy, first, last,
    //     [](auto const first, auto const& indices, int const)
    //     { std::iter_swap(first + indices[0b10u], first + indices[0b11u]); },
    //     target_qubit, control_qubit);
    template <typename ParallelPolicy, typename RandomAccessIterator, typename Function, typename Qubit, typename... Qubits>
    inline void gate(
      ParallelPolicy const parallel_policy,
      RandomAccessIterator const first, RandomAccessIterator const last,
      Function&& function, Qubit&& qubit, Qubits&&... qubits)
    {
      using state_integer_type = typename ::ket::meta::state_integer_of<typename std::remove_reference<Qubit>::type>::type;
      using bit_integer_type = typename ::ket::meta::bit_integer_of<typename std::remove_reference<Qubit>::type>::type;
      static_assert(std::is_unsigned<state_integer_type>::value, "state_integer_type of Qubit should be unsigned");
      static_assert(std::is_unsigned<bit_integer_type>::value, "bit_integer_type of Qubit should be unsigned");
      assert(
        ::ket::utility::integer_exp2<state_integer_type>(qubit)
        < static_cast<state_integer_type>(last - first));
      assert(
        ::ket::utility::integer_exp2<state_integer_type>(
          ::ket::utility::integer_log2<bit_integer_type>(last - first))
        == static_cast<state_integer_type>(last - first));

      static constexpr auto num_operated_qubits = sizeof...(Qubits) + 1u;
      auto qubit_masks = std::array<state_integer_type, num_operated_qubits>{};
      ::ket::gate::gate_detail::make_qubit_masks(qubit_masks, qubit, qubits...);
      auto index_masks = std::array<state_integer_type, num_operated_qubits + 1u>{};
      ::ket::gate::gate_detail::make_index_masks(index_masks, std::forward<Qubit>(qubit), std::forward<Qubits>(qubits)...);

      auto indices = std::array<state_integer_type, ::ket::utility::integer_exp2<std::size_t>(num_operated_qubits)>{};
      using ::ket::utility::loop_n;
      loop_n(
        parallel_policy,
        static_cast<state_integer_type>(last - first) >> num_operated_qubits,
        [first, &function, &qubit_masks, &index_masks, &indices](state_integer_type const index_wo_qubits, int const thread_index)
        {
          // ex. qubit_masks[0]=00000100000; qubit_masks[1]=00100000000; qubit_masks[2]=00000000100;
          // indices[0b000]=xx0xx0xx0xx; indices[0b001]=xx0xx1xx0xx; indices[0b010]=xx1xx0xx0xx; indices[0b011]=xx1xx1xx0xx;
          // indices[0b100]=xx0xx0xx1xx; indices[0b101]=xx0xx1xx1xx; indices[0b110]=xx1xx0xx1xx; indices[0b111]=xx1xx1xx1xx;
          ::ket::gate::gate_detail::make_indices(indices, index_wo_qubits, qubit_masks, index_masks);
          function(first, indices, thread_index);
        });
    }

    template <typename RandomAccessIterator, typename Function, typename Qubit, typename... Qubits>
    inline void gate(
      RandomAccessIterator const first, RandomAccessIterator const last,
      Function&& function, Qubit&& qubit, Qubits&&... qubits)
    {
      ::ket::gate::gate(
        ::ket::utility::policy::make_sequential(),
        first, last, std::forward<Function>(function),
        std::forward<Qubit>(qubit), std::forward<Qubits>(qubits)...);
    }

    namespace ranges
    {
      template <typename ParallelPolicy, typename RandomAccessRange, typename Function, typename Qubit, typename... Qubits>
      inline RandomAccessRange& gate(
        ParallelPolicy const parallel_policy, RandomAccessRange& state,
        Function&& function, Qubit&& qubit, Qubits&&... qubits)
      {
        ::ket::gate::gate(
          parallel_policy,
          std::begin(state), std::end(state), std::forward<Function>(function),
          std::forward<Qubit>(qubit), std::forward<Qubits>(qubits)...);
        return state;
      }

      template <typename RandomAccessRange, typename Function, typename Qubit, typename... Qubits>
      inline RandomAccessRange& gate(
        RandomAccessRange& state, Function&& function, Qubit&& qubit, Qubits&&... qubits)
      {
        return ::ket::gate::ranges::gate(
          ::ket::utility::policy::make_sequential(),
          state, std::forward<Function>(function), std::forward<Qubit>(qubit), std::forward<Qubits>(qubits)...);
      }
    } // namespace ranges
  } // namespace gate
} // namespace ket


#endif // KET_GATE_GATE_HPP
